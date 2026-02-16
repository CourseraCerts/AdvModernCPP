#include <algorithm>
#include <chrono>
#include <deque>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <random>
#include <vector>

class IteratorCategoryAnalysis {
 private:
  static const int DATA_SIZE = 100000;
  std::vector<int> vectorData;
  std::list<int> listData;
  std::forward_list<int> forwardListData;
  std::deque<int> dequeData;

  template <typename Func>
  double measureTime(Func&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / 1000.0;  // Convert to milliseconds
  }

 public:
  void generateTestData() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 1000);

    // Generate consistent data for all containers
    std::vector<int> sourceData;
    sourceData.reserve(DATA_SIZE);
    for (int i = 0; i < DATA_SIZE; ++i) {
      sourceData.push_back(dist(gen));
    }

    // Initialize all containers with same data
    vectorData = sourceData;
    listData.assign(sourceData.begin(), sourceData.end());
    forwardListData.assign(sourceData.begin(), sourceData.end());
    dequeData.assign(sourceData.begin(), sourceData.end());

    std::cout << "Generated " << DATA_SIZE << " test elements across all containers" << std::endl;
  }

  void demonstrateRandomAccessIterators() {
    std::cout << "\n=== RANDOM ACCESS ITERATORS (vector, deque) ===" << std::endl;

    // Demonstrate vector random access capabilities
    std::cout << "Vector Random Access Capabilities:" << std::endl;

    // Direct indexing
    std::cout << "  Direct access [5]: " << vectorData[5] << std::endl;
    std::cout << "  Iterator arithmetic [begin() + 100]: " << *(vectorData.begin() + 100) << std::endl;

    // Iterator comparisons
    auto it1 = vectorData.begin() + 10;
    auto it2 = vectorData.begin() + 50;
    std::cout << "  Iterator comparison (it1 < it2): " << (it1 < it2 ? "true" : "false") << std::endl;
    std::cout << "  Distance between iterators: " << std::distance(it1, it2) << std::endl;

    // Sorting (requires random access)
    std::vector<int> sortableData(vectorData.begin(), vectorData.begin() + 10000);
    double sortTime = measureTime([&]() { std::sort(sortableData.begin(), sortableData.end()); });

    std::cout << "  Sorted 10000 elements in: " << sortTime << " ms" << std::endl;

    // Binary search (requires random access for efficiency)
    bool found = std::binary_search(sortableData.begin(), sortableData.end(), 5000);
    std::cout << "  Binary search for 5000: " << (found ? "found" : "not found") << std::endl;

    // Deque comparison
    std::cout << "\nDeque Random Access (similar to vector):" << std::endl;
    std::cout << "  Direct access [5]: " << dequeData[5] << std::endl;
    std::cout << "  Supports same operations as vector iterators" << std::endl;
  }

  void demonstrateBidirectionalIterators() {
    std::cout << "\n=== BIDIRECTIONAL ITERATORS (list) ===" << std::endl;

    std::cout << "List Bidirectional Capabilities:" << std::endl;

    // Forward traversal
    std::cout << "  Forward traversal (first 10): ";
    auto forwardIt = listData.begin();
    for (int i = 0; i < 10 && forwardIt != listData.end(); ++i, ++forwardIt) {
      std::cout << *forwardIt << " ";
    }
    std::cout << std::endl;

    // Backward traversal using reverse iterators
    std::cout << "  Backward traversal (last 10): ";
    auto reverseIt = listData.rbegin();
    for (int i = 0; i < 10 && reverseIt != listData.rend(); ++i, ++reverseIt) {
      std::cout << *reverseIt << " ";
    }
    std::cout << std::endl;

    // Bidirectional movement
    auto bidirIt = listData.begin();
    std::advance(bidirIt, 50);
    std::cout << "  Advanced to position 50: " << *bidirIt << std::endl;

    // Move backward
    for (int i = 0; i < 5; ++i) {
      --bidirIt;
    }
    std::cout << "  Moved back 5 positions: " << *bidirIt << std::endl;

    // Performance comparison: find in middle
    int targetValue = *std::next(listData.begin(), DATA_SIZE / 2);

    double forwardSearchTime = measureTime([&]() {
      auto found = std::find(listData.begin(), listData.end(), targetValue);
      volatile bool result = (found != listData.end());
    });

    double reverseSearchTime = measureTime([&]() {
      auto found = std::find(listData.rbegin(), listData.rend(), targetValue);
      volatile bool result = (found != listData.rend());
    });

    std::cout << "  Forward search time: " << forwardSearchTime << " ms" << std::endl;
    std::cout << "  Reverse search time: " << reverseSearchTime << " ms" << std::endl;
  }

  void demonstrateForwardIterators() {
    std::cout << "\n=== FORWARD ITERATORS (forward_list) ===" << std::endl;

    std::cout << "Forward List Iterator Capabilities:" << std::endl;

    // Forward-only traversal
    std::cout << "  Forward traversal (first 15): ";
    auto forwardIt = forwardListData.begin();
    for (int i = 0; i < 15 && forwardIt != forwardListData.end(); ++i, ++forwardIt) {
      std::cout << *forwardIt << " ";
    }
    std::cout << std::endl;

    // Cannot go backward - this would be a compile error:
    // --forwardIt;  // Error: no operator-- for forward iterator

    // Single-pass algorithms work well
    int sum = 0;
    double singlePassTime =
        measureTime([&]() { sum = std::accumulate(forwardListData.begin(), forwardListData.end(), 0); });

    std::cout << "  Single-pass accumulation: sum = " << sum << " (time: " << singlePassTime << " ms)" << std::endl;

    // Finding elements (single pass)
    auto found = std::find_if(forwardListData.begin(), forwardListData.end(), [](int x) { return x > 800; });

    if (found != forwardListData.end()) {
      std::cout << "  First element > 800: " << *found << std::endl;
    }

    std::cout << "  Limitations: No reverse iteration, no random access, no backward movement" << std::endl;
  }

  void performanceComparison() {
    std::cout << "\n=== ITERATOR PERFORMANCE COMPARISON ===" << std::endl;

    // Sequential traversal comparison
    std::cout << "Sequential Traversal Performance:" << std::endl;

    volatile int sum;  // Prevent optimization

    // Vector traversal
    double vectorTime = measureTime([&]() {
      sum = 0;
      for (auto it = vectorData.begin(); it != vectorData.end(); ++it) {
        sum += *it;
      }
    });

    // List traversal
    double listTime = measureTime([&]() {
      sum = 0;
      for (auto it = listData.begin(); it != listData.end(); ++it) {
        sum += *it;
      }
    });

    // Forward list traversal
    double forwardTime = measureTime([&]() {
      sum = 0;
      for (auto it = forwardListData.begin(); it != forwardListData.end(); ++it) {
        sum += *it;
      }
    });

    // Deque traversal
    double dequeTime = measureTime([&]() {
      sum = 0;
      for (auto it = dequeData.begin(); it != dequeData.end(); ++it) {
        sum += *it;
      }
    });

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "  Vector (random access): " << vectorTime << " ms" << std::endl;
    std::cout << "  Deque (random access): " << dequeTime << " ms" << std::endl;
    std::cout << "  List (bidirectional): " << listTime << " ms" << std::endl;
    std::cout << "  Forward List (forward): " << forwardTime << " ms" << std::endl;

    // Random access performance test (vector only)
    std::cout << "\nRandom Access Performance (vector only):" << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> indexDist(0, DATA_SIZE - 1);

    double randomAccessTime = measureTime([&]() {
      sum = 0;
      for (int i = 0; i < 1000; ++i) {
        int randomIndex = indexDist(gen);
        sum += vectorData[randomIndex];
      }
    });

    std::cout << "  1000 random accesses: " << randomAccessTime << " ms" << std::endl;
    std::cout << "  (Only possible with random access iterators)" << std::endl;

    std::cout << "\nPerformance Insights:" << std::endl;
    std::cout << "• Vector/Deque: Fastest sequential access due to cache locality" << std::endl;
    std::cout << "• List: Slower due to pointer chasing, but efficient insertion/deletion" << std::endl;
    std::cout << "• Forward List: Memory efficient, good for single-pass algorithms" << std::endl;
    std::cout << "• Random access dramatically faster than sequential search for specific elements" << std::endl;
  }
};

int main() {
  std::cout << "ITERATOR CATEGORY ANALYSIS AND PERFORMANCE COMPARISON" << std::endl;
  std::cout << "=====================================================" << std::endl;

  IteratorCategoryAnalysis analysis;

  // Generate test data
  analysis.generateTestData();

  // Demonstrate different iterator categories
  analysis.demonstrateRandomAccessIterators();
  analysis.demonstrateBidirectionalIterators();
  analysis.demonstrateForwardIterators();

  // Compare performance
  analysis.performanceComparison();

  return 0;
}