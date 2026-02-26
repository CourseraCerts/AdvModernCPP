// ============================================================================
// Iterator and Function Object Data Processing - Starter Files
// Overview: Students practice iterator categories and function objects
//           with practical data processing scenarios
// Duration: 60-75 minutes
// Instructions: Follow the GRADED CHALLENGES to implement iterator-based
//               data processing with custom function objects
// ============================================================================

/*
 * File: iterator_processing.cpp
 * Author: Nitin Sharma
 * Date: Feb 19 2026
 * Purpose: This program demonstrates iterator categories and function objects
 *          for practical data processing tasks.
 */

#include <algorithm>
#include <chrono>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <numeric>
#include <random>
#include <vector>

using namespace std;

// Constants for data generation and processing
const int DATASET_SIZE = 10000;
const int MAX_VALUE = 100;
const int MIN_VALUE = 1;

// Function objects for data processing
class IsEven {
 public:
  bool operator()(int x) const { return x % 2 == 0; }
};

class IsPositive {
 public:
  bool operator()(int x) const { return x > 0; }
};

class MultiplyBy {
 private:
  int factor;

 public:
  MultiplyBy(int f) : factor(f) {}

  int operator()(int x) const { return factor * x; }
};

class AddValue {
 private:
  int addend;

 public:
  AddValue(int a) : addend(a) {}

  int operator()(int x) const { return x + addend; }
};

class IsGreaterThan {
 private:
  int threshold;

 public:
  IsGreaterThan(int t) : threshold(t) {}

  bool operator()(int x) const { return x > threshold; }
};

// Helper function to display container contents
template <typename Container>
void displayContainer(const Container& container, const string& name, int limit = 10) {
  cout << name << " (first " << limit << "): ";
  int count = 0;
  auto it = container.begin();
  while (it != container.end() && count < limit) {
    cout << *it << " ";
    ++it;
    count++;
  }
  cout << endl;
}

class SumIfEven {
 public:
  int operator()(int accumulator, int value) const {
    if (value % 2 == 0) {
      return accumulator + value;
    }
    return accumulator;
  }
};

class IteratorProcessor {
 private:
  vector<int> vectorData;
  list<int> listData;
  forward_list<int> forwardListData;

 public:
  /**
   * GRADED CHALLENGE 1
   * TASK: Implement iteratorCategoryDemo to show appropriate usage
   * of different iterator types with their optimal use cases.
   *
   * Requirements:
   * - Demonstrate random access iterator capabilities with vector
   * - Show bidirectional iterator usage with list (forward and reverse)
   * - Use forward iterator with forward_list for single-pass operations
   * - Explain the capabilities and limitations of each iterator type
   */
  void iteratorCategoryDemo() {
    cout << "\n=== Iterator Category Demonstration ===" << endl;

    // TODO: Demonstrate random access iterator with vector
    cout << "\n1. Vector (Random Access Iterator):" << endl;
    // Show direct access: vectorData[5]
    // Show iterator arithmetic: *(vectorData.begin() + 10)
    // Explain random access capabilities
    cout << "  accessing random value: " << vectorData[5] << endl;
    cout << "  accessing begin() + 10 value: " << *(vectorData.begin() + 10) << endl;
    auto it1 = vectorData.begin() + 10;
    auto it2 = vectorData.begin() + 20;
    cout << "  iterator comparison : " << (it1 < it2 ? "true" : "false") << endl;
    cout << "  iterator distance comparison : " << distance(it1, it2) << endl;
    cout << "  access to random locations is allowed " << endl;

    // TODO: Demonstrate bidirectional iterator with list
    cout << "\n2. List (Bidirectional Iterator):" << endl;
    // Show forward traversal using ++iterator
    // Show reverse traversal using list.rbegin() and rend()
    // Explain bidirectional capabilities
    cout << "  first 10 or min 10: ";
    auto forwardIt = listData.begin();
    for (int i = 0; i < 10 && listData.end() != forwardIt; ++i, ++forwardIt) {
      cout << "  " << *forwardIt;
    }
    cout << "\n  last 10 in reverse order: ";
    auto reverseIt = listData.rbegin();
    for (int k = 0; k < 10 && listData.rend() != reverseIt; k++, ++reverseIt) {
      cout << " " << *reverseIt;
    }
    auto bidirIt = listData.begin();
    std::advance(bidirIt, 10);
    cout << "  backwards after moving forward: ";
    for (int i = 0; i < 5; ++i) {
      cout << " " << *bidirIt--;
    }
    cout << endl;

    // TODO: Demonstrate forward iterator with forward_list
    cout << "\n3. Forward List (Forward Iterator):" << endl;
    auto forwardIt = forwardListData.begin();
    for (int i = 0; i < 10 && forwardIt != forwardListData.end(); ++i, ++forwardIt) {
      cout << *forwardIt << " ";
    }
    cout << endl;

    // TODO: Compare iterator capabilities
    cout << "\nIterator Capability Summary:" << endl;
    cout << "Vector (Random Access): Supports [], +, -, <, >, random jumps" << endl;
    cout << "List (Bidirectional): Supports ++, --, forward/reverse traversal" << endl;
    cout << "Forward List (Forward): Supports ++ only, single direction traversal" << endl;
  }

  /**
   * GRADED CHALLENGE 2
   * TASK: Implement functionObjectProcessing using custom function objects
   * with STL algorithms for reusable data transformation operations.
   *
   * Requirements:
   * - Use MultiplyBy function object with std::transform
   * - Use IsEven function object with std::count_if
   * - Use AddValue function object for data modification
   * - Apply function objects to different container types
   * - Display results showing function object reusability
   */
  void functionObjectProcessing() {
    cout << "\n=== Function Object Processing ===" << endl;

    // TODO: Create function object instances
    MultiplyBy doubler(2);
    IsEven evenChecker;
    AddValue incrementer(10);
    IsGreaterThan aboveThreshold(50);

    // TODO: Use std::transform with MultiplyBy to double values in vectorData
    vector<int> doubledValues(vectorData.size());
    transform(vectorData.begin(), vectorData.end(), doubledValues.begin(), doubler);

    cout << "\nApplied doubler function object to vector:" << endl;
    displayContainer(vectorData, "Original vector");
    displayContainer(doubledValues, "Original vector");

    // TODO: Use std::count_if with IsEven to count even numbers
    int eventCount = count_if(vectorData.begin(), vectorData.end(), evenChecker);
    cout << "Even numbers in vector:" << eventCount << " out of " < vectorData.size() << endl;

    // TODO: Use AddValue with std::transform on listData
    list<int> incrementedList;
    transform(listData.begin(), listData.end(), back_inserter(incrementedList), incrementer);
    cout << "\nApplied incrementer function object to list:" << endl;
    displayContainer(listData, "Original list");
    displayContainer(incrementedList, "Incremented list");

    // TODO: Use IsGreaterThan with std::count_if
    int aboveCount = count_if(vectorData.begin(), vectorData.end(), aboveThreshold);
    cout << "Numberfs above 50: " << aboveCount << endl;

    // TODO: Display results
    cout << "Original vector size: " << vectorData.size() << endl;
    cout << "Even numbers found: " << eventCount << endl;
    cout << "Numbers above 50: " << aboveCount << endl;

    cout << "Function objects demonstrate reusability across containers" << endl;
    cout << " Reusable across different containers and algorithms" << endl;
    cout << " Stateful (can store configuration like facgtor or threshold)" << endl;
    cout << " Type-safe and efficient (inline during compilation)" << endl;
    cout << " Clearn, readable code with meaningful names" << endl;
  }

  /**
   * GRADED CHALLENGE 3
   * TASK: Implement algorithmIntegration showing function objects with
   * different STL algorithms across various iterator types.
   *
   * Requirements:
   * - Use std::find_if with function objects
   * - Use std::for_each with function objects for output
   * - Use std::accumulate with function objects for aggregation
   * - Apply to different container types with different iterator categories
   */
  void algorithmIntegration() {
    cout << "\n=== Algorithm Integration with Function Objects ===" << endl;

    // Use std::find_if to find first even number in vector
    IsEven evenChecker;
    auto found = find_if(vectorData.begin(), vectorData.end(), evenChecker);
    if (found != vectorData.end()) {
      cout << "First even number in vector:" << *found << endl;
    } else {
      cout << "No even number found in vector" << endl;
    }

    // Use std::for_each to display elements above threshold
    IsGreaterThan aboveThreshold(25);
    cout << "Elements above 25: ";
    for_each(vectorData.begin(), vectorData.end(), [&aboveThreshold](int x) {
      if (aboveThreshold(x)) cout << x << " ";
    });
    cout << endl;

    // Use function objects with std::accumulate for conditional sum
    SumIfEven evenSumAdder;
    int evenSum = accumulate(vectorData.begin(), vectorData.end(), 0, evenSumAdder);

    // Apply same function objects to list (bidirectional iterators)
    cout << "\nApplying same function objects to list:" << endl;
    auto foundInList = find_if(listData.begin(), listData.end(), evenChecker);
    if (foundInList != listData.end()) {
      cout << "First even number in list: " << *foundInList << endl;
    }

    // TODO: Apply to forward_list (forward iterators)
    cout << "Applying to forward_list (forward iterators only):" << endl;
    auto foundInForward = find_if(forwardListData.begin(), forwardListData.end(), evenChecker);
    if (foundInForward != forwardListData.end()) {
      cout << "First even number in forward_list: " << *foundInForward << endl;
    }

    cout << "Function objects work seamlessly across different iterator types!" << endl;
  }

  /**
   * GRADED CHALLENGE 4
   * TASK: Implement performanceComparison to analyze performance
   * characteristics of different iterator categories.
   *
   * Requirements:
   * - Measure traversal time for vector, list, and forward_list
   * - Compare random access vs sequential access performance
   * - Analyze when each iterator type is optimal
   * - Document performance characteristics
   */
  void performanceComparison() {
    cout << "\n=== Performance Comparison ===" << endl;

    // TODO: Measure vector traversal time (random access)
    auto start = chrono::high_resolution_clock::now();
    // Use accumulate or manual loop to sum vectorData
    long long vectorSum = accumulate(vectorData.begin(), vectorData.end(), 0LL);
    auto end = chrono::high_resolution_clock::now();
    auto vectorTime = chrono::duration_cast<chrono::microseconds>(end - start);

    // TODO: Measure list traversal time (bidirectional)
    start = chrono::high_resolution_clock::now();
    long long listSum = accumulate(listData.begin(), listData.end(), 0LL);
    // Sum listData elements
    end = chrono::high_resolution_clock::now();
    auto listTime = chrono::duration_cast<chrono::microseconds>(end - start);

    // TODO: Measure forward_list traversal time (forward)
    start = chrono::high_resolution_clock::now();
    long long forwardSum = 0;
    for (auto it = forwardListData.begin(); it != forwardListData.end(); ++it) {
      forwardSum += *it;
    }
    // Sum forwardListData elements
    end = chrono::high_resolution_clock::now();
    auto forwardTime = chrono::duration_cast<chrono::microseconds>(end - start);

    // TODO: Display performance results
    cout << "Vector traversal: " << vectorTime.count() << " μs (sum: " << vectorSum << ")" << endl;
    cout << "List traversal: " << listTime.count() << " μs (sum: " << listSum << ")" << endl;
    cout << "Forward list traversal: " << forwardTime.count() << " μs (sum: " << forwardSum << ")" << endl;

    cout << "\nPerformance Characteristics:" << endl;
    cout << "- Vector: Fast random access, cache-friendly sequential access" << endl;
    cout << "- List: Efficient insertion/deletion, slower traversal due to pointer chasing" << endl;
    cout << "- Forward List: Minimal memory overhead, forward-only access limitation" << endl;
  }

  /**
   * GRADED CHALLENGE 5
   * TASK: Implement dataProcessingPipeline to create a complete processing
   * system using appropriate iterators and function objects.
   *
   * Requirements:
   * - Filter data using function objects and appropriate algorithms
   * - Transform data using function objects with different iterator types
   * - Aggregate results using function objects and accumulate
   * - Choose optimal iterator/container combinations for each stage
   */
  void dataProcessingPipeline() {
    cout << "\n=== Data Processing Pipeline ===" << endl;

    // Stage 1 - Filter positive numbers from vector
    IsPositive positiveChecker;
    vector<int> positiveNumbers;
    // Use copy_if to filter positive numbers
    copy_if(vectorData.begin(), vectorData.end(), back_inserter(positiveNumbers), positiveChecker);

    // Stage 2 - Transform: multiply by 2 using appropriate container
    MultiplyBy doubler(2);
    vector<int> doubledPositives(positiveNumbers.size());
    transform(positiveNumbers.begin(), positiveNumbers.end(), doubledPositives.begin(), doubler);

    // Stage 3 - Filter even numbers from the doubled results
    IsEven evenChecker;
    vector<int> evenDoubledPositives;
    copy_if(doubledPositives.begin(), doubledPositives.end(), back_inserter(evenDoubledPositives), evenChecker);

    // Stage 4 - Calculate statistics using accumulate
    int finalSum = accumulate(evenDoubledPositives.begin(), evenDoubledPositives.end(), 0);
    int finalCount = evenDoubledPositives.size();
    double average = finalCount > 0 ? static_cast<double>(finalSum) / finalCount : 0.0;

    // : Display pipeline results
    cout << "Processing Pipeline Results:" << endl;
    cout << "Original data size: " << vectorData.size() << endl;
    cout << "Positive numbers: " << positiveNumbers.size() << endl;
    cout << "After doubling: " << doubledPositives.size() << endl;
    cout << "Final even results: " << evenDoubledPositives.size() << endl;
    cout << "Final sum: " << finalSum << endl;
    cout << "Final average: " << fixed << setprecision(2) << average << endl;

    displayContainer(evenDoubledPositives, "Final processed data", 15);

    cout << "\nPipeline demonstrates:" << endl;
    cout << "- Function object reusability across processing stages" << endl;
    cout << "- Iterator compatibility with different algorithms" << endl;
    cout << "- Efficient data processing using STL algorithms" << endl;
  }

  /**
   * Function: generateTestData
   * Description: Creates test data in different container types
   */
  void generateTestData() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(MIN_VALUE, MAX_VALUE);

    // Generate data for vector
    vectorData.clear();
    vectorData.reserve(DATASET_SIZE);
    for (int i = 0; i < DATASET_SIZE; ++i) {
      vectorData.push_back(dist(gen));
    }

    // Generate data for list
    listData.clear();
    for (int i = 0; i < DATASET_SIZE; ++i) {
      listData.push_back(dist(gen));
    }

    // Generate data for forward_list (note: pushes to front)
    forwardListData.clear();
    for (int i = 0; i < DATASET_SIZE; ++i) {
      forwardListData.push_front(dist(gen));
    }

    cout << "Generated " << DATASET_SIZE << " random values (range: " << MIN_VALUE << "-" << MAX_VALUE << ")" << endl;
  }

  /**
   * Function: runProcessingTests
   * Description: Executes all processing demonstrations
   */
  void runProcessingTests() {
    cout << "=== Iterator and Function Object Data Processing ===" << endl;

    // Generate test data
    cout << "\nGenerating test data..." << endl;
    generateTestData();
    cout << "✓ Generated data in vector, list, and forward_list containers" << endl;

    // Run all demonstrations
    iteratorCategoryDemo();
    functionObjectProcessing();
    algorithmIntegration();
    performanceComparison();
    dataProcessingPipeline();
  }
};

/*
REQUIREMENTS SUMMARY:
- Demonstrate appropriate usage of different iterator categories
- Implement and use custom function objects with STL algorithms
- Apply function objects across different container types and iterator categories
- Measure and compare performance characteristics of different approaches
- Create a complete data processing pipeline using iterators and function objects
- Show when to choose different iterator/container combinations
- Handle different container types and their iterator capabilities
*/

/**
 * Function: main
 * Description: Program entry point demonstrating iterator and function object processing
 */
int main() {
  cout << "=== Iterator and Function Object Data Processing ===" << endl;
  cout << "Demonstrating iterator categories and function objects\n" << endl;

  IteratorProcessor processor;
  processor.runProcessingTests();

  cout << "\n=== Processing Complete ===" << endl;
  cout << "Review your iterator choices and function object implementations!" << endl;

  return 0;
}

/*
EXPECTED OUTPUT STRUCTURE:
- Iterator category demonstrations with appropriate usage examples
- Function object applications showing reusability across containers
- Algorithm integration with function objects across different iterator types
- Performance comparison analysis across iterator categories
- Complete data processing pipeline demonstrating all concepts
-
*/