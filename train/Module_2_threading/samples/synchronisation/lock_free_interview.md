# Lock-Free Stack Interview Simulation (mdanki Ready)

## Card 01 – Objective
Q: What performance study does `lock_free_ai.cpp` execute?
A: It measures lock-based versus lock-free stack implementations across thread counts, operation volumes, and read/write mixes, printing millisecond timings plus derived speedups to highlight when each strategy excels.

## Card 02 – Lock-Free Technique
Q: Which lock-free algorithm underpins the stack implementation?
A: A Treiber-style singly linked stack driven by `std::atomic<Node*>` head updates with `compare_exchange_weak`, ensuring pushes and pops proceed via CAS without OS-level locks.

## Card 03 – ABA Mitigation
Q: How does the stack sidestep ABA-induced memory corruption?
A: Instead of deleting nodes immediately, popped nodes are placed on a secondary lock-free retired list, with reclamation deferred to stack destruction when no mutating threads remain, eliminating reuse of reclaimed memory during concurrent accesses.

## Card 04 – Memory Safety
Q: Why is the deferred reclamation scheme considered safe for these benchmarks?
A: Worker threads terminate before `LockFreeStack` destruction, so every retired node remains valid until all concurrent operations finish, guaranteeing no thread dereferences freed memory.

## Card 05 – Lock-Based Baseline
Q: Describe the baseline structure used for comparison.
A: `LockBasedStack` wraps a `std::vector<T>` with `std::shared_mutex`, granting shared locks for read-only peeks and exclusive locks for push/pop so it represents a modern mutex-guarded design.

## Card 06 – Workload Profiles
Q: How are different read/write ratios modeled?
A: Three `WorkloadProfile`s—write-heavy 80/20, balanced 50/50, and read-heavy 20/80—parameterize a Bernoulli distribution that decides whether each operation mutates or merely peeks.

## Card 07 – Thread Scaling
Q: Which thread counts are exercised automatically?
A: A constexpr array enumerates {1, 2, 4, 8, 16} threads, letting the harness reveal scaling behavior from single-threaded to high-contention regimes.

## Card 08 – Operation Counts
Q: How does the program vary total work per scenario?
A: Each scenario iterates over 1,000, 10,000, and 100,000 logical operations, dividing them evenly among worker threads (with remainder balancing) to inspect both short bursts and long steady-state loads.

## Card 09 – Start Synchronization
Q: How is measurement start synchronized across workers?
A: A `std::barrier` gates all worker threads; its completion function captures the shared start timestamp the moment the final thread arrives, ensuring consistent timing windows.

## Card 10 – RNG Usage
Q: What randomization technique feeds the workload decisions?
A: Each worker owns a seeded `std::mt19937` RNG driving two `std::bernoulli_distribution`s—one for write-vs-read, another for push-vs-pop—to avoid accidental synchronization between threads.

## Card 11 – Metric Reporting
Q: What columns appear in the benchmark matrix output?
A: Threads, operations, workload label, lock-based duration, lock-free duration, and computed speedup (lock-based divided by lock-free) are rendered using fixed-width formatting for easy scanning.

## Card 12 – Insight Aggregation
Q: How are insights beyond raw tables generated?
A: Speedups are aggregated by thread count and workload label via `SpeedAggregate`, and summaries identify how many scenarios favor lock-free vs lock-based along with best/worst cases.

## Card 13 – Size Tracking
Q: Why does the lock-free stack still maintain an atomic size counter?
A: The `size_` counter gives cheap visibility into stack depth for debugging or potential future diagnostics without requiring traversal or additional locks.

## Card 14 – Peek Semantics
Q: How are read-only peeks handled without disturbing synchronization?
A: `peek` simply loads the atomic head with `memory_order_acquire` and copies the value if present; it never mutates the structure, so it remains wait-free for readers even under contention.

## Card 15 – CAS Ordering
Q: What memory orders are used for head updates and why?
A: Push uses release semantics to publish the new node before updating the head, while pop uses acquire-release to ensure it sees a fully initialized node and that subsequent reads of `next` are properly ordered.

## Card 16 – Retired List Mechanics
Q: How is the retired list updated without locks?
A: Each popped node is wrapped in a `RetiredNode` structure and inserted via another CAS loop on `retiredHead_`, so retirement itself stays lock-free and contention-friendly.

## Card 17 – Balanced Division of Work
Q: How does the harness handle operation counts that are not divisible by the thread count?
A: It distributes the remainder across the lowest-index threads, ensuring every operation is executed exactly once and that no worker idles prematurely.

## Card 18 – Deterministic Seeds
Q: Why is the RNG seed derived from thread index, operations, and workload size?
A: This makes each scenario repeatable (same number of operations yields the same sequence) while still differentiating threads so that contention patterns remain realistic.

## Card 19 – Result Interpretation
Q: How can the reported speedup values guide architectural choices?
A: Speedup greater than 1 indicates the lock-free design saves time, signaling that high-contention or write-heavy workloads benefit; values below 1 highlight scenarios where mutex overhead is negligible and the simpler lock-based stack may be preferable.

## Card 20 – Extensibility
Q: What would it take to benchmark other data structures with this harness?
A: Implement the same `push`, `tryPop`, and `peek` interface on new stack-like containers and plug them into `measureStack`, leveraging the existing workload generator and reporting pipeline.

## Justification for the Selected Solution
This solution models real-world decision making by benchmarking concrete implementations rather than theorizing. The Treiber-based lock-free stack stays compliant with modern C++20 style by using RAII-friendly helpers, CAS loops with explicit memory orders, and a deferred reclamation list that guarantees memory safety without resorting to OS locks. Pairing it with a `std::shared_mutex` baseline, randomized workload generator, and aggregated insight reporting lets engineers see exactly which thread counts and read/write mixes favor each strategy. That combination of correctness, observability, and reproducibility is why this approach is the best way to study lock-free adoption in performance-critical systems.
