/*
Practice
Using the code below, create a distributed analytics simulation that:
* Submits 20 datasets for processing with varying complexity levels
* Monitors task completion status in real-time
* Handles failures gracefully and retries failed tasks
* Aggregates results from successful computations
Experiment with different timeout values and failure rates to understand system behavior under stress.

✅ Success Checklist
* System submits and tracks multiple asynchronous tasks correctly
* Failed tasks are handled without crashing the system
* Results are aggregated only from successful computations
* Real-time status monitoring shows accurate task states

Implementation note:
The original version used std::bind to turn a callable plus arguments into a nullary functor that the
packaged_task executes. This lambda-based variant instead captures decayed copies of the callable and
its arguments, then replays them via std::apply. The lambda gives the compiler full visibility into the
closure layout (typically enabling better inlining/optimization) and avoids std::bind placeholder
indirection while matching its copy semantics, so the two approaches behave the same but the lambda is
clearer and often slightly more performant.
Concepts used:
* std::decay_t strips references/qualifiers to mirror std::bind's copy semantics so stored arguments own
  their state even if temporaries are passed in.
* Structured lambda init-capture stores the callable plus the tuple of arguments directly inside the
  closure for later execution.
* std::tuple + std::make_tuple hold heterogeneous arguments, preserving value categories when combined
  with std::apply.
* std::apply unpacks the tuple back into the callable when the packaged_task runs, avoiding manual index
  sequencing.
* std::packaged_task/std::future pair captures results or exceptions from the detached worker thread
  so the manager can retrieve them later.
*/
#include <chrono>
#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

template <typename T>
class AsyncTaskManager {
 private:
  std::vector<std::future<T>> activeTasks_;
  mutable std::mutex tasksMutex_;

 public:
  // Submit async task with custom executor
  template <typename Func, typename... Args>
  auto submitTask(Func&& func, Args&&... args) -> std::future<T> {
    // auto task =
    //     std::make_shared<std::packaged_task<T()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    // Create packaged_task using a lambda so argument forwarding mirrors std::bind's behavior
    auto task = std::make_shared<std::packaged_task<T()>>(
        [callable = std::decay_t<Func>(std::forward<Func>(func)),
         arguments = std::make_tuple(std::decay_t<Args>(std::forward<Args>(args))...)]() mutable {
          return std::apply(callable, arguments);
        });

    std::future<T> future = task->get_future();

    // Execute in separate thread
    std::thread([task]() {
      try {
        (*task)();
      } catch (...) {
        std::cerr << "\t\t Error: obtained in elipses" << std::endl;
        // Exception automatically captured by packaged_task
      }
    }).detach();

    {
      std::lock_guard<std::mutex> lock(tasksMutex_);
      activeTasks_.push_back(std::move(future));
    }

    return future;
  }

  // Wait for all tasks with timeout
  std::vector<T> waitForAll(std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)) {
    std::vector<T> results;

    std::lock_guard<std::mutex> lock(tasksMutex_);
    for (auto& future : activeTasks_) {
      try {
        if (future.wait_for(timeout) == std::future_status::ready) {
          results.push_back(future.get());
        } else {
          std::cout << "Task timed out" << std::endl;
        }
      } catch (const std::exception& e) {
        std::cout << "Task failed: " << e.what() << std::endl;
      }
    }

    activeTasks_.clear();
    return results;
  }

  // Get completion status of all tasks
  struct TaskStatus {
    size_t completed = 0;
    size_t pending = 0;
    size_t failed = 0;
  };

  TaskStatus getStatus() const {
    TaskStatus status;

    std::lock_guard<std::mutex> guard(tasksMutex_);
    for (const auto& future : activeTasks_) {
      auto taskStatus = future.wait_for(std::chrono::milliseconds(0));
      if (taskStatus == std::future_status::ready) {
        status.completed++;
      } else {
        status.pending++;
      }
    }

    return status;
  }
};

// Complex computation that might fail
struct DataAnalysis {
  int datasetId;
  double result;
  std::chrono::milliseconds processingTime;

  static DataAnalysis processDataset(int id, int complexity) {
    std::random_device rd;
    std::mt19937 gen(rd());

    // Simulate variable processing time
    std::uniform_int_distribution<> timeDist(100, 1000);
    auto processingTime = std::chrono::milliseconds(timeDist(gen));

    std::this_thread::sleep_for(processingTime);

    // Simulate potential failures
    std::uniform_int_distribution<> failDist(1, 100);
    if (failDist(gen) <= 10) {  // 10% failure rate
      throw std::runtime_error("Dataset processing failed for ID " + std::to_string(id));
    }

    // Complex mathematical computation
    double result = 0.0;
    for (int i = 0; i < complexity * 1000; ++i) {
      result += std::sin(i) * std::cos(i * id);
    }

    return DataAnalysis{id, result, processingTime};
  }
};

int main() {
  AsyncTaskManager<DataAnalysis> manager;

  // submit 20 tasks with varying complexity
  for (int idx = 0; idx < 20; ++idx) {
    int complexity = 100 + (idx % 5) * 5;
    manager.submitTask(DataAnalysis::processDataset, idx, complexity);
  }
  // monitor task completion status
  std::thread monitor([&manager] {
    for (int off = 0; off < 20; ++off) {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      auto status = manager.getStatus();
      std::cout << "\t status - completed: " << status.completed << " pending: " << status.pending << std::endl;
    }
  });
  // collect results
  auto results = manager.waitForAll();
  monitor.join();

  // Aggregate successful results
  double totalResult = 0.0;
  std::chrono::milliseconds totalTime{0};

  for (const auto& analysis : results) {
    totalResult += analysis.result;
    totalTime += analysis.processingTime;
  }
  std::cout << " Analysis complete - " << results.size() << " successful datasets" << std::endl;
  std::cout << " Total computation result: " << totalResult << std::endl;
  std::cout << " Total processing time: " << totalTime.count() << " ms" << std::endl;

  return 0;
}
