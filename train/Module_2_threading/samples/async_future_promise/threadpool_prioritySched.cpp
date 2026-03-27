/*
Practice
Using the code below, create a load testing scenario that:
* Submits tasks with different priorities and processing times
* Monitors thread pool scaling behavior under varying loads
* Compares performance with different min/max thread configurations
* Analyzes queue dynamics and thread utilization patterns
* Test with burst loads, sustained loads, and mixed priority scenarios.

✅ Success Checklist

* Higher priority tasks are processed before lower priority tasks
* Thread pool scales up during high load periods
* Performance statistics accurately reflect system behavior
* System maintains stability under extreme load conditions
*/

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <ranges>
#include <thread>
constexpr auto MAX_SIM = 5;
enum class TaskPriority { LOW = 1, NORMAL = 2, HIGH = 3, CRITICAL = 4 };
template <typename E>
constexpr auto get_val(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}
namespace {
std::mutex gLogMutex;
template <typename... Args>
void logSync(std::ostream& stream, Args&&... args) {
  std::scoped_lock lock(gLogMutex);
  (stream << ... << args);
  stream.flush();
}
}  // namespace

struct Task {
  std::function<void()> function;
  TaskPriority priority;
  std::chrono::steady_clock::time_point submitTime;
  std::string taskId;

  Task(std::function<void()> func, TaskPriority prio, const std::string& id = "")
      : function(std::move(func)), priority(prio), submitTime(std::chrono::steady_clock::now()), taskId(id) {}
};

struct TaskComparator {
  bool operator()(const Task& a, const Task& b) const {
    if (a.priority != b.priority) {
      return static_cast<int>(a.priority) < static_cast<int>(b.priority);
    }
    return a.submitTime > b.submitTime;  // Earlier submission has higher priority
  }
};

class DynamicThreadPool {
 private:
  std::vector<std::thread> workers_;
  std::priority_queue<Task, std::vector<Task>, TaskComparator> taskQueue_;

  mutable std::mutex queueMutex_;
  std::condition_variable condition_;

  std::atomic<bool> shutdown_{false};
  std::atomic<size_t> activeThreads_{0};
  std::atomic<size_t> totalTasksProcessed_{0};

  // Dynamic scaling parameters
  std::atomic<size_t> minThreads_;
  std::atomic<size_t> maxThreads_;
  std::atomic<size_t> currentThreads_{0};

  // Performance monitoring
  std::atomic<double> averageTaskTime_{0.0};
  std::atomic<size_t> queueHighWaterMark_{0};

  std::chrono::steady_clock::time_point t0;

  void workerThread() {
    while (!shutdown_.load()) {
      Task task([]() {}, TaskPriority::LOW);
      bool hasTask = false;

      {
        std::unique_lock<std::mutex> lock(queueMutex_);

        condition_.wait(lock, [this] { return !taskQueue_.empty() || shutdown_.load(); });

        if (shutdown_.load() && taskQueue_.empty()) {
          break;
        }

        if (!taskQueue_.empty()) {
          task = std::move(const_cast<Task&>(taskQueue_.top()));
          taskQueue_.pop();
          hasTask = true;
        }
      }

      if (hasTask) {
        activeThreads_.fetch_add(1);

        auto startTime = std::chrono::steady_clock::now();

        try {
          auto submit_offset = std::chrono::duration_cast<std::chrono::microseconds>(task.submitTime - t0).count();
          logSync(std::cout, " processing task (" + std::to_string(get_val(task.priority)) + ") : " + task.taskId +
                                 "\t task order by time: " + std::to_string(submit_offset) + "\n");
          task.function();
        } catch (const std::exception& e) {
          std::cout << "Task " << task.taskId << " failed: " << e.what() << std::endl;
        } catch (...) {
          std::cout << "Task " << task.taskId << " failed with unknown exception" << std::endl;
        }

        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(endTime - startTime);

        updatePerformanceMetrics(duration.count());

        totalTasksProcessed_.fetch_add(1);
        activeThreads_.fetch_sub(1);
      }
    }

    currentThreads_.fetch_sub(1);
  }

  void updatePerformanceMetrics(double taskDuration) {
    // Simple exponential moving average
    double currentAvg = averageTaskTime_.load();
    double newAvg = (currentAvg * 0.9) + (taskDuration * 0.1);
    averageTaskTime_.store(newAvg);
  }

  void scaleThreadPool() {
    size_t queueSize = getQueueSize();
    size_t current = currentThreads_.load();
    size_t active = activeThreads_.load();

    // Update high water mark
    size_t currentHighWater = queueHighWaterMark_.load();
    if (queueSize > currentHighWater) {
      queueHighWaterMark_.store(queueSize);
    }

    // Scale up if queue is growing and we have capacity
    if (queueSize > current * 2 && current < maxThreads_.load()) {
      addWorkerThread();
    }

    // Scale down if threads are mostly idle (simplified logic)
    if (queueSize == 0 && active < current / 2 && current > minThreads_.load()) {
      // In a real implementation, we'd implement controlled thread termination
      // For simplicity, we'll just track that we could scale down
    }
  }

  void addWorkerThread() {
    workers_.emplace_back(&DynamicThreadPool::workerThread, this);
    currentThreads_.fetch_add(1);
    std::cout << "Scaled up to " << currentThreads_.load() << " threads" << std::endl;
  }

 public:
  DynamicThreadPool(size_t minThreads = 2, size_t maxThreads = std::thread::hardware_concurrency() * 2)
      : minThreads_(minThreads), maxThreads_(maxThreads) {
    // Start with minimum threads
    for (size_t i = 0; i < minThreads; ++i) {
      addWorkerThread();
    }

    std::cout << "Dynamic thread pool initialized with " << minThreads << " threads (max: " << maxThreads << ")"
              << std::endl;
    this->t0 = std::chrono::steady_clock::now();
  }

  ~DynamicThreadPool() { shutdown(); }

  template <typename Func>
  void submit(Func&& func, TaskPriority priority = TaskPriority::NORMAL, const std::string& taskId = "") {
    {
      std::lock_guard<std::mutex> lock(queueMutex_);
      taskQueue_.emplace(std::forward<Func>(func), priority, taskId);
    }

    condition_.notify_one();

    // Trigger scaling evaluation
    scaleThreadPool();
  }

  size_t getQueueSize() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return taskQueue_.size();
  }

  struct PoolStats {
    size_t currentThreads;
    size_t activeThreads;
    size_t queueSize;
    size_t totalTasksProcessed;
    double averageTaskTime;
    size_t queueHighWaterMark;
  };

  PoolStats getStats() const {
    return PoolStats{currentThreads_.load(),      activeThreads_.load(),   getQueueSize(),
                     totalTasksProcessed_.load(), averageTaskTime_.load(), queueHighWaterMark_.load()};
  }

  void shutdown() {
    shutdown_.store(true);
    condition_.notify_all();

    for (auto& worker : workers_) {
      if (worker.joinable()) {
        worker.join();
      }
    }

    workers_.clear();
    std::cout << "Thread pool shutdown completed" << std::endl;
  }

  void printStats() const {
    auto stats = getStats();
    std::cout << "\n=== Thread Pool Statistics ===" << std::endl;
    std::cout << "Current threads: " << stats.currentThreads << " | \t Active threads: " << stats.activeThreads
              << std::endl;
    std::cout << "Queue size: " << stats.queueSize << " | \t Total tasks processed: " << stats.totalTasksProcessed
              << std::endl;
    std::cout << "Average task time: " << stats.averageTaskTime << " ms" << std::endl;
    std::cout << "Queue high water mark: " << stats.queueHighWaterMark << std::endl;
  }
};

auto getWork(int val) {
  auto work = [val]() {
    // logSync(std::cout, "\t processing Task: ", val);
    std::this_thread::sleep_for(std::chrono::milliseconds(val));
  };
  return std::move(work);
}
void runTaskGenerator(DynamicThreadPool& dtp, int sim_id, int task_count) {
  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int> dist(get_val(TaskPriority::LOW), get_val(TaskPriority::CRITICAL));
  for (auto val : std::ranges::iota_view(1, task_count + 1)) {
    int taskId = sim_id * 1000 + val;
    auto priority = static_cast<TaskPriority>(dist(rng) % (get_val(TaskPriority::CRITICAL) + 1));
    auto now = std::chrono::steady_clock::now();
    auto function = getWork(taskId);
    dtp.submit(std::move(function), priority, std::to_string(taskId));
  }
}

int main() {
  for (auto val : std::ranges::iota_view(2, MAX_SIM + 1)) {
    DynamicThreadPool dtp;
    logSync(std::cout, "starting with events: " + std::to_string(100 * val));
    runTaskGenerator(dtp, val, 100 * val);
    std::jthread monitor([&dtp](std::stop_token stopToken) {
      while (dtp.getQueueSize() != 0) {
        auto qsize = dtp.getQueueSize();
        logSync(std::cerr, "Qsize: " + std::to_string(qsize));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    });
    while (dtp.getQueueSize() != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    dtp.printStats();
    logSync(std::cout, "\n -------------------  processing complete -------------------------- \n");

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  }
  return 0;
}