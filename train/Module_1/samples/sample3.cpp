// Associative container performance characteristics
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <atomic>

struct User {
  int userId;
  std::string username;
  std::string email;

  User() = default;
  User(int id, const std::string& name, const std::string& mail) : userId(id), username(name), email(mail) {}
};

class AssociativeContainerBenchmark {
 private:
  static const int USER_COUNT = 100000;
  std::vector<User> testUsers;
  std::vector<int> lookupIds;
  std::mt19937 rng{std::random_device{}()};

  template <typename Func>
  double measureTime(Func&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / 1000.0;
  }

  void generateTestData() {
    testUsers.reserve(USER_COUNT);
    lookupIds.reserve(USER_COUNT / 10);

    // Generate test users
    for (int i = 1; i <= USER_COUNT; ++i) {
      testUsers.emplace_back(i, "user" + std::to_string(i), "user" + std::to_string(i) + "@company.com");
    }

    // Generate random lookup IDs
    std::uniform_int_distribution<int> idDist(1, USER_COUNT);
    for (int i = 0; i < USER_COUNT / 10; ++i) {
      lookupIds.push_back(idDist(rng));
    }

    // Shuffle for realistic access pattern
    std::shuffle(testUsers.begin(), testUsers.end(), rng);
    std::shuffle(lookupIds.begin(), lookupIds.end(), rng);
  }

 public:
  AssociativeContainerBenchmark() { generateTestData(); }

  void benchmarkMapContainers() {
    std::cout << "\n=== ASSOCIATIVE CONTAINER PERFORMANCE COMPARISON ===" << std::endl;
    std::cout << "Dataset: " << USER_COUNT << " users with " << lookupIds.size() << " lookups\n" << std::endl;

    std::map<int, User> orderedMap;
    std::unordered_map<int, User> unorderedMap;

    std::cout << std::fixed << std::setprecision(3);
    std::cout << std::setw(25) << "Operation" << std::setw(20) << "std::map (ms)" << std::setw(20)
              << "std::unordered_map (ms)" << std::endl;
    std::cout << std::string(65, '-') << std::endl;

    // Test insertion performance
    double mapInsertTime = measureTime([&]() {
      for (const auto& user : testUsers) {
        orderedMap[user.userId] = user;
      }
    });

    double unorderedMapInsertTime = measureTime([&]() {
      for (const auto& user : testUsers) {
        unorderedMap[user.userId] = user;
      }
    });

    std::cout << std::setw(25) << "Insertion" << std::setw(20) << mapInsertTime << std::setw(20)
              << unorderedMapInsertTime << std::endl;

    // Test lookup performance
    double mapLookupTime = measureTime([&]() {
      std::atomic<int> found{0};
      for (int id : lookupIds) {
        auto it = orderedMap.find(id);
        if (it != orderedMap.end()) {
          found.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });

    double unorderedMapLookupTime = measureTime([&]() {
      std::atomic<int> found{0};
      for (int id : lookupIds) {
        auto it = unorderedMap.find(id);
        if (it != unorderedMap.end()) {
          found.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });

    std::cout << std::setw(25) << "Random Lookup" << std::setw(20) << mapLookupTime << std::setw(20)
              << unorderedMapLookupTime << std::endl;

    // Test ordered traversal
    double mapTraversalTime = measureTime([&]() {
      std::atomic<int> count{0};
      for (const auto& pair : orderedMap) {
        count.fetch_add(pair.first, std::memory_order_relaxed);
      }
    });

    double unorderedMapTraversalTime = measureTime([&]() {
      std::atomic<int> count{0};
      for (const auto& pair : unorderedMap) {
        count.fetch_add(pair.first, std::memory_order_relaxed);
      }
    });

    std::cout << std::setw(25) << "Full Traversal" << std::setw(20) << mapTraversalTime << std::setw(20)
              << unorderedMapTraversalTime << std::endl;

    std::cout << std::string(65, '-') << std::endl;
    std::cout << "Map Performance Analysis:" << std::endl;
    std::cout << "• std::map: O(log n) operations, maintains sorted order" << std::endl;
    std::cout << "• std::unordered_map: O(1) average operations, no order guarantee" << std::endl;
    std::cout << "• Choose map when you need sorted data or range queries" << std::endl;
    std::cout << "• Choose unordered_map for fastest single-element operations" << std::endl;
  }

  void benchmarkSetContainers() {
    std::cout << "\n=== SET CONTAINER PERFORMANCE COMPARISON ===" << std::endl;

    // Create unique IDs for set operations
    std::vector<int> uniqueIds;
    for (int i = 1; i <= USER_COUNT; ++i) {
      uniqueIds.push_back(i);
    }
    std::shuffle(uniqueIds.begin(), uniqueIds.end(), rng);

    std::set<int> orderedSet;
    std::unordered_set<int> unorderedSet;

    std::cout << std::setw(25) << "Operation" << std::setw(20) << "std::set (ms)" << std::setw(20)
              << "std::unordered_set (ms)" << std::endl;
    std::cout << std::string(65, '-') << std::endl;

    // Test insertion performance
    double setInsertTime = measureTime([&]() {
      for (int id : uniqueIds) {
        orderedSet.insert(id);
      }
    });

    double unorderedSetInsertTime = measureTime([&]() {
      for (int id : uniqueIds) {
        unorderedSet.insert(id);
      }
    });

    std::cout << std::setw(25) << "Insertion" << std::setw(20) << setInsertTime << std::setw(20)
              << unorderedSetInsertTime << std::endl;

    // Test lookup performance
    double setLookupTime = measureTime([&]() {
      std::atomic<int> found{0};
      for (int id : lookupIds) {
        if (orderedSet.count(id) > 0) {
          found.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });

    double unorderedSetLookupTime = measureTime([&]() {
      std::atomic<int> found{0};
      for (int id : lookupIds) {
        if (unorderedSet.count(id) > 0) {
          found.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });

    std::cout << std::setw(25) << "Lookup" << std::setw(20) << setLookupTime << std::setw(20) << unorderedSetLookupTime
              << std::endl;

    // Demonstrate ordered set advantage: range queries
    double rangeQueryTime = measureTime([&]() {
      std::atomic<int> count{0};
      // Count elements between 25000 and 75000
      auto lower = orderedSet.lower_bound(25000);
      auto upper = orderedSet.upper_bound(75000);
      for (auto it = lower; it != upper; ++it) {
        count.fetch_add(1, std::memory_order_relaxed);
      }
    });

    std::cout << std::setw(25) << "Range Query (25k-75k)" << std::setw(20) << rangeQueryTime << std::setw(20)
              << "Not Available" << std::endl;

    std::cout << std::string(65, '-') << std::endl;
    std::cout << "Set Performance Analysis:" << std::endl;
    std::cout << "• std::set: Supports efficient range queries and ordered iteration" << std::endl;
    std::cout << "• std::unordered_set: Fastest for simple membership testing" << std::endl;
  }
};

int main() {
  std::cout << "STL ASSOCIATIVE CONTAINER PERFORMANCE ANALYSIS" << std::endl;
  std::cout << "==============================================" << std::endl;

  AssociativeContainerBenchmark benchmark;
  benchmark.benchmarkMapContainers();
  benchmark.benchmarkSetContainers();

  return 0;
}