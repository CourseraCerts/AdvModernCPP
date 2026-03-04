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
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <random>
#include <ranges>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

namespace {
class ThreadSafeQueue {
 public:
  using Item = std::pair<int, double>;

  void push(int producerId, double value) {
    {
      std::lock_guard lock(mutex_);
      queue_.emplace(producerId, value);
    }
    condition_.notify_one();
  }

  std::optional<Item> wait_and_pop(std::stop_token stopToken) {
    std::unique_lock lock(mutex_);
    std::stop_callback stopCb(stopToken, [this] { condition_.notify_all(); });

    condition_.wait(lock, [this, &stopToken] {
      return !queue_.empty() || closed_ || stopToken.stop_requested();
    });

    if (stopToken.stop_requested()) {
      return std::nullopt;
    }

    if (queue_.empty()) {
      return std::nullopt;  // closed_ && empty
    }

    Item value = queue_.front();
    queue_.pop();
    return value;
  }

  void close() {
    {
      std::lock_guard lock(mutex_);
      closed_ = true;
    }
    condition_.notify_all();
  }

  bool closed() const {
    std::lock_guard lock(mutex_);
    return closed_;
  }

 private:
  mutable std::mutex mutex_;
  std::queue<Item> queue_;
  std::condition_variable condition_;
  bool closed_ = false;
};

struct ProductionPlan {
  int producers = 3;
  int consumers = 2;
  int itemsPerProducer = 15;
};

struct Metrics {
  std::atomic<int> produced{0};
  std::atomic<int> consumed{0};
};

std::vector<double> create_payloads(int count, std::uint64_t seed) {
  std::mt19937_64 rng{seed};
  std::uniform_real_distribution<double> dist(0.0, 100.0);
  std::vector<double> payloads;
  payloads.reserve(count);
  for (int i = 0; i < count; ++i) {
    payloads.push_back(dist(rng));
  }
  return payloads;
}

void run_demo(const ProductionPlan& plan) {
  ThreadSafeQueue queue;
  Metrics metrics;

  auto consumerTask = [&queue, &metrics](std::stop_token stopToken, int consumerId) {
    for (;;) {
      if (auto item = queue.wait_and_pop(stopToken)) {
        ++metrics.consumed;
        std::cout << "consumer " << consumerId << " processed value " << item->second << " from producer "
                  << item->first << '\n';
        std::this_thread::sleep_for(20ms);
      } else {
        if (stopToken.stop_requested()) {
          std::cout << "consumer " << consumerId << " stopping on request" << std::endl;
        } else {
          std::cout << "consumer " << consumerId << " drained the queue" << std::endl;
        }
        break;
      }
    }
  };

  auto producerTask = [&queue, &metrics, &plan](std::stop_token stopToken, int producerId) {
    auto payloads = create_payloads(plan.itemsPerProducer, std::random_device{}() + producerId);
    for (double value : payloads) {
      if (stopToken.stop_requested()) {
        std::cout << "producer " << producerId << " cancelled" << std::endl;
        break;
      }
      queue.push(producerId, value);
      ++metrics.produced;
      std::cout << "producer " << producerId << " -> " << value << std::endl;
      std::this_thread::sleep_for(10ms);
    }
  };

  std::vector<std::jthread> consumers;
  consumers.reserve(plan.consumers);
  for (int id : std::views::iota(0, plan.consumers)) {
    consumers.emplace_back(consumerTask, id);
  }

  {
    std::vector<std::jthread> producers;
    producers.reserve(plan.producers);
    for (int id : std::views::iota(0, plan.producers)) {
      producers.emplace_back(producerTask, id);
    }
  }  // producers completed their work here

  queue.close();

  std::cout << "Produced " << metrics.produced.load() << " item(s); Consumed " << metrics.consumed.load()
            << " item(s)." << std::endl;

  // Let consumers finish draining the queue before leaving scope (jthread joins automatically).
}
}  // namespace

int main() {
  ProductionPlan plan{};
  run_demo(plan);
  std::cout << "All tasks finished." << std::endl;
  return 0;
}
