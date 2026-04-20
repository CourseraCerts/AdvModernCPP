/*

🔍 Practice
In the code below, compare performance of lock-free vs lock-based data structures:
* Benchmark lock-free queue vs std::queue with mutex protection
* Test with varying numbers of producer and consumer threads
* Analyze memory ordering effects on performance
* Measure contention and scalability characteristics
Experiment with different memory ordering constraints and their impact on correctness and performance.

✅ Success Checklist
* Lock-free data structures operate correctly without explicit synchronization
* Performance benchmarks show scalability advantages under high contention
* Memory ordering ensures correctness without unnecessary synchronization overhead
* System handles memory management safely in concurrent environment

*/
#include <atomic>
#include <iostream>
#include <memory>
#include <ostream>
#include <thread>
#include <type_traits>
#include <vector>

/* LockFreeQueue moved to LockFreeQueue.h */
#include "LockFreeQueue.h"

// Lock-free stack for comparison
template <typename T>
class LockFreeStack {
 private:
  struct Node {
    T data;
    Node* next;

    Node(T item) : data(std::move(item)), next(nullptr) {}
  };

  std::atomic<Node*> head_{nullptr};
  std::atomic<size_t> size_{0};

 public:
  void push(T item) {
    Node* newNode = new Node(std::move(item));
    newNode->next = head_.load(std::memory_order_relaxed);

    while (!head_.compare_exchange_weak(newNode->next, newNode, std::memory_order_release, std::memory_order_relaxed)) {
      // Loop until successful
    }

    size_.fetch_add(1, std::memory_order_relaxed);
  }

  bool pop(T& result) {
    Node* head = head_.load(std::memory_order_acquire);

    while (head &&
           !head_.compare_exchange_weak(head, head->next, std::memory_order_release, std::memory_order_relaxed)) {
      // Retry with updated head
    }

    if (!head) {
      return false;
    }

    result = std::move(head->data);
    size_.fetch_sub(1, std::memory_order_relaxed);

    // In production, use proper memory reclamation
    delete head;
    return true;
  }

  bool empty() const { return head_.load(std::memory_order_acquire) == nullptr; }

  size_t size() const { return size_.load(std::memory_order_acquire); }
};

// Performance benchmarking utility
class LockFreeBenchmark {
 public:
  template <typename Container>
  static void benchmarkContainer(const std::string& containerName, int operations, int producerThreads,
                                 int consumerThreads) {
    Container container;
    std::atomic<bool> start{false};
    std::atomic<int> itemsProduced{0};
    std::atomic<int> itemsConsumed{0};

    auto startTime = std::chrono::high_resolution_clock::now();

    // Producer threads
    std::vector<std::thread> producers;
    for (int i = 0; i < producerThreads; ++i) {
      producers.emplace_back([&, i]() {
        while (!start.load()) { /* spin wait */
        }

        int itemsPerProducer = operations / producerThreads;
        for (int j = 0; j < itemsPerProducer; ++j) {
          container.push(i * 1000 + j);
          itemsProduced.fetch_add(1);
        }
      });
    }

    // Consumer threads
    std::vector<std::thread> consumers;
    for (int i = 0; i < consumerThreads; ++i) {
      consumers.emplace_back([&]() {
        while (!start.load()) { /* spin wait */
        }

        int item;
        while (itemsConsumed.load() < operations) {
          if (container.pop(item)) {
            itemsConsumed.fetch_add(1);
          } else {
            std::this_thread::yield();
          }
        }
      });
    }

    // Start benchmark
    start.store(true);

    // Wait for completion
    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << containerName << " Benchmark Results:" << std::endl;
    std::cout << "  Operations: " << operations << std::endl;
    std::cout << "  Producer threads: " << producerThreads << std::endl;
    std::cout << "  Consumer threads: " << consumerThreads << std::endl;
    std::cout << "  Duration: " << duration.count() << " ms" << std::endl;
    std::cout << "  Items produced: " << itemsProduced.load() << std::endl;
    std::cout << "  Items consumed: " << itemsConsumed.load() << std::endl;
    std::cout << "  Throughput: " << (operations * 1000.0 / duration.count()) << " ops/sec" << std::endl;
    std::cout << std::endl;
  }
};

int main() { return 0; }