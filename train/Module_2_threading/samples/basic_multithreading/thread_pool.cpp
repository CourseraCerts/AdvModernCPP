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
void dataProcessor(int processorId, int itemCount) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(1.0, 100.0);

  for (int i = 0; i < itemCount; ++i) {
    double value = dis(gen);

    // Thread-safe data insertion
    {
      std::lock_guard<std::mutex> lock(dataMutex);
      sharedData.push_back(value);
    }

    processedCount.fetch_add(1);

    // Simulate processing time
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  std::cout << "Processor " << processorId << " completed processing " << itemCount << " items\n";
}

int main() {
  std::vector<std::thread> threads;
  for (int i = 0; i < 5; ++i) {
    threads.emplace_back(dataProcessor, i, 20);
  }
  for (auto& t : threads) {
    t.join();
  }
  std::cout << "total procesed" << processedCount.load() << std::endl;
  std::cout << "DAta size: " << sharedData.size() << std::endl;
  for (auto& val : sharedData) {
    std::cout << val << " ";
  }
  std::cout << "\n";

  return 0;
}