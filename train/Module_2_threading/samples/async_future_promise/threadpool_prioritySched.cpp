/*
Practice
Using the code below, create a load testing scenario that:
* Submits tasks with different priorities and processing times
* Monitors thread pool scaling behavior under varying loads
* Compares performance with different min/max thread configurations
* Analyzes queue dynamics and thread utilization patterns
* Test with burst loads, sustained loads, and mixed priority scenarios.

✅ Success Checklist

* Higher priority tasks are processed before lower priority tasks
* Thread pool scales up during high load periods
* Performance statistics accurately reflect system behavior
* System maintains stability under extreme load conditions
*/

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <ranges>
#include <thread>

#include "DynamicThreadPool.h"
#include "logging.h"

constexpr auto MAX_SIM = 5;

auto getWork(int val) {
  auto work = [val]() {
    // logSync(std::cout, "\t processing Task: ", val);
    std::this_thread::sleep_for(std::chrono::milliseconds(val));
  };
  return std::move(work);
}
void runTaskGenerator(DynamicThreadPool& dtp, int sim_id, int task_count) {
  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int> dist(get_val(TaskPriority::LOW), get_val(TaskPriority::CRITICAL));
  for (auto val : std::ranges::iota_view(1, task_count + 1)) {
    int taskId = sim_id * 1000 + val;
    auto priority = static_cast<TaskPriority>(dist(rng) % (get_val(TaskPriority::CRITICAL) + 1));
    auto now = std::chrono::steady_clock::now();
    auto function = getWork(taskId);
    dtp.submit(std::move(function), priority, std::to_string(taskId));
  }
}

int main() {
  for (auto val : std::ranges::iota_view(2, MAX_SIM + 1)) {
    DynamicThreadPool dtp;
    logSync(std::cout, "starting with events: " + std::to_string(100 * val));
    runTaskGenerator(dtp, val, 100 * val);
    std::jthread monitor([&dtp](std::stop_token stopToken) {
      while (dtp.getQueueSize() != 0) {
        auto qsize = dtp.getQueueSize();
        logSync(std::cerr, "Qsize: " + std::to_string(qsize));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    });
    while (dtp.getQueueSize() != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    dtp.printStats();
    logSync(std::cout, "\n -------------------  processing complete -------------------------- \n");

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  }
  return 0;
}