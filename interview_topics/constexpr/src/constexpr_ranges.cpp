// Demonstrates constexpr-friendly ranges pipelines, combining filter/transform
// views and compile-time diagnostics to summarize sensor readings.
#include <array>
#include <iostream>
#include <ranges>

struct is_even {
    constexpr bool operator()(int value) const { return (value % 2) == 0; }
};

struct square {
    constexpr int operator()(int value) const { return value * value; }
};

constexpr std::array<int, 8> sensor_readings{7, 4, 5, 2, 10, 11, 6, 8};

constexpr int even_sum = [] {
    int total = 0;
    for (int value : sensor_readings | std::views::filter(is_even{}) | std::views::transform(square{})) {
        total += value;
    }
    return total;
}();

constexpr bool increasing_tail = [] {
    auto tail = sensor_readings | std::views::drop(2);
    int prev = -1;
    for (int value : tail) {
        if (prev != -1 && value < prev) {
            return false;
        }
        prev = value;
    }
    return true;
}();

static_assert(even_sum == (4 * 4 + 2 * 2 + 10 * 10 + 6 * 6 + 8 * 8), "Unexpected even sum");
static_assert(increasing_tail == false, "Tail order detection broken");

int main() {
    std::cout << "Sum of squared even readings = " << even_sum << '\n';
    std::cout << "Tail is strictly non-decreasing? " << std::boolalpha << increasing_tail << '\n';
}
