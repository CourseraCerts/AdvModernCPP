/*
In the code below, use the container selection framework to analyze your own
application scenarios:

Application Analysis Exercise:

1. E-commerce Cart System: Customers add/remove items, need to display in order
added
2. Network Packet Router: Process packets based on priority levels
3. Social Media Feed: Store posts with fast user ID lookup and chronological
display
4. Game Leaderboard: Maintain top scores in sorted order with frequent updates
5. File System Cache: Fast file lookup with LRU (Least Recently Used) eviction

For each scenario, determine:

* Primary access pattern (insert, lookup, iterate, etc.)
* Performance requirements (speed vs. memory)
* Ordering requirements (sorted, insertion order, priority)
* Data uniqueness requirements

Container Selection Practice: Apply the decision tree to choose optimal
containers for each scenario and justify your choices based on time complexity
and usage patterns.

Success Checklist
Understanding of systematic approach to container selection
Ability to analyze application requirements and map to optimal containers
Knowledge of trade-offs between different container characteristics
Practical framework for making container decisions in real projects

💡 Key Points
Container selection significantly impacts application performance, especially with large datasets
Vector provides the best general-purpose performance for most scenarios requiring random access
List excels when frequent insertion/deletion at arbitrary positions is the primary operation
Unordered containers (hash-based) provide O(1) average case operations but use more memory
Ordered containers (tree-based) enable range queries and sorted iteration at O(log n) cost
Container adaptors (stack, queue, priority_queue) provide specialized interfaces for common patterns

❗ Common Mistakes to Avoid
Choosing containers based on familiarity rather than performance characteristics
Using vector for frequent front insertions or middle operations
Using list when random access is frequently needed
Selecting ordered containers when hash-based containers would be more efficient
Ignoring memory overhead differences between container types
Not considering the impact of container choice on cache performance

🚀 Next Steps
* These container selection and performance analysis skills are fundamental for building high-performance C++
applications.
* In enterprise environments, optimal data structure choice directly impacts system scalability, response times, and
resource utilization.
* Consider exploring advanced topics like custom allocators for performance tuning, container adaptors for specialized
use cases, and how modern C++17/20 features enhance STL container functionality.
* These capabilities will be essential when architecting systems that must handle millions of operations per second or
process large-scale datasets efficiently.

*/
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class AccessPattern {
  FREQUENT_FRONT_BACK,     // Queue-like operations
  FREQUENT_MIDDLE,         // Insert/delete at arbitrary positions
  FREQUENT_RANDOM_ACCESS,  // Array-like access by index
  FREQUENT_SEARCH,         // Finding specific elements
  ORDERED_ITERATION,       // Need sorted order
  PRIORITY_PROCESSING,     // Process by importance
  UNIQUE_ELEMENTS,         // Set-like behavior
  KEY_VALUE_PAIRS          // Dictionary-like behavior
};

class ContainerSelector {
 private:
  struct ContainerRecommendation {
    std::string containerType;
    std::string rationale;
    std::string timeComplexity;
    std::string alternativeOptions;
  };

  ContainerRecommendation analyzeRequirements(AccessPattern pattern, bool needsOrder, bool uniqueElements,
                                              int expectedSize) {
    ContainerRecommendation rec;

    switch (pattern) {
      case AccessPattern::FREQUENT_FRONT_BACK:
        if (needsOrder && expectedSize < 10000) {
          rec.containerType = "std::deque<T>";
          rec.rationale = "Efficient front/back operations with random access";
          rec.timeComplexity = "O(1) front/back, O(1) random access";
          rec.alternativeOptions = "std::queue<T> for strict FIFO, std::stack<T> for LIFO";
        } else {
          rec.containerType = "std::list<T>";
          rec.rationale =
              "Optimal for frequent front/back operations without random "
              "access needs";
          rec.timeComplexity = "O(1) front/back operations";
          rec.alternativeOptions = "std::deque<T> if random access needed";
        }
        break;

      case AccessPattern::FREQUENT_MIDDLE:
        rec.containerType = "std::list<T>";
        rec.rationale = "Constant time insertion/deletion at any position";
        rec.timeComplexity = "O(1) insert/delete at known position";
        rec.alternativeOptions =
            "std::deque<T> for better cache locality with moderate middle "
            "operations";
        break;

      case AccessPattern::FREQUENT_RANDOM_ACCESS:
        rec.containerType = "std::vector<T>";
        rec.rationale = "Contiguous memory provides fastest random access";
        rec.timeComplexity = "O(1) random access, O(1) amortized back insertion";
        rec.alternativeOptions = "std::deque<T> if front insertion also needed";
        break;

      case AccessPattern::FREQUENT_SEARCH:
        if (needsOrder) {
          rec.containerType = "std::set<T> or std::map<K,V>";
          rec.rationale = "Logarithmic search with maintained sorted order";
          rec.timeComplexity = "O(log n) search, insert, delete";
          rec.alternativeOptions = "std::multiset<T> for duplicate values";
        } else {
          rec.containerType = "std::unordered_set<T> or std::unordered_map<K,V>";
          rec.rationale = "Hash-based containers provide fastest average search";
          rec.timeComplexity = "O(1) average case search, insert, delete";
          rec.alternativeOptions = "std::set<T> if order or range queries needed";
        }
        break;

      case AccessPattern::PRIORITY_PROCESSING:
        rec.containerType = "std::priority_queue<T>";
        rec.rationale = "Heap-based structure always provides highest priority element";
        rec.timeComplexity = "O(1) top access, O(log n) insert/remove";
        rec.alternativeOptions = "std::multimap<Priority,T> for stable priority ordering";
        break;

      default:
        rec.containerType = "std::vector<T>";
        rec.rationale = "General-purpose container with good all-around performance";
        rec.timeComplexity = "O(1) random access and back operations";
        rec.alternativeOptions = "Analyze specific usage patterns for optimization";
    }

    return rec;
  }

 public:
  void demonstrateContainerSelection() {
    std::cout << "=== STL CONTAINER SELECTION DECISION FRAMEWORK ===" << std::endl;
    std::cout << "Analyzing different application scenarios:\n" << std::endl;

    // Scenario 1: Task Queue System
    std::cout << "SCENARIO 1: Task Queue Processing System" << std::endl;
    std::cout << "Requirements: FIFO processing, frequent front/back operations" << std::endl;
    auto rec1 = analyzeRequirements(AccessPattern::FREQUENT_FRONT_BACK, false, false, 5000);
    printRecommendation(rec1);

    // Scenario 2: Text Editor Buffer
    std::cout << "\nSCENARIO 2: Text Editor Buffer" << std::endl;
    std::cout << "Requirements: Frequent insertion/deletion at cursor position" << std::endl;
    auto rec2 = analyzeRequirements(AccessPattern::FREQUENT_MIDDLE, false, false, 50000);
    printRecommendation(rec2);

    // Scenario 3: Game Physics Array
    std::cout << "\nSCENARIO 3: Game Physics Object Array" << std::endl;
    std::cout << "Requirements: Frequent random access, mathematical operations" << std::endl;
    auto rec3 = analyzeRequirements(AccessPattern::FREQUENT_RANDOM_ACCESS, false, false, 10000);
    printRecommendation(rec3);

    // Scenario 4: User Database
    std::cout << "\nSCENARIO 4: User Database with Sorted Reports" << std::endl;
    std::cout << "Requirements: Fast user lookup, ordered iteration for reports" << std::endl;
    auto rec4 = analyzeRequirements(AccessPattern::FREQUENT_SEARCH, true, false, 100000);
    printRecommendation(rec4);

    // Scenario 5: High-Frequency Trading
    std::cout << "\nSCENARIO 5: High-Frequency Trading System" << std::endl;
    std::cout << "Requirements: Fastest possible symbol lookup, no order needed" << std::endl;
    auto rec5 = analyzeRequirements(AccessPattern::FREQUENT_SEARCH, false, false, 50000);
    printRecommendation(rec5);

    // Scenario 6: Priority-Based Alert System
    std::cout << "\nSCENARIO 6: Critical Alert Processing" << std::endl;
    std::cout << "Requirements: Always process highest priority alerts first" << std::endl;
    auto rec6 = analyzeRequirements(AccessPattern::PRIORITY_PROCESSING, false, false, 1000);
    printRecommendation(rec6);
  }

  void printRecommendation(const ContainerRecommendation& rec) {
    std::cout << "  RECOMMENDED: " << rec.containerType << std::endl;
    std::cout << "  RATIONALE: " << rec.rationale << std::endl;
    std::cout << "  COMPLEXITY: " << rec.timeComplexity << std::endl;
    std::cout << "  ALTERNATIVES: " << rec.alternativeOptions << std::endl;
  }

  void provideSelectionGuidelines() {
    std::cout << "\n=== CONTAINER SELECTION QUICK REFERENCE ===" << std::endl;

    std::cout << "\n📊 SEQUENCE CONTAINERS:" << std::endl;
    std::cout << "• vector<T>     - Default choice, best random access, "
                 "cache-friendly"
              << std::endl;
    std::cout << "• list<T>       - Frequent middle insert/delete, no random "
                 "access needed"
              << std::endl;
    std::cout << "• deque<T>      - Queue operations + random access, good compromise" << std::endl;

    std::cout << "\n  ASSOCIATIVE CONTAINERS:" << std::endl;
    std::cout << "• map<K,V>      - Key-value pairs with sorted keys, range queries" << std::endl;
    std::cout << "• unordered_map - Key-value pairs, fastest lookup, no order" << std::endl;
    std::cout << "• set<T>        - Unique elements with sorted order" << std::endl;
    std::cout << "• unordered_set - Unique elements, fastest membership testing" << std::endl;

    std::cout << "\n📚 CONTAINER ADAPTORS:" << std::endl;
    std::cout << "• stack<T>      - LIFO operations (undo systems, recursion)" << std::endl;
    std::cout << "• queue<T>      - FIFO operations (task processing, BFS)" << std::endl;
    std::cout << "• priority_queue- Priority-based processing (event systems)" << std::endl;

    std::cout << "\n🎯 SELECTION DECISION TREE:" << std::endl;
    std::cout << "1. Need key-value mapping? → map/unordered_map" << std::endl;
    std::cout << "2. Need unique elements only? → set/unordered_set" << std::endl;
    std::cout << "3. Need priority processing? → priority_queue" << std::endl;
    std::cout << "4. Need LIFO/FIFO only? → stack/queue" << std::endl;
    std::cout << "5. Need frequent middle insert/delete? → list" << std::endl;
    std::cout << "6. Need random access + end operations? → vector" << std::endl;
    std::cout << "7. Need both-end operations + random access? → deque" << std::endl;

    std::cout << "\n⚡ PERFORMANCE CONSIDERATIONS:" << std::endl;
    std::cout << "• Small datasets (<1000): Performance differences often negligible" << std::endl;
    std::cout << "• Large datasets (>100k): Container choice becomes critical" << std::endl;
    std::cout << "• Memory-constrained: Prefer ordered containers (no hash overhead)" << std::endl;
    std::cout << "• CPU-bound operations: Prefer unordered containers for speed" << std::endl;
  }
};

int main() {
  std::cout << "STL CONTAINER SELECTION FRAMEWORK" << std::endl;
  std::cout << "=================================" << std::endl;

  ContainerSelector selector;
  selector.demonstrateContainerSelection();
  selector.provideSelectionGuidelines();

  return 0;
}