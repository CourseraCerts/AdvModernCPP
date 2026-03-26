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
*/
#include <chrono>
#include <cmath>
#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>

// Get completion status of all tasks
struct TaskStatus {
  size_t completed = 0;
  size_t pending = 0;
  size_t failed = 0;
};
template <typename Stream>
Stream& operator<<(Stream& stream, const TaskStatus& ts) {
  stream << "\t Pending: " << ts.pending << " | completed: " << ts.completed << " | failed: " << ts.failed;
  return stream;
}

namespace {
using namespace std::chrono_literals;
constexpr int kDatasetCount = 20;
constexpr int kComplexityBase = 100;
constexpr int kComplexityStep = 5;
constexpr std::size_t kMaxRetries = 2;
constexpr std::chrono::milliseconds kRetryDelay = 75ms;
constexpr std::chrono::milliseconds kMonitorInterval = 50ms;
constexpr std::chrono::milliseconds kResultPollInterval = 150ms;

std::mutex gLogMutex;

template <typename... Args>
void logSync(std::ostream& stream, Args&&... args) {
  std::scoped_lock lock(gLogMutex);
  (stream << ... << args);
  stream.flush();
}
}  // namespace

template <typename T>
class AsyncTaskManager {
 private:
  std::vector<std::shared_future<T>> activeTasks_;
  mutable std::mutex tasksMutex_;
  TaskStatus taskStatus_;

 public:
  template <typename Func, typename... Args>
  [[nodiscard]] std::shared_future<T> submitTask(Func&& func, Args&&... args) {
    auto callable = std::bind_front(std::forward<Func>(func), std::forward<Args>(args)...);
    auto future = std::async(std::launch::async,
                             [bound = std::move(callable)]() mutable -> T { return std::invoke(bound); })
                      .share();

    {
      std::scoped_lock lock(tasksMutex_);
      activeTasks_.push_back(future);
      ++taskStatus_.pending;
    }

    return future;
  }

  [[nodiscard]] std::vector<T> waitForAll(std::chrono::milliseconds pollInterval = std::chrono::milliseconds{100}) {
    std::vector<T> results;

    while (true) {
      bool finished = false;
      {
        std::scoped_lock lock(tasksMutex_);
        std::vector<std::shared_future<T>> stillPending;

        for (auto& future : activeTasks_) {
          if (future.wait_for(std::chrono::milliseconds{0}) == std::future_status::ready) {
            try {
              results.push_back(future.get());
              ++taskStatus_.completed;
            } catch (const std::exception& ex) {
              ++taskStatus_.failed;
              logSync(std::cerr, "Task failed: ", ex.what(), '\n');
            }

            if (taskStatus_.pending > 0) {
              --taskStatus_.pending;
            }
          } else {
            stillPending.push_back(future);
          }
        }

        activeTasks_ = std::move(stillPending);
        finished = activeTasks_.empty();
      }

      if (finished) {
        break;
      }

      std::this_thread::sleep_for(pollInterval);
    }

    return results;
  }

  [[nodiscard]] TaskStatus getStatus() const {
    std::scoped_lock lock(tasksMutex_);
    return taskStatus_;
  }
};

// Complex computation that might fail
struct DataAnalysis {
  int datasetId;
  double result;
  std::chrono::milliseconds processingTime;

  [[nodiscard]] static DataAnalysis processDataset(int id, int complexity) {
    thread_local std::mt19937 generator{std::random_device{}()};

    std::uniform_int_distribution<int> timeDist(100, 1'000);
    const auto processingTime = std::chrono::milliseconds{timeDist(generator)};
    std::this_thread::sleep_for(processingTime);

    std::uniform_real_distribution<double> failDist(0.0, 1.0);
    constexpr double failureProbability = 0.1;  // 10%
    if (failDist(generator) < failureProbability) {
      throw std::runtime_error("Dataset processing failed for ID " + std::to_string(id));
    }

    double result = 0.0;
    for (int i = 0; i < complexity * 1'000; ++i) {
      const double scaled = static_cast<double>(i) * 0.001;
      result += std::sin(scaled) * std::cos(scaled * static_cast<double>(id));
    }

    return DataAnalysis{id, result, processingTime};
  }
};

int main() {
  AsyncTaskManager<DataAnalysis> manager;

  for (int idx = 0; idx < kDatasetCount; ++idx) {
    const int complexity = kComplexityBase + (idx % 5) * kComplexityStep;

    [[maybe_unused]] auto future = manager.submitTask([idx, complexity] {
      for (std::size_t attempt = 0; attempt <= kMaxRetries; ++attempt) {
        try {
          return DataAnalysis::processDataset(idx, complexity);
        } catch (const std::exception& ex) {
          if (attempt == kMaxRetries) {
            logSync(std::cerr, "Dataset ", idx, " exhausted retries after error: ", ex.what(), '\n');
            throw;
          }

          logSync(std::clog, "Retrying dataset ", idx, " (attempt ", (attempt + 2), "/", (kMaxRetries + 1),
                  ") due to: ", ex.what(), '\n');
          std::this_thread::sleep_for(kRetryDelay * static_cast<int>(attempt + 1));
        }
      }

      throw std::runtime_error("Retries exhausted unexpectedly");
    });
  }

  std::jthread monitor([&manager](std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
      const auto status = manager.getStatus();
      logSync(std::cout, "\t status - completed: ", status.completed, " pending: ", status.pending,
              " failed: ", status.failed, '\n');

      if (status.pending == 0) {
        logSync(std::cout, "\t all tasks complete, stopping monitoring\n");
        break;
      }

      std::this_thread::sleep_for(kMonitorInterval);
    }
  });

  logSync(std::cout, "\t\t Waiting for results...\n");
  auto results = manager.waitForAll(kResultPollInterval);

  monitor.request_stop();

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
