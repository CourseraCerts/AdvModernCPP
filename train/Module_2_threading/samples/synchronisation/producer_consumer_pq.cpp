/*
Implement a sophisticated message processing system with priority-based task scheduling and multiple consumer types,
simulating a real-time event processing platform.

🔍 Practice
Using the code below, create a complete task processing system:
* Multiple producer threads generating tasks with different priorities
* Different types of consumer threads (some handle only high priority, others handle all)
* Monitor queue size and consumer wait times
* Implement graceful shutdown that processes all remaining tasks
Test the system under various load conditions and priority distributions.

✅ Success Checklist

* Higher priority tasks are processed before lower priority tasks
* System maintains responsiveness even under high load
* Graceful shutdown processes all remaining tasks
* Performance metrics show effective load balancing across consumers
*/
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <ranges>
#include <thread>

enum class Priority { LOW = 1, NORMAL = 2, HIGH = 3, CRITICAL = 4 };
template <typename E>
constexpr auto get_val(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}
struct Task {
  int id;
  Priority priority;
  std::string payload;
  std::chrono::steady_clock::time_point timestamp;

  Task(int id, Priority p, const std::string& data)
      : id(id), priority(p), payload(data), timestamp(std::chrono::steady_clock::now()) {}
};
template <typename S>
S& operator<<(S& stream, const Task& task) {
  auto time_now = std::chrono::steady_clock::now();
  stream << "taskId: " << task.id << "  priority: " << get_val(task.priority) << " creation v/s consumption: "
         << std::chrono::duration_cast<std::chrono::microseconds>(time_now - task.timestamp).count();
  return stream;
}
std::mutex coutMutex;
void log_thread_safe(std::string_view message) {
  std::scoped_lock guard(coutMutex);
  std::cout << message << '\n';
}

struct TaskComparator {
  bool operator()(const Task& a, const Task& b) const {
    if (a.priority != b.priority) {
      return static_cast<int>(a.priority) < static_cast<int>(b.priority);
    }
    return a.timestamp > b.timestamp;  // Earlier timestamp has higher priority
  }
};

class PriorityTaskQueue {
 private:
  mutable std::mutex mutex_;
  std::priority_queue<Task, std::vector<Task>, TaskComparator> queue_;
  std::condition_variable condition_;
  std::atomic<bool> shutdown_{false};
  std::atomic<int> waitingConsumers_{0};

 public:
  void push(const Task& task) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(task);
    condition_.notify_one();
  }

  bool pop(Task& task, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000)) {
    std::unique_lock<std::mutex> lock(mutex_);
    waitingConsumers_.fetch_add(1);

    auto result = condition_.wait_for(lock, timeout, [this] { return !queue_.empty() || shutdown_.load(); });

    waitingConsumers_.fetch_sub(1);

    if (!result || (shutdown_.load() && queue_.empty())) {
      return false;
    }

    task = queue_.top();
    queue_.pop();
    return true;
  }

  void shutdown() {
    shutdown_.store(true);
    condition_.notify_all();
  }

  size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  int getWaitingConsumers() const { return waitingConsumers_.load(); }
};

class TaskProcessor {
 private:
  int processorId_;
  std::atomic<int> processedTasks_{0};

 public:
  TaskProcessor(int id) : processorId_(id) {}

  void processTask(const Task& task) {
    auto processingTime = std::chrono::milliseconds(50 + static_cast<int>(task.priority) * 25);
    std::stringstream ss;
    ss << "Processor " << processorId_ << " processing task " << task.id
       << " (Priority: " << static_cast<int>(task.priority) << ")" << " " << processedTasks_.load();
    log_thread_safe(ss.view());

    std::this_thread::sleep_for(processingTime);
    processedTasks_.fetch_add(1);
  }

  int getProcessedCount() const { return processedTasks_.load(); }
};
class ThreadGuard {
  std::thread thread_{};

 public:
  ThreadGuard() = default;
  ThreadGuard(std::thread&& t) noexcept : thread_(std::move(t)) {}
  ThreadGuard(const ThreadGuard&) = delete;
  ThreadGuard& operator=(const ThreadGuard&) = delete;

  ThreadGuard(ThreadGuard&& other) = default;
  ThreadGuard& operator=(ThreadGuard&& other) {
    if (this != &other) {
      if (thread_.joinable()) {
        thread_.join();
      }
      thread_ = std::move(other.thread_);
    }
    return *this;
  }
  ~ThreadGuard() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }
};

void run() {
  std::vector<ThreadGuard> consumers;
  PriorityTaskQueue ptq{};
  constexpr int PRODUCER_SIZE = 10;
  constexpr int PRODUCER_TASK_SIZE = 10;
  constexpr int ALL_CONSUMER_SIZE = 2;
  constexpr int HP_CONSUMER_SIZE = 3;

  auto producer = [&ptq](int idx) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(get_val(Priority::LOW), get_val(Priority::CRITICAL));
    for (auto val : std::ranges::iota_view(0, PRODUCER_TASK_SIZE)) {
      int taskid = idx * 1000 + val;
      auto priority = static_cast<Priority>(dist(gen) % (get_val(Priority::CRITICAL) + 1));
      Task task{taskid, priority, "task data for task: " + std::to_string(taskid)};
      ptq.push(task);
    }
  };

  auto consume_h = [&ptq](std::string tag, Priority priority, int idx) {
    log_thread_safe("\t\t " + tag + "\t STARTING with priority: " + std::to_string(get_val(priority)));
    TaskProcessor tp(idx);
    while (true) {
      Task taskObj(-1, Priority::CRITICAL, "Dummy object");
      if (ptq.pop(taskObj)) {
        if (taskObj.priority != priority) {
          ptq.push(taskObj);
        } else {
          tp.processTask(taskObj);
        }
      } else {
        log_thread_safe("\t\t" + tag + "\t STOPPING after processing: " + std::to_string(tp.getProcessedCount()));
        break;
      }
    }
  };
  auto consume_all = [&ptq](std::string tag, int idx) {
    log_thread_safe("\t\t " + tag + "\t STARTING");
    TaskProcessor tp(idx);
    while (true) {
      Task taskObj(-1, Priority::CRITICAL, "Dummy object");
      if (ptq.pop(taskObj)) {
        tp.processTask(taskObj);
      } else {
        log_thread_safe("\t\t" + tag + "\t STOPPING after processing: " + std::to_string(tp.getProcessedCount()));
        break;
      }
    }
  };
  {
    std::vector<ThreadGuard> producers;
    for (int i = 1; i <= PRODUCER_SIZE; ++i) {
      producers.emplace_back(std::thread(producer, i));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    for (int i = 0; i < HP_CONSUMER_SIZE; ++i) {
      consumers.emplace_back(std::thread(consume_h, "hp_consumer" + std::to_string(i), Priority::HIGH, i));
      log_thread_safe("\t consumers waiting: " + std::to_string(ptq.getWaitingConsumers()));
    }
    for (int i = 0; i < ALL_CONSUMER_SIZE; ++i) {
      consumers.emplace_back(std::thread(consume_all, "consumer_" + std::to_string(i), i));
      log_thread_safe("\t consumers waiting: " + std::to_string(ptq.getWaitingConsumers()));
    }
  }
  while (ptq.size() > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  ptq.shutdown();
}

int main() {
  run();
  return 0;
}