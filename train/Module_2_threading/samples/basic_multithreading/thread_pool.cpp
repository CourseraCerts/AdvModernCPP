/*
Extend your system to implement proper thread lifecycle management with a custom thread guard class and exception
handling. 🔍 Practice Using the code below, create a system that: Divides the collected data into segments for parallel
risk analysis Uses ThreadGuard to ensure all threads complete properly Handles exceptions that might occur during
processing Test with both valid data and empty data segments to observe exception handling.

✅ Success Checklist
ThreadGuard automatically joins threads when going out of scope
Exception handling prevents thread crashes
All data segments are processed correctly
Resource cleanup occurs even when exceptions are thrown
*/
#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

// shared variables

std::mutex dataMutex;
std::vector<double> sharedData;
std::atomic<int> processedCount{0};

class ThreadGuard {
 private:
  std::thread& thread_;

 public:
  explicit ThreadGuard(std::thread& t) : thread_(t) {}

  // move constructible so it can be stored in containers like std::vector
  ThreadGuard(ThreadGuard&& other) noexcept : thread_(other.thread_) {}
  // references can't be rebound, so disable move assignment
  ThreadGuard& operator=(ThreadGuard&&) = delete;

  ~ThreadGuard() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  // Prevent copying
  ThreadGuard(const ThreadGuard&) = delete;
  ThreadGuard& operator=(const ThreadGuard&) = delete;
};

void riskAnalyzer(int analyzerId, const std::vector<double>& data, double& result) {
  try {
    if (data.empty()) {
      throw std::runtime_error("No data to analyze");
    }

    double sum = 0.0;
    for (double value : data) {
      sum += value * value;  // Risk calculation simulation
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    result = sum / data.size();
    std::cout << "Analyzer " << analyzerId << " calculated risk: " << result << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "Analyzer " << analyzerId << " error: " << e.what() << std::endl;
    result = -1.0;
  }
}

int test() {
  std::vector<std::thread> threads;
  std::vector<double> result(3);
  std::vector<ThreadGuard> tg;
  std::vector<std::vector<double>> segments = {{1.0, 2.0, 3.0, 4.0, 5.0}, {6.0, 7.0, 8.0}, {}};
  for (int i = 0; i < 3; ++i) {
    threads.emplace_back(riskAnalyzer, i, std::ref(segments[i]), std::ref(result[i]));
  }
  for (auto& t : threads) {
    tg.emplace_back(t);
  }
  return 0;
}
int main() {
  auto result = test();
  std::cout << "Done" << std::endl;
  return result;
}