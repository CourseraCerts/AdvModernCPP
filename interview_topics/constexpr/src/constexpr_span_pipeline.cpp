// Highlights how std::span works in constexpr contexts by computing prefix
// sums and detecting threshold-breaking sliding windows entirely at compile
// time before printing the results.
#include <array>
#include <cstddef>
#include <iostream>
#include <span>

// Computes prefix sums of a span with static extent so it is constexpr-friendly.
template <std::size_t N>
constexpr std::array<int, N> prefix_sums(std::span<const int, N> data) {
    std::array<int, N> result{};
    int running = 0;
    for (std::size_t i = 0; i < N; ++i) {
        running += data[i];
        result[i] = running;
    }
    return result;
}

// Sliding window helper that works with dynamic-extent spans.
constexpr int first_window_exceeding(std::span<const int> data, std::size_t window, int threshold) {
    if (window == 0 || window > data.size()) {
        return -1;
    }
    int sum = 0;
    for (std::size_t i = 0; i < window; ++i) {
        sum += data[i];
    }
    if (sum > threshold) {
        return 0;
    }
    for (std::size_t start = 1; start + window <= data.size(); ++start) {
        sum += data[start + window - 1];
        sum -= data[start - 1];
        if (sum > threshold) {
            return static_cast<int>(start);
        }
    }
    return -1;
}

constexpr std::array<int, 6> traffic_counts{5, 8, 4, 9, 6, 7};
constexpr auto prefix = prefix_sums(std::span<const int, traffic_counts.size()>(traffic_counts));
static_assert(prefix[5] == 39, "Prefix sum mismatch");

constexpr int warning_window = first_window_exceeding(std::span<const int>(traffic_counts), 3, 20);
static_assert(warning_window == 1, "Window detection failed");

int main() {
    std::cout << "Prefix sums: ";
    for (int value : prefix) {
        std::cout << value << ' ';
    }
    std::cout << '\n';

    std::cout << "First window exceeding threshold starts at index " << warning_window << '\n';
}
