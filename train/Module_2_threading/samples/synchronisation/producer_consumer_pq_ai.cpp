/*
Implement a sophisticated message processing system with priority-based task scheduling and multiple consumer types,
simulating a real-time event processing platform.

🔍 Practice
Using the code below, create a complete task processing system:

Multiple producer threads generating tasks with different priorities

Different types of consumer threads (some handle only high priority, others handle all)

Monitor queue size and consumer wait times

Implement graceful shutdown that processes all remaining tasks

Test the system under various load conditions and priority distributions.
*/
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <sstream>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

enum class Priority { LOW = 1, NORMAL = 2, HIGH = 3, CRITICAL = 4 };

struct Task {
  int id;
  Priority priority;
  std::string payload;
  std::chrono::steady_clock::time_point timestamp;

  Task(int id, Priority p, std::string data)
      : id(id), priority(p), payload(std::move(data)), timestamp(std::chrono::steady_clock::now()) {}
};

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
  std::atomic<std::uint64_t> totalWaitNanos_{0};
  std::atomic<std::uint64_t> waitSamples_{0};

  void recordWaitDuration(std::chrono::steady_clock::time_point start) {
    const auto waited = std::chrono::steady_clock::now() - start;
    const auto nanos = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(waited).count());
    totalWaitNanos_.fetch_add(nanos, std::memory_order_relaxed);
    waitSamples_.fetch_add(1, std::memory_order_relaxed);
  }

 public:
  void push(const Task& task) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(task);
    }
    condition_.notify_one();
  }

  bool pop(Task& task, Priority minPriority = Priority::LOW,
           std::chrono::milliseconds timeout = std::chrono::milliseconds(1000)) {
    const auto startWait = std::chrono::steady_clock::now();
    waitingConsumers_.fetch_add(1, std::memory_order_relaxed);
    std::unique_lock<std::mutex> lock(mutex_);
    const auto predicate = [this, minPriority]() {
      if (shutdown_.load(std::memory_order_relaxed)) {
        return true;
      }
      if (queue_.empty()) {
        return false;
      }
      return static_cast<int>(queue_.top().priority) >= static_cast<int>(minPriority);
    };

    while (true) {
      if (!condition_.wait_for(lock, timeout, predicate)) {
        waitingConsumers_.fetch_sub(1, std::memory_order_relaxed);
        recordWaitDuration(startWait);
        return false;
      }

      if (!queue_.empty() &&
          static_cast<int>(queue_.top().priority) >= static_cast<int>(minPriority)) {
        task = queue_.top();
        queue_.pop();
        waitingConsumers_.fetch_sub(1, std::memory_order_relaxed);
        recordWaitDuration(startWait);
        return true;
      }

      if (shutdown_.load(std::memory_order_relaxed) && queue_.empty()) {
        waitingConsumers_.fetch_sub(1, std::memory_order_relaxed);
        recordWaitDuration(startWait);
        return false;
      }
    }
  }

  void shutdown() {
    shutdown_.store(true, std::memory_order_relaxed);
    condition_.notify_all();
  }

  size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  int getWaitingConsumers() const { return waitingConsumers_.load(std::memory_order_relaxed); }

  bool isShutdown() const { return shutdown_.load(std::memory_order_relaxed); }

  double averageWaitMillis() const {
    const auto samples = waitSamples_.load(std::memory_order_relaxed);
    if (samples == 0) {
      return 0.0;
    }
    const auto nanos = totalWaitNanos_.load(std::memory_order_relaxed);
    return static_cast<double>(nanos) / 1'000'000.0 / static_cast<double>(samples);
  }
};

class TaskProcessor {
 private:
  int processorId_;
  std::atomic<int> processedTasks_{0};

 public:
  explicit TaskProcessor(int id) : processorId_(id) {}

  void processTask(const Task& task) {
    const auto processingTime = std::chrono::milliseconds(50 + static_cast<int>(task.priority) * 25);

    std::cout << "Processor " << processorId_ << " processing task " << task.id
              << " (Priority: " << static_cast<int>(task.priority) << ")" << std::endl;

    std::this_thread::sleep_for(processingTime);
    processedTasks_.fetch_add(1, std::memory_order_relaxed);
  }

  int getProcessedCount() const { return processedTasks_.load(std::memory_order_relaxed); }
};

struct ProducerProfile {
  std::string name;
  std::chrono::milliseconds minDelay;
  std::chrono::milliseconds maxDelay;
  std::array<int, 4> priorityWeights;
};

struct SimulationProfile {
  std::string label;
  std::vector<ProducerProfile> producerProfiles;
  int generalConsumers{0};
  int urgentOnlyConsumers{0};
  Priority urgentThreshold{Priority::HIGH};
  std::chrono::milliseconds duration{0ms};
  std::chrono::milliseconds monitorInterval{500ms};
};

Priority pickPriority(const ProducerProfile& profile, std::mt19937& rng) {
  std::discrete_distribution<int> dist(profile.priorityWeights.begin(), profile.priorityWeights.end());
  return static_cast<Priority>(dist(rng) + 1);
}

void runProducer(std::stop_token stopToken, PriorityTaskQueue& queue, const ProducerProfile& profile,
                 std::atomic<int>& nextTaskId, std::atomic<int>& producedCount) {
  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int> delayDist(static_cast<int>(profile.minDelay.count()),
                                               static_cast<int>(profile.maxDelay.count()));

  while (!stopToken.stop_requested()) {
    const int id = nextTaskId.fetch_add(1, std::memory_order_relaxed);
    const auto priority = pickPriority(profile, rng);

    std::ostringstream payload;
    payload << profile.name << "-task-" << id;

    queue.push(Task{id, priority, payload.str()});
    producedCount.fetch_add(1, std::memory_order_relaxed);
    std::this_thread::sleep_for(std::chrono::milliseconds(delayDist(rng)));
  }
}

void consumerLoop(std::stop_token stopToken, PriorityTaskQueue& queue, TaskProcessor& processor,
                  Priority minPriority) {
  Task placeholder{0, Priority::LOW, ""};
  while (!stopToken.stop_requested()) {
    if (queue.pop(placeholder, minPriority, 750ms)) {
      processor.processTask(placeholder);
      continue;
    }

    if (queue.isShutdown()) {
      break;
    }
  }

  while (queue.pop(placeholder, minPriority, 100ms)) {
    processor.processTask(placeholder);
  }
}

std::string formatMillis(double value) {
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(2) << value;
  return stream.str();
}

void monitorQueue(const PriorityTaskQueue& queue, std::stop_token stopToken,
                  std::chrono::milliseconds interval) {
  while (!stopToken.stop_requested()) {
    std::cout << "[Monitor] pending=" << queue.size() << " waitingConsumers=" << queue.getWaitingConsumers()
              << " avgWaitMs=" << formatMillis(queue.averageWaitMillis()) << std::endl;
    std::this_thread::sleep_for(interval);
  }
}

struct SimulationSummary {
  int produced{0};
  std::vector<int> processedPerConsumer;
  double averageWaitMs{0.0};
};

SimulationSummary runSimulation(const SimulationProfile& profile) {
  std::cout << "\n=== Scenario: " << profile.label << " ===" << std::endl;

  PriorityTaskQueue queue;
  std::atomic<int> nextTaskId{0};
  std::atomic<int> producedCount{0};

  std::vector<std::jthread> producers;
  producers.reserve(profile.producerProfiles.size());

  for (const auto& producerProfile : profile.producerProfiles) {
    producers.emplace_back([&queue, &nextTaskId, &producedCount, producerProfile](std::stop_token token) {
      runProducer(token, queue, producerProfile, nextTaskId, producedCount);
    });
  }

  const int totalConsumers = profile.generalConsumers + profile.urgentOnlyConsumers;
  std::vector<std::unique_ptr<TaskProcessor>> processors;
  processors.reserve(totalConsumers);
  std::vector<std::jthread> consumers;
  consumers.reserve(totalConsumers);

  int processorId = 1;
  for (int idx = 0; idx < profile.generalConsumers; ++idx) {
    processors.emplace_back(std::make_unique<TaskProcessor>(processorId++));
    auto* processorPtr = processors.back().get();
    consumers.emplace_back([&queue, processorPtr](std::stop_token token) {
      consumerLoop(token, queue, *processorPtr, Priority::LOW);
    });
  }

  for (int idx = 0; idx < profile.urgentOnlyConsumers; ++idx) {
    processors.emplace_back(std::make_unique<TaskProcessor>(processorId++));
    auto* processorPtr = processors.back().get();
    consumers.emplace_back([&queue, processorPtr, threshold = profile.urgentThreshold](std::stop_token token) {
      consumerLoop(token, queue, *processorPtr, threshold);
    });
  }

  std::jthread monitorThread([&queue, interval = profile.monitorInterval](std::stop_token token) {
    monitorQueue(queue, token, interval);
  });

  std::this_thread::sleep_for(profile.duration);

  for (auto& producer : producers) {
    producer.request_stop();
  }

  while (queue.size() > 0) {
    std::this_thread::sleep_for(50ms);
  }

  queue.shutdown();

  for (auto& consumer : consumers) {
    consumer.request_stop();
  }

  monitorThread.request_stop();

  SimulationSummary summary;
  summary.produced = producedCount.load(std::memory_order_relaxed);
  summary.processedPerConsumer.reserve(processors.size());
  for (const auto& processor : processors) {
    summary.processedPerConsumer.push_back(processor->getProcessedCount());
  }
  summary.averageWaitMs = queue.averageWaitMillis();

  std::cout << "Produced tasks: " << summary.produced << std::endl;
  for (std::size_t idx = 0; idx < summary.processedPerConsumer.size(); ++idx) {
    std::cout << "Processor " << (idx + 1) << " handled " << summary.processedPerConsumer[idx] << " tasks"
              << std::endl;
  }
  std::cout << "Average consumer wait: " << formatMillis(summary.averageWaitMs) << " ms" << std::endl;

  return summary;
}

int main() {
  const std::vector<SimulationProfile> profiles{
      {.label = "Balanced mixed workload",
       .producerProfiles = {
           {.name = "telemetry", .minDelay = 30ms, .maxDelay = 90ms, .priorityWeights = {6, 5, 3, 1}},
           {.name = "payments", .minDelay = 40ms, .maxDelay = 120ms, .priorityWeights = {2, 4, 6, 3}},
       },
       .generalConsumers = 2,
       .urgentOnlyConsumers = 1,
       .urgentThreshold = Priority::HIGH,
       .duration = 3s,
       .monitorInterval = 500ms},
      {.label = "Latency-sensitive burst",
       .producerProfiles = {
           {.name = "alerts", .minDelay = 15ms, .maxDelay = 40ms, .priorityWeights = {1, 2, 5, 7}},
           {.name = "analytics", .minDelay = 50ms, .maxDelay = 150ms, .priorityWeights = {5, 4, 2, 1}},
           {.name = "ad-hoc", .minDelay = 60ms, .maxDelay = 140ms, .priorityWeights = {4, 4, 3, 2}},
       },
       .generalConsumers = 3,
       .urgentOnlyConsumers = 2,
       .urgentThreshold = Priority::HIGH,
       .duration = 3s,
       .monitorInterval = 400ms},
      {.label = "Throughput stress",
       .producerProfiles = {
           {.name = "iot", .minDelay = 10ms, .maxDelay = 30ms, .priorityWeights = {7, 6, 2, 1}},
           {.name = "edge", .minDelay = 12ms, .maxDelay = 35ms, .priorityWeights = {6, 5, 3, 2}},
           {.name = "priority_clients", .minDelay = 20ms, .maxDelay = 60ms, .priorityWeights = {1, 2, 6, 8}},
           {.name = "batch", .minDelay = 80ms, .maxDelay = 200ms, .priorityWeights = {8, 3, 1, 1}},
       },
       .generalConsumers = 4,
       .urgentOnlyConsumers = 2,
       .urgentThreshold = Priority::CRITICAL,
       .duration = 3s,
       .monitorInterval = 300ms}};

  for (const auto& profile : profiles) {
    runSimulation(profile);
  }

  std::cout << "\nAll simulations completed successfully." << std::endl;
  return 0;
}
