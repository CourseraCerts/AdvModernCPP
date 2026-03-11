/*
Implement a lock-free data structure using advanced atomic operations for maximum performance in high-throughput
scenarios.

🔍 Practice
Using the code below, compare the performance of lock-based vs lock-free data structures under different conditions:
* Varying numbers of threads (1, 2, 4, 8, 16)
* Different operation counts (1000, 10000, 100000)
* Mixed read/write ratios
Analyze when lock-free structures provide benefits and when traditional locking might be preferable.

✅ Success Checklist
* Lock-free stack operates correctly without explicit synchronization
* Performance comparison shows measurable differences under high contention
* System handles memory management safely in concurrent environment
* Understanding of when to choose lock-free vs lock-based approaches

💡 Key Points
* Mutex Types: std::shared_mutex enables multiple readers with exclusive writers for read-heavy workloads
* Deadlock Prevention: Consistent resource ordering eliminates circular dependencies that cause deadlocks
* Condition Variables: Enable efficient coordination without busy-waiting, essential for producer-consumer patterns
* Lock-Free Programming: Uses atomic operations for maximum performance but requires careful memory management
* RAII Principles: Automatic lock management prevents resource leaks and ensures exception safety
* Performance Analysis: Benchmarking helps choose appropriate synchronization strategies for specific use cases

❗ Common Mistakes to Avoid
* Using regular mutexes when shared mutexes would improve read performance significantly
* Inconsistent lock ordering across different functions leading to deadlocks
* Forgetting to handle spurious wake-ups in condition variable wait conditions
* Improper exception handling that leaves resources locked or corrupted
* Overusing lock-free structures without understanding their complexity and memory requirements
* Not considering the performance implications of different synchronization primitives

🚀 Next Steps
These advanced synchronization skills prepare you for building enterprise-grade concurrent systems. Consider applying
these concepts to:
* High-Frequency Trading Systems: Building ultra-low latency transaction processing with lock-free data structures
* Real-Time Analytics Platforms: Creating streaming data processors that handle millions of events per second
* Distributed Database Systems: Implementing transaction coordinators with sophisticated deadlock prevention
* Game Engine Development: Building concurrent systems for physics, rendering, and AI processing
* Financial Risk Management: Developing real-time portfolio analysis systems with complex synchronization requirements
*/
#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <stack>
#include <thread>
#include <vector>

template <typename T>
class LockFreeStack {
 private:
  struct Node {
    T data;
    std::atomic<Node*> next;

    Node(T item) : data(std::move(item)), next(nullptr) {}
  };

  std::atomic<Node*> head_;
  std::atomic<size_t> size_{0};

 public:
  LockFreeStack() : head_(nullptr) {}

  ~LockFreeStack() {
    Node* node = head_.load();
    while (node) {
      Node* next_node = node->next;
      delete node;
      head_.store(next_node);
      node = next_node;
    }
  }

  void push(T item) {
    Node* newNode = new Node(std::move(item));
    Node* oldHead = head_.load();  // Create an lvalue to hold the current head

    do {
      newNode->next = oldHead;  // Set the new node's next to the current head
    }

    while (!head_.compare_exchange_weak(oldHead, newNode));
    size_.fetch_add(1);
  }

  bool pop(T& result) {
    Node* head = head_.load();

    while (head && !head_.compare_exchange_weak(head, head->next.load())) {
      // Retry with updated head
    }

    if (!head) {
      return false;
    }

    result = std::move(head->data);
    size_.fetch_sub(1);

    // Safe deletion - in production, use hazard pointers or epochs
    delete head;
    return true;
  }

  bool empty() const { return head_.load() == nullptr; }

  size_t size() const { return size_.load(); }
};

class PerformanceComparison {
 public:
  static void benchmarkLockBased(int operations, int threads) {
    std::stack<int> stack;
    std::mutex stackMutex;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> workerThreads;
    for (int i = 0; i < threads; ++i) {
      workerThreads.emplace_back([&, i]() {
        for (int j = 0; j < operations / threads; ++j) {
          {
            std::lock_guard<std::mutex> lock(stackMutex);
            stack.push(i * 1000 + j);
          }

          {
            std::lock_guard<std::mutex> lock(stackMutex);
            if (!stack.empty()) {
              stack.pop();
            }
          }
        }
      });
    }

    for (auto& t : workerThreads) {
      t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Lock-based stack: " << duration.count() << " ms" << std::endl;
  }

  static void benchmarkLockFree(int operations, int threads) {
    LockFreeStack<int> stack;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> workerThreads;
    for (int i = 0; i < threads; ++i) {
      workerThreads.emplace_back([&, i]() {
        for (int j = 0; j < operations / threads; ++j) {
          stack.push(i * 1000 + j);

          int dummy;
          stack.pop(dummy);
        }
      });
    }

    for (auto& t : workerThreads) {
      t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Lock-free stack: " << duration.count() << " ms" << std::endl;
  }
};