/*
Implement a producer-consumer pattern using condition variables to coordinate between data collection and processing
threads.

🔍 Practice
Using the code below, build a complete producer-consumer system where:
Producer threads generate data and add it to the thread-safe queue
Consumer threads process data from the queue as it becomes available
The system gracefully shuts down when all data is processed
Experiment with different numbers of producers and consumers to observe performance characteristics.

✅ Success Checklist

Producer threads successfully add data to the queue
Consumer threads wait appropriately when queue is empty
All produced data gets consumed without loss
System shuts down cleanly when production is complete
*/
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <queue>
#include <random>
#include <ranges>
#include <thread>
#include <utility>

class ThreadGuard {
  std::thread thread_;

 public:
  ThreadGuard() = default;
  explicit ThreadGuard(std::thread&& worker) noexcept : thread_(std::move(worker)) {}
  ThreadGuard(const ThreadGuard&) = delete;
  ThreadGuard& operator=(const ThreadGuard&) = delete;

  ThreadGuard(ThreadGuard&& other) noexcept = default;
  ThreadGuard& operator=(ThreadGuard&& other) noexcept {
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

class ThreadSafeQueue {
 private:
  mutable std::mutex mutex_;
  std::queue<std::pair<int, double>> queue_;
  std::condition_variable condition_;
  bool finished_ = false;

 public:
  void push(int id, double item) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(std::make_pair(id, item));
    condition_.notify_one();
  }

  bool pop(std::pair<int, double>& item) {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this] { return !queue_.empty() || finished_; });

    if (queue_.empty()) {
      return false;  // No more items and producer finished
    }

    item = queue_.front();
    queue_.pop();
    return true;
  }

  void finish() {
    std::lock_guard<std::mutex> lock(mutex_);
    finished_ = true;
    condition_.notify_all();
  }

  size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }
};

int test() {
  ThreadSafeQueue tsq;
  auto producer = [&tsq](int idx) {
    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_real_distribution<double> dist(0.0, 10.0);
    for (auto i : std::ranges::iota_view{0, 20}) {
      auto value = dist(rng);
      std::cout << "producer: " << i << " : " << value << std::endl;
      tsq.push(idx, value);
    }
  };
  auto consumer = [&tsq](int idx) {
    std::pair<int, double> data;
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    while (tsq.size() != 0) {
      if (tsq.pop(data)) {
        std::cout << "consumer: " << idx << " : " << " producer: " << data.first << " value: " << data.second
                  << std::endl;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds{5});
    }
    std::cout << "\t STOPPING consumer: " << idx << " queue has been emptied" << std::endl;
  };

  std::vector<ThreadGuard> producers;
  std::vector<ThreadGuard> consumers;
  for (auto i : std::ranges::iota_view{0, 3}) {
    producers.emplace_back(std::thread(producer, i));
    consumers.emplace_back(std::thread(consumer, i));
  }
  return 0;
}

int main() {
  auto val = test();
  std::cout << "============ DONE ++++++++++++++ " << std::endl;
  return val;
}