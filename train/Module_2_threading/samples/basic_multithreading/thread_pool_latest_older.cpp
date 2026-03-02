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
#include <expected>
#include <iostream>
#include <mutex>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

namespace {
std::mutex coutMutex;

void log_thread_safe(std::string_view message) {
  std::scoped_lock guard(coutMutex);
  std::cout << message << '\n';
}

class ThreadGuard {
 public:
  ThreadGuard() = default;

  template <class Callable, class... Args>
  void emplace_back(Callable&& callable, Args&&... args) {
    threads_.emplace_back(std::forward<Callable>(callable), std::forward<Args>(args)...);
  }

  ~ThreadGuard() noexcept {
    for (auto& worker : threads_) {
      if (worker.joinable()) {
        worker.join();
      }
    }
  }

  ThreadGuard(const ThreadGuard&) = delete;
  ThreadGuard& operator=(const ThreadGuard&) = delete;
  ThreadGuard(ThreadGuard&&) = delete;
  ThreadGuard& operator=(ThreadGuard&&) = delete;

 private:
  std::vector<std::thread> threads_;
};

struct SegmentReport {
  std::size_t analyzerId{};
  std::expected<double, std::string> outcome = std::unexpected(std::string{"pending"});
};

struct AnalysisSummary {
  std::vector<SegmentReport> reports;
  std::size_t processedSegments{};
};

std::vector<std::vector<double>> create_demo_segments() {
  std::vector<std::vector<double>> segments{{1.0, 2.0, 3.0, 4.0, 5.0}, {6.0, 7.0, 8.0}, {}};

  std::mt19937_64 rng{std::random_device{}()};
  std::uniform_real_distribution<double> dist(0.0, 10.0);

  std::vector<double> randomSegment;
  randomSegment.reserve(6);
  for (int i = 0; i < 6; ++i) {
    randomSegment.push_back(dist(rng));
  }
  segments.emplace_back(std::move(randomSegment));
  return segments;
}

double compute_risk(std::span<const double> data) {
  if (data.empty()) {
    throw std::runtime_error("No data to analyze");
  }

  double sum = 0.0;
  for (double value : data) {
    sum += value * value;
    std::this_thread::sleep_for(5ms);  // Simulate expensive processing
  }
  return sum / static_cast<double>(data.size());
}

AnalysisSummary run_parallel_analysis(const std::vector<std::vector<double>>& segments) {
  AnalysisSummary summary;
  summary.reports.resize(segments.size());
  for (std::size_t i = 0; i < segments.size(); ++i) {
    summary.reports[i].analyzerId = i;
  }

  std::atomic_size_t processed{0};

  {
    ThreadGuard guard;

    for (std::size_t i = 0; i < segments.size(); ++i) {
      guard.emplace_back([&, idx = i] {
        try {
          const double result = compute_risk(std::span<const double>{segments[idx]});
          summary.reports[idx].outcome = result;
          log_thread_safe("Analyzer " + std::to_string(idx) + " calculated risk: " + std::to_string(result));
        } catch (const std::exception& e) {
          summary.reports[idx].outcome = std::unexpected(std::string{e.what()});
          log_thread_safe("Analyzer " + std::to_string(idx) + " error: " + e.what());
        }
        processed.fetch_add(1, std::memory_order_relaxed);
      });
    }
  }  // guard joins threads here, even when exceptions escape

  summary.processedSegments = processed.load();
  return summary;
}
}  // namespace

int main() {
  try {
    const auto segments = create_demo_segments();
    const auto summary = run_parallel_analysis(segments);

    for (const auto& report : summary.reports) {
      if (report.outcome.has_value()) {
        std::cout << "Report for analyzer " << report.analyzerId << ": risk=" << report.outcome.value() << '\n';
      } else {
        std::cout << "Report for analyzer " << report.analyzerId << ": error=" << report.outcome.error() << '\n';
      }
    }

    std::cout << "Processed " << summary.processedSegments << " segment(s)" << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
