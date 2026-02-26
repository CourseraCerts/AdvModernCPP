/*
You're developing a data analytics platform that processes customer transaction data from multiple sources
simultaneously. Each data source runs on a separate thread to maximize throughput. 🔍 Practice Run the code below and
observe the output. Think about: How does std::lock_guard ensure thread safety when accessing sharedData? Why is
std::atomic used for processedCount instead of a regular integer? Modify the code to create 5 threads, each processing
20 data items. Test your changes and observe: How the total processed count matches the expected value (100) Whether the
output messages from different threads appear in order

✅ Success Checklist
All threads complete without data races or corruption
The atomic counter shows the correct total number of processed items
Shared vector contains data from all threads without missing entries
*/
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

std::mutex dataMutex;
std::vector<double> sharedData;
std::atomic<int> processedCount{0};

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