# Producer-Consumer Interview Simulation (mdanki Ready)

## Card 01 – Architecture Overview
Q: How does the new priority-driven producer-consumer architecture move a task from creation to completion?
A: Producers described by `ProducerProfile` instances create timestamped `Task` objects, push them into the `PriorityTaskQueue`, consumers pop the highest-priority eligible task, and `TaskProcessor` instances execute the payload while metrics and monitoring threads observe the flow.

## Card 02 – Priority Enforcement
Q: What guarantees that urgent work always jumps ahead of background traffic in this design?
A: Tasks are stored in a `std::priority_queue` with a comparator that orders by priority and timestamp, so the queue always surfaces the most urgent and oldest eligible task first regardless of producer source.

## Card 03 – High-Priority Consumers
Q: How do specialist consumers avoid getting stuck behind lower-priority items when the queue contains mixed work?
A: `PriorityTaskQueue::pop` accepts a `minPriority` threshold and the condition variable only wakes those consumers when a qualifying task exists or a shutdown occurs, preventing busy loops on ineligible tasks.

## Card 04 – General Consumers
Q: Why keep generalist consumers in addition to high-priority-only workers?
A: General consumers keep the pipeline drained by handling any priority, which prevents buildup of low-importance work and ensures the high-priority-only workers remain focused on urgent spikes.

## Card 05 – Randomized Workload Generation
Q: How is realistic traffic modeled for interviews or load tests?
A: Each producer owns a `std::discrete_distribution` seeded RNG, so its weighted `priorityWeights` create repeatable yet varied mixes of LOW/NORMAL/HIGH/CRITICAL tasks and a separate delay distribution shapes burstiness.

## Card 06 – Monitoring Signals
Q: What observability hooks exist to debug live behavior?
A: A dedicated `monitorQueue` `std::jthread` prints queue depth, waiting consumer counts, and rolling average wait times at configurable intervals, enabling quick diagnosis of backpressure.

## Card 07 – Wait-Time Accounting
Q: How are consumer wait times captured without locks around every measurement?
A: The queue tracks per-pop wait durations via relaxed atomics (`totalWaitNanos_` and `waitSamples_`), so the monitoring thread can report averages without stopping work.

## Card 08 – Graceful Shutdown Flow
Q: Walk through the shutdown sequence that still drains the queue.
A: Simulations first request all producers to stop, wait until `queue.size()` reaches zero, call `queue.shutdown()` to wake consumers, request the consumers to stop, and finally let each `std::jthread` destructor join so that every remaining task is processed before exit.

## Card 09 – Use of `std::jthread`
Q: Why are `std::jthread`s preferred here over legacy `std::thread`s?
A: `std::jthread` automatically joins on scope exit and propagates `std::stop_token`s into thread functions, so cancellation is explicit and memory-safe without manual `join()` bookkeeping.

## Card 10 – TaskProcessor Responsibilities
Q: What is encapsulated inside `TaskProcessor::processTask` and why keep it separated?
A: It prints diagnostic info, simulates priority-dependent latency, and increments per-processor counters, making it easy to swap in domain-specific work without touching scheduling primitives.

## Card 11 – Load Profiles
Q: How do the simulations cover “various load conditions and priority distributions” from the requirements?
A: `SimulationProfile` bundles producer mixes, consumer counts, run duration, and monitor cadence; `main` iterates three distinct profiles—balanced, latency-sensitive, and throughput stress—to validate behavior under different mixes.

## Card 12 – Preventing Starvation
Q: What keeps low-priority tasks from starving when urgent work is constant?
A: The presence of general consumers combined with FIFO order within the same priority (timestamp tie-breaker) ensures that even under high priority load, older low-priority tasks eventually bubble up and are handled.

## Card 13 – Metrics Export
Q: Where would you instrument additional telemetry if you needed Prometheus/Grafana integration?
A: `monitorQueue` already aggregates core stats, so extending it to emit structured metrics or pushing the aggregated `SimulationSummary` data would provide scrape-ready counters without redesigning the queue.

## Card 14 – Adding More Producers
Q: How does adding a new producer affect synchronization concerns?
A: Producers only interact with `PriorityTaskQueue::push`, which locks internally and notifies consumers, so adding more producers merely increases contention on that mutex without requiring new coordination primitives.

## Card 15 – Extending Consumer Types
Q: If a product team wants medium-priority specialists, what minimal change is required?
A: Instantiate more consumers with `minPriority = Priority::NORMAL` and provide a matching `TaskProcessor`; the queue already honors arbitrary thresholds through the predicate-based pop.

## Card 16 – Backpressure Diagnosis
Q: During an interview, how would you explain diagnosing long wait times?
A: Watch the monitor output for rising `pending` counts and `avgWaitMs`; if they spike, inspect which processors’ `processedTasks` plateau to decide whether to scale consumers or rebalance producer weights.

## Card 17 – Deterministic Testing
Q: How can this setup aid deterministic unit or soak tests?
A: Because each producer owns its RNG, you can seed them explicitly for reproducible sequences or replace them with scripted task feeds while keeping the same queue/consumer logic.

## Card 18 – Memory Safety Considerations
Q: How is lifetime management handled for processors referenced by long-running threads?
A: Each `TaskProcessor` lives behind a `std::unique_ptr`; the raw pointer captured by the consumer thread stays valid because the owning unique_ptr outlives all consumer threads and is destroyed only after they stop.

## Card 19 – Timeout Behavior
Q: What happens when a consumer’s timed pop expires without finding work?
A: `pop` returns `false`, the consumer checks `queue.isShutdown()`, and if the system is still live it loops again, so no spurious exits occur while also preventing infinite blocking when shutting down.

## Card 20 – Simulation Output Usage
Q: How are the per-simulation summaries useful for interviews?
A: `SimulationSummary` holds produced counts, per-consumer throughput, and average waits, giving interviewees concrete evidence of scaling behavior and a starting point for optimization discussions.

## Justification for the Selected Solution
The implemented system represents a best-practice answer because it unifies fairness and responsiveness: a priority queue enforces deterministic ordering, predicate-aware `pop` calls prevent busy loops for specialist consumers, and `std::jthread` plus `std::stop_token` simplify cancellation safety. Monitoring and metrics are built-in through lightweight atomics, giving immediate visibility into queue depth and wait times without external tooling. Multiple simulation profiles stress the design under balanced, latency-sensitive, and throughput-heavy workloads, demonstrating that the approach generalizes without code changes. Altogether this architecture offers precise control, observability, and graceful shutdown semantics with modern C++20 idioms, which is what interviewers typically look for in production-ready concurrency solutions.
