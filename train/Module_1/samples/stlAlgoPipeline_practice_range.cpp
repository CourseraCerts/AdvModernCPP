// ============================================================================
// STL Algorithm Pipeline for Data Processing - Starter Files
// Overview: Students build a data processing pipeline using STL algorithms
//           with function objects and simple predicates for sales analysis
// Duration: 60-75 minutes
// Instructions: Follow the GRADED CHALLENGES to implement algorithm-based
//               data filtering, transformation, and analysis
// ============================================================================

/*
 * File: algorithm_pipeline.cpp
 * Author: [Your Name]
 * Date: [Current Date]
 * Purpose: This program implements a data processing pipeline using STL algorithms
 *          with function objects for sales transaction analysis and transformation.
 */

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <ranges>
#include <string>
#include <vector>

using namespace std;
namespace views = std::views;

// Constants
const int DATASET_SIZE = 1000;
const double TAX_RATE = 0.08;
const double DISCOUNT_THRESHOLD = 1000.0;
const double DISCOUNT_RATE = 0.10;

struct Transaction {
  int id;
  string customerId;
  double amount;
  string region;

  // Derived fields (to be calculated)
  double taxAmount;
  double discount;
  double finalTotal;

  Transaction(int i, string cust, double amt, string reg)
      : id(i), customerId(cust), amount(amt), region(reg), taxAmount(0), discount(0), finalTotal(amt) {}
};

// TODO: Function objects for validation and processing
// These replace complex lambda expressions with simple, readable classes

class IsValidTransaction {
 public:
  bool operator()(const Transaction& t) const {
    // TODO: Return true if transaction has amount > 0 and non-empty customerId
    bool validity = t.amount > 0 && !t.customerId.empty();
    return validity;
  }
};

class IsHighValueTransaction {
 public:
  bool operator()(const Transaction& t) const {
    // TODO: Return true if finalTotal > 500
    return t.finalTotal > 500;  // Placeholder
  }
};

// TODO: Transformation function object
class CalculateDerivedValues {
 public:
  Transaction operator()(Transaction t) const {
    // TODO: Calculate taxAmount, discount, and finalTotal
    t.taxAmount = t.amount * TAX_RATE;
    t.discount = (t.amount > DISCOUNT_THRESHOLD) ? t.amount * DISCOUNT_RATE : 0;
    t.finalTotal = t.amount + t.taxAmount - t.discount;
    return t;  // Placeholder - return modified transaction
  }
};

// TODO: Simple helper functions (alternatives to complex function objects)
bool hasValidAmount(const Transaction& t) {
  // TODO: Return true if amount > 0
  return false;  // Placeholder
}

bool hasValidCustomer(const Transaction& t) {
  // TODO: Return true if customerId is not empty
  return false;  // Placeholder
}

double extractFinalTotal(const Transaction& t) {
  // TODO: Return the finalTotal field
  return 0.0;  // Placeholder
}

// TODO: Comparison functions for sorting
bool compareByFinalTotal(const Transaction& a, const Transaction& b) {
  // TODO: Return true if a.finalTotal > b.finalTotal (descending order)
  return a.finalTotal > b.finalTotal;  // Placeholder
}

bool compareByRegionThenAmount(const Transaction& a, const Transaction& b) {
  // TODO: Compare by region first, then by finalTotal if regions are equal
  bool result = a.region == b.region ? a.finalTotal > b.finalTotal : a.region > b.region;
  return result;  // Placeholder
}

class AlgorithmPipeline {
 private:
  vector<Transaction> rawTransactions;
  vector<Transaction> validTransactions;

  template <typename Func>
  double measureTime(Func&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
  }

 public:
  /**
   * GRADED CHALLENGE 1
   * TASK: Implement filterValidTransactions using STL algorithms with function objects
   * to filter out invalid transactions and count statistics.
   *
   * Requirements:
   * - Use copy_if with IsValidTransaction function object
   * - Store results in validTransactions vector
   * - Count and display valid vs invalid transaction statistics
   * - Use count_if with appropriate predicates
   */
  void filterValidTransactions() {
    cout << "Original transactions: " << rawTransactions.size() << endl;

    // TODO: Create IsValidTransaction function object
    IsValidTransaction isValid;

    // TODO: Use copy_if to filter valid transactions
    validTransactions.clear();
    // copy_if(rawTransactions.begin(), rawTransactions.end(), back_inserter(validTransactions), isValid);
    // auto view = rawTransactions | views::filter(isValid);
    // validTransactions.assign(view.begin(), view.end());
    validTransactions = rawTransactions | views::filter(isValid) | std::ranges::to<std::vector>();

    // TODO: Count invalid transactions using count_if
    // You can create a "not valid" predicate or count and subtract
    auto invalid_count = std::count_if(rawTransactions.begin(), rawTransactions.end(),
                                       [&isValid](Transaction& t) { return !isValid(t); });

    // TODO: Display statistics
    cout << "Valid transactions: " << validTransactions.size() << endl;
    cout << "Invalid transactions: " << invalid_count << endl;
  }

  /**
   * GRADED CHALLENGE 2
   * TASK: Implement calculateDerivedValues using std::transform with function objects
   * to compute tax amounts, discounts, and final totals.
   *
   * Requirements:
   * - Use std::transform with CalculateDerivedValues function object
   * - Transform validTransactions in-place
   * - Display sample calculation results
   * - All derived fields should be properly calculated
   */
  void calculateDerivedValues() {
    // TODO: Create CalculateDerivedValues function object
    CalculateDerivedValues calculator;

    // TODO: Use std::transform to calculate derived values in-place
    std::transform(validTransactions.begin(), validTransactions.end(), validTransactions.begin(), calculator);

    cout << "Calculated derived values for " << validTransactions.size() << " transactions" << endl;

    // TODO: Display sample calculation for first transaction
    if (!validTransactions.empty()) {
      const auto& sample = validTransactions[0];
      cout << "Sample: Amount $" << fixed << setprecision(2) << sample.amount;
      cout << " -> Final $" << sample.finalTotal << endl;
    }
  }

  /**
   * GRADED CHALLENGE 3
   * TASK: Implement generateStatistics using accumulate, count_if, and minmax_element
   * for basic data analysis with simple helper functions.
   *
   * Requirements:
   * - Use std::accumulate to calculate total revenue
   * - Use std::count_if with IsHighValueTransaction for segmentation
   * - Use std::minmax_element to find highest/lowest transactions
   * - Calculate and display average transaction value
   * - Use simple helper functions instead of complex expressions
   */
  void generateStatistics() {
    cout << "\n=== Transaction Statistics ===" << endl;

    // TODO: Calculate total revenue using std::accumulate
    // You can use a simple function or the extractFinalTotal helper
    double totalRevenue = accumulate(validTransactions.begin(), validTransactions.end(), 0.0,
                                     [](auto a, const Transaction& b) { return a + b.amount; });

    // TODO: Count high-value transactions using count_if
    IsHighValueTransaction isHighValue;
    int highValueCount = std::ranges::count_if(validTransactions.begin(), validTransactions.end(), isHighValue);

    // TODO: Find min and max transactions using minmax_element
    // Use compareByFinalTotal or create a simple comparison
    auto min_max = std::ranges::minmax_element(validTransactions.begin(), validTransactions.end(), compareByFinalTotal);

    // TODO: Calculate average
    double average = totalRevenue / validTransactions.size();

    // TODO: Display formatted results
    cout << fixed << setprecision(2);
    cout << "Total Revenue: $" << totalRevenue << endl;
    cout << "Average Transaction: $" << average << endl;
    cout << "High-Value Transactions (>$500): " << highValueCount << endl;
  }

  /**
   * GRADED CHALLENGE 4
   * TASK: Implement sortAndRank using sorting algorithms with simple comparison functions
   * to organize transactions and create rankings.
   *
   * Requirements:
   * - Sort transactions by finalTotal using std::sort with compareByFinalTotal
   * - Use std::partial_sort to find top 10 transactions efficiently
   * - Sort by region then amount using compareByRegionThenAmount
   * - Display top transactions and regional rankings
   */
  void sortAndRank() {
    cout << "\n=== Sorting and Ranking Analysis ===" << endl;

    // Create copy for sorting operations
    vector<Transaction> sortedTransactions = validTransactions;

    // TODO: Sort by finalTotal (descending) using std::sort
    sort(sortedTransactions.begin(), sortedTransactions.end(), compareByFinalTotal);

    // TODO: Display top 5 transactions
    cout << "\nTop 5 Transactions (Overall):" << endl;
    cout << fixed << setprecision(2);
    for (size_t i = 0; i < min(5ul, sortedTransactions.size()); ++i) {
      auto t = sortedTransactions[i];
      cout << "  #" << (i + 1) << ": " << t.customerId << " - $" << t.finalTotal << " (" << t.region << ")" << endl;
    }

    // TODO: Use partial_sort to find top 10 transactions efficiently
    vector<Transaction> topTransactions = validTransactions;
    size_t top_count = min(10ul, topTransactions.size());
    ranges::partial_sort(topTransactions, topTransactions.begin() + top_count, compareByFinalTotal);

    cout << "\nTop 10 Transactions (Partial Sort):" << endl;
    for (size_t i = 0; i < top_count; ++i) {
      auto t = topTransactions[i];
      cout << "  #" << (i + 1) << ": " << t.customerId << " - $" << t.finalTotal << endl;
    }

    // TODO: Sort by region then amount using compareByRegionThenAmount
    ranges::sort(sortedTransactions.begin(), sortedTransactions.end(), compareByRegionThenAmount);

    // === REGIONAL RANKINGS (C++20 Ranges Implementation) ===
    cout << "\n--- Regional Rankings (Ranges Approach) ---" << endl;

    // Group transactions by region using ranges
    vector<string> regions = {"East", "North", "South", "West"};  // Sorted for grouped output

    for (const auto& region : regions) {
      // Filter transactions for this region
      auto regionView =
          validTransactions | views::filter([&region](const Transaction& t) { return t.region == region; });

      auto count = ranges::distance(regionView);
      if (count == 0) continue;

      // Materialize view into a vector so we can sort and index it
      auto regionTransactions = regionView | std::ranges::to<std::vector>();

      // Compute top count using matching unsigned type
      size_t topCount = std::min(5ul, regionTransactions.size());

      // Partial sort the materialized vector to get top entries
      if (topCount > 0 && regionTransactions.size() > 1) {
        std::partial_sort(regionTransactions.begin(), regionTransactions.begin() + topCount, regionTransactions.end(),
                          compareByFinalTotal);
      }

      cout << "\n" << region << " Region:" << endl;

      // Display top entries in this region
      for (size_t i = 0; i < topCount; ++i) {
        cout << "  #" << (i + 1) << ": " << regionTransactions[i].customerId << " - $"
             << regionTransactions[i].finalTotal << endl;
      }
    }
  }

  /**
   * Function: generateTestData
   * Description: Creates realistic transaction data for algorithm processing
   */
  void generateTestData(int count) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> amountDist(10.0, 2000.0);
    uniform_int_distribution<> customerDist(1, 100);

    vector<string> regions = {"North", "South", "East", "West"};

    for (int i = 0; i < count; ++i) {
      // Occasionally create invalid data for filtering practice
      double amount = (i % 50 == 0) ? -1.0 : amountDist(gen);
      string customerId = (i % 75 == 0) ? "" : "CUST_" + to_string(customerDist(gen));

      rawTransactions.emplace_back(i, customerId, amount, regions[i % regions.size()]);
    }
  }

  /**
   * Function: runPipelineTests
   * Description: Executes all pipeline stages and demonstrates algorithm usage
   */
  void runPipelineTests() {
    cout << "=== STL Algorithm Pipeline Processing ===" << endl;

    // Generate test data
    cout << "Generating test dataset..." << endl;
    generateTestData(DATASET_SIZE);
    cout << "✓ Generated " << rawTransactions.size() << " transactions" << endl;

    // Stage 1: Data filtering
    cout << "\nStage 1: Data validation and filtering..." << endl;
    double filter_exec_time = measureTime([this]() { filterValidTransactions(); });
    cout << "✓ Filtering complete" << endl;
    cout << "filter exec time: " << std::fixed << std::setprecision(6) << filter_exec_time << " mic s" << std::endl;

    // Stage 2: Data transformation
    cout << "\nStage 2: Calculating derived values..." << endl;
    calculateDerivedValues();
    cout << "✓ Transformation complete" << endl;

    // Stage 3: Statistical analysis
    cout << "\nStage 3: Statistical analysis..." << endl;
    generateStatistics();
    cout << "✓ Statistics complete" << endl;

    // Stage 4: Sorting and ranking
    cout << "\nStage 4: Sorting and ranking..." << endl;
    sortAndRank();
    cout << "✓ Sorting complete" << endl;
  }
};

/*
REQUIREMENTS SUMMARY:
- Use function objects (classes with operator()) instead of complex expressions
- Apply copy_if and count_if for data filtering and counting
- Use std::transform with function objects for value calculations
- Use accumulate, minmax_element for statistical analysis with simple helpers
- Apply sorting algorithms with simple comparison functions
- Handle edge cases and display meaningful results
- Follow proper C++ coding standards and formatting
*/

/**
 * Function: main
 * Description: Program entry point demonstrating STL algorithm pipeline processing
 */
int main() {
  cout << "=== STL Algorithm Pipeline for Data Processing ===" << endl;
  cout << "Demonstrating algorithm usage with function objects and simple predicates\n" << endl;

  AlgorithmPipeline pipeline;
  pipeline.runPipelineTests();

  cout << "\n=== Pipeline Processing Complete ===" << endl;
  cout << "Review your algorithm choices and function object implementations!" << endl;

  return 0;
}

/*
EXPECTED OUTPUT STRUCTURE:
- Transaction counts and validation statistics using function objects
- Derived value calculations with std::transform and calculation objects
- Statistical analysis using accumulate, count_if with simple predicates
- Sorted rankings using comparison functions
- Clear, formatted output showing algorithm effectiveness with readable code
*/