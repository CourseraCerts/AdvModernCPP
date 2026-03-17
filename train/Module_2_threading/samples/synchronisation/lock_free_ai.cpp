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
#include <array>
#include <atomic>
#include <barrier>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <shared_mutex>
#include <span>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using namespace std::chrono_literals;

template <typename T>
class LockFreeStack {
 private:
  struct Node {
    T value;
    Node* next{nullptr};

    explicit Node(T data) : value(std::move(data)) {}
  };

  struct RetiredNode {
    Node* node;
    RetiredNode* next;
  };

  std::atomic<Node*> head_{nullptr};
  std::atomic<RetiredNode*> retiredHead_{nullptr};
  std::atomic<std::size_t> size_{0};

  static void reclaim(Node* node) {
    while (node) {
      Node* next = node->next;
      delete node;
      node = next;
    }
  }

  static void reclaimRetired(RetiredNode* retired) {
    while (retired) {
      RetiredNode* next = retired->next;
      delete retired->node;
      delete retired;
      retired = next;
    }
  }

  void deferDelete(Node* node) {
    auto* retired = new RetiredNode{node, nullptr};
    auto oldHead = retiredHead_.load(std::memory_order_relaxed);
    do {
      retired->next = oldHead;
    } while (!retiredHead_.compare_exchange_weak(oldHead, retired, std::memory_order_release,
                                                std::memory_order_relaxed));
  }

 public:
  LockFreeStack() = default;
  LockFreeStack(const LockFreeStack&) = delete;
  LockFreeStack& operator=(const LockFreeStack&) = delete;

  ~LockFreeStack() {
    reclaim(head_.exchange(nullptr, std::memory_order_acquire));
    reclaimRetired(retiredHead_.exchange(nullptr, std::memory_order_acquire));
  }

  void push(T value) {
    Node* newNode = new Node(std::move(value));
    Node* oldHead = head_.load(std::memory_order_acquire);
    do {
      newNode->next = oldHead;
    } while (!head_.compare_exchange_weak(oldHead, newNode, std::memory_order_release,
                                          std::memory_order_acquire));
    size_.fetch_add(1, std::memory_order_relaxed);
  }

  [[nodiscard]] bool tryPop(T& result) {
    Node* oldHead = head_.load(std::memory_order_acquire);
    while (oldHead &&
           !head_.compare_exchange_weak(oldHead, oldHead->next, std::memory_order_acq_rel,
                                        std::memory_order_acquire)) {
    }

    if (!oldHead) {
      return false;
    }

    result = oldHead->value;
    size_.fetch_sub(1, std::memory_order_relaxed);
    deferDelete(oldHead);
    return true;
  }

  [[nodiscard]] bool peek(T& result) const {
    Node* snapshot = head_.load(std::memory_order_acquire);
    if (!snapshot) {
      return false;
    }
    result = snapshot->value;
    return true;
  }

  [[nodiscard]] bool empty() const { return head_.load(std::memory_order_acquire) == nullptr; }

  [[nodiscard]] std::size_t size() const { return size_.load(std::memory_order_relaxed); }
};

template <typename T>
class LockBasedStack {
 private:
  std::vector<T> storage_;
  mutable std::shared_mutex mutex_;

 public:
  LockBasedStack() = default;
  LockBasedStack(const LockBasedStack&) = delete;
  LockBasedStack& operator=(const LockBasedStack&) = delete;

  void push(T value) {
    std::unique_lock lock(mutex_);
    storage_.push_back(std::move(value));
  }

  [[nodiscard]] bool tryPop(T& result) {
    std::unique_lock lock(mutex_);
    if (storage_.empty()) {
      return false;
    }
    result = storage_.back();
    storage_.pop_back();
    return true;
  }

  [[nodiscard]] bool peek(T& result) const {
    std::shared_lock lock(mutex_);
    if (storage_.empty()) {
      return false;
    }
    result = storage_.back();
    return true;
  }

  [[nodiscard]] bool empty() const {
    std::shared_lock lock(mutex_);
    return storage_.empty();
  }

  [[nodiscard]] std::size_t size() const {
    std::shared_lock lock(mutex_);
    return storage_.size();
  }
};

struct WorkloadProfile {
  std::string label;
  double writeProbability;  // 0..1 where 1 means always push/pop
};

struct ScenarioResult {
  int threads;
  int operations;
  std::string workload;
  double lockBasedMs;
  double lockFreeMs;

  [[nodiscard]] double speedup() const { return lockFreeMs == 0.0 ? 0.0 : lockBasedMs / lockFreeMs; }
};

struct SpeedAggregate {
  double total{0.0};
  int samples{0};

  void add(double value) {
    total += value;
    ++samples;
  }

  [[nodiscard]] double average() const { return samples == 0 ? 0.0 : total / static_cast<double>(samples); }
};

class BenchmarkHarness {
 public:
  [[nodiscard]] std::vector<ScenarioResult> runAll() const {
    std::vector<ScenarioResult> rows;
    rows.reserve(kThreadCounts.size() * kOperationCounts.size() * kWorkloads.size());

    for (const int operations : kOperationCounts) {
      for (const int threads : kThreadCounts) {
        for (const auto& workload : kWorkloads) {
          rows.push_back(runScenario(operations, threads, workload));
        }
      }
    }

    return rows;
  }

  static void printTable(std::span<const ScenarioResult> rows) {
    std::cout << "\n=== Benchmark Matrix (milliseconds) ===" << std::endl;
    std::cout << std::left << std::setw(8) << "Thr" << std::setw(12) << "Ops"
              << std::setw(22) << "Workload" << std::setw(16) << "Lock-Based"
              << std::setw(16) << "Lock-Free" << std::setw(10) << "Speedup" << std::endl;
    std::cout << std::string(84, '-') << std::endl;

    std::cout << std::fixed << std::setprecision(3);
    for (const auto& row : rows) {
      std::cout << std::left << std::setw(8) << row.threads << std::setw(12) << row.operations
                << std::setw(22) << row.workload << std::setw(16) << row.lockBasedMs
                << std::setw(16) << row.lockFreeMs << std::setw(10) << row.speedup() << std::endl;
    }
  }

  static void printInsights(std::span<const ScenarioResult> rows) {
    std::map<int, SpeedAggregate> byThreads;
    std::map<std::string, SpeedAggregate> byWorkload;
    int lockFreeWins = 0;
    int lockBasedWins = 0;
    const ScenarioResult* best = nullptr;
    const ScenarioResult* worst = nullptr;

    for (const auto& row : rows) {
      const double speed = row.speedup();
      byThreads[row.threads].add(speed);
      byWorkload[row.workload].add(speed);
      if (speed > 1.0) {
        ++lockFreeWins;
      } else {
        ++lockBasedWins;
      }

      if (best == nullptr || speed > best->speedup()) {
        best = &row;
      }
      if (worst == nullptr || speed < worst->speedup()) {
        worst = &row;
      }
    }

    std::cout << "\n=== Speedup by Thread Count (lock-based / lock-free) ===" << std::endl;
    for (const auto& [threads, aggregate] : byThreads) {
      std::cout << "Threads " << std::setw(2) << threads << ": avg speedup " << std::setw(6) << std::setprecision(3)
                << aggregate.average() << std::endl;
    }

    std::cout << "\n=== Speedup by Workload Mix ===" << std::endl;
    for (const auto& [label, aggregate] : byWorkload) {
      std::cout << std::left << std::setw(22) << label << ": avg speedup " << std::setw(6) << std::setprecision(3)
                << aggregate.average() << std::endl;
    }

    if (best && worst) {
      std::cout << "\nLock-free faster in " << lockFreeWins << " scenarios; lock-based faster in " << lockBasedWins
                << "." << std::endl;
      std::cout << "Best lock-free gain: " << best->workload << ", " << best->threads << " threads, "
                << best->operations << " ops => speedup " << best->speedup() << "x." << std::endl;
      std::cout << "Lock-based advantage: " << worst->workload << ", " << worst->threads << " threads, "
                << worst->operations << " ops => speedup " << worst->speedup() << "x." << std::endl;
    }
  }

 private:
  static ScenarioResult runScenario(int operations, int threads, const WorkloadProfile& workload) {
    ScenarioResult result{threads, operations, workload.label, 0.0, 0.0};
    result.lockBasedMs = measureStack<LockBasedStack<int>>(operations, threads, workload);
    result.lockFreeMs = measureStack<LockFreeStack<int>>(operations, threads, workload);
    return result;
  }

  template <typename Stack>
  static double measureStack(int operations, int threadCount, const WorkloadProfile& workload) {
    Stack stack;
    return executeWorkload(stack, operations, threadCount, workload);
  }

  template <typename Stack>
  static double executeWorkload(Stack& stack, int operations, int threadCount, const WorkloadProfile& workload) {
    using Clock = std::chrono::steady_clock;
    std::vector<std::jthread> workers;
    workers.reserve(threadCount);

    Clock::time_point startTime{};
    std::barrier startGate(threadCount, [&]() noexcept { startTime = Clock::now(); });

    const int baseOpsPerThread = operations / threadCount;
    const int remainder = operations % threadCount;

    for (int idx = 0; idx < threadCount; ++idx) {
      const int opsForThread = baseOpsPerThread + (idx < remainder ? 1 : 0);
      workers.emplace_back([&, opsForThread, idx](std::stop_token) {
        std::mt19937 rng(static_cast<unsigned int>(idx * 9973u + opsForThread + operations));
        std::bernoulli_distribution isWrite(workload.writeProbability);
        std::bernoulli_distribution isPush(0.5);
        int value = 0;

        startGate.arrive_and_wait();

        for (int op = 0; op < opsForThread; ++op) {
          if (isWrite(rng)) {
            if (isPush(rng)) {
              stack.push(idx * 1'000'000 + op);
            } else {
              (void)stack.tryPop(value);
            }
          } else {
            (void)stack.peek(value);
          }
        }
      });
    }

    for (auto& worker : workers) {
      worker.join();
    }

    const auto endTime = Clock::now();
    return std::chrono::duration<double, std::milli>(endTime - startTime).count();
  }

  static constexpr std::array<int, 5> kThreadCounts{1, 2, 4, 8, 16};
  static constexpr std::array<int, 3> kOperationCounts{1000, 10000, 100000};
  static constexpr std::array<WorkloadProfile, 3> kWorkloads{
      WorkloadProfile{"Write-heavy 80/20", 0.8},
      WorkloadProfile{"Balanced 50/50", 0.5},
      WorkloadProfile{"Read-heavy 20/80", 0.2},
  };
};

int main() {
  BenchmarkHarness harness;
  const auto results = harness.runAll();
  BenchmarkHarness::printTable(results);
  BenchmarkHarness::printInsights(results);
  std::cout << "\nBenchmarking complete." << std::endl;
  return 0;
}
