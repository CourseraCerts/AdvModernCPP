// Demonstrates constexpr numeric algorithms that precompute factorials and
// Fibonacci tables at compile time, then reuse those results at runtime.
#include <array>
#include <cstdint>
#include <iostream>

// Simple loop-based factorial usable as a constant expression.
constexpr std::uint64_t factorial(std::uint64_t n) {
    std::uint64_t result = 1;
    for (std::uint64_t i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

// Generates a Fibonacci lookup table entirely at compile time.
constexpr auto fibonacci_table() {
    std::array<std::uint64_t, 12> values{};
    values[0] = 0;
    values[1] = 1;
    for (std::size_t i = 2; i < values.size(); ++i) {
        values[i] = values[i - 1] + values[i - 2];
    }
    return values;
}

constexpr auto fib_values = fibonacci_table();
static_assert(fib_values[10] == 55, "Fibonacci table generation failed");
static_assert(factorial(5) == 120, "Factorial must be correct at compile time");

int main() {
    std::cout << "factorial(10) = " << factorial(10) << '\n';
    std::cout << "First 12 Fibonacci numbers: ";
    for (auto value : fib_values) {
        std::cout << value << ' ';
    }
    std::cout << '\n';
}
