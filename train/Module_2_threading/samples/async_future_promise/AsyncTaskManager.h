#ifndef ASYNC_TASK_MANAGER_H
#define ASYNC_TASK_MANAGER_H

#include <atomic>
#include <chrono>
#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

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
    auto future = std::async(std::launch::async, [bound = std::move(callable)]() mutable -> T {
                    return std::invoke(bound);
                  }).share();

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
              std::cerr << "Task failed: " << ex.what() << '\n';
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

#endif  // ASYNC_TASK_MANAGER_H
