// Uses a consteval parser to enforce compile-time validation of semantic
// version literals and publishes the result via constexpr/constinit globals.
#include <array>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string_view>

struct Version {
    int major;
    int minor;
    int patch;
};

// Enforce compile-time parsing of a semantic version literal.
consteval Version parse_version(std::string_view text) {
    auto read_number = [&](std::size_t& pos) {
        if (pos >= text.size() || !std::isdigit(static_cast<unsigned char>(text[pos]))) {
            throw std::logic_error{"Expected digit"};
        }
        int value = 0;
        while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos]))) {
            value = value * 10 + (text[pos] - '0');
            ++pos;
        }
        return value;
    };

    std::size_t pos = 0;
    int major = read_number(pos);
    if (pos >= text.size() || text[pos] != '.') throw std::logic_error{"Expected '.'"};
    ++pos;
    int minor = read_number(pos);
    if (pos >= text.size() || text[pos] != '.') throw std::logic_error{"Expected '.'"};
    ++pos;
    int patch = read_number(pos);
    if (pos != text.size()) throw std::logic_error{"Unexpected trailing characters"};

    return Version{major, minor, patch};
}

constexpr Version engine_version = parse_version("2.7.3");
constinit Version runtime_version = engine_version; // could be mutated later if needed

int main() {
    std::cout << "Engine version: " << runtime_version.major << '.'
              << runtime_version.minor << '.' << runtime_version.patch << '\n';
}
