# Modern `constexpr` in C++ (C++11–C++26)

`constexpr` promises that an expression can be evaluated at compile time if its operands are also compile-time constants. Once restricted to trivial use cases, it now underpins meta-programming, embedded development, and aggressive optimization pipelines. This guide summarizes what the newest C++ standards allow you to do with `constexpr`, how it evolved, and how to avoid the common traps.

## Evolution at a glance

| Standard | Highlights |
| --- | --- |
| C++11 | `constexpr` functions, constructors, and objects must contain a single `return` and no branches/loops (extremely constrained). |
| C++14 | Restrictions lifted: loops, multiple statements, local variables, and `constexpr` `std::array` become practical; literal types slightly broadened. |
| C++17 | Added `constexpr` lambdas, allowed `constexpr` on `if` / `switch`, and enabled `constexpr` dynamic memory for literal types with trivial destructors. |
| C++20 | `consteval`, `constinit`, `constexpr` virtual functions, `constexpr` standard-library algorithms/containers (most of `<algorithm>`, `<numeric>`, `<string_view>`, `<complex>`, `<optional>`, `<variant>`), and immediate functions. |
| C++23/26 | Expanded `constexpr` support for `<memory>`, `<future>`, atomic operations, ranges algorithms, and proposed constexpr-friendly reflection utilities (P1240, P2996). |

## Key terminology

- **Constant expression**: Evaluates entirely during translation; usable in template arguments, non-type template parameters, array bounds, etc.
- **Literal type**: Type that can appear in a constant expression. All structural types with public members and constexpr constructors now qualify.
- **Immediate (consteval) function**: Must be evaluated at compile time; debugs template code by guaranteeing no runtime fallback.
- **`constinit` storage**: Ensures static storage duration objects are initialized before any dynamic initialization but does not imply immutability.

## Writing `constexpr` entities

### Variables

```cpp
constexpr double pi = 3.14159265358979323846;    // usable everywhere
constinit double cache_line_size = 64.0;          // compile-time init, but mutable at runtime
```

- A `constexpr` variable is implicitly `const` and must have a literal type.
- Objects with `constexpr` constructors can be marked `constexpr` even if they contain complex subobjects.
- Use `constinit` for globals that must be initialized at compile time but later mutated.

### Functions and lambdas

```cpp
constexpr int popcount(unsigned x) {
    int count = 0;
    while (x) {
        count += x & 1u;
        x >>= 1;
    }
    return count;
}

constexpr auto add = [](auto a, auto b) { return a + b; };
```

Rules:
- The function body must be valid for compile-time evaluation (no `goto`, no `try`/`catch` until C++23 relaxed some aspects, no virtual dispatch prior to C++20).
- If called with non-constant arguments, it still works at runtime.
- Use `consteval` when runtime fallback would be a bug (e.g., generating lookup tables for template metaprogramming).

### Constructors and member functions

- Mark constructors `constexpr` so aggregate initialization can occur at compile time.
- C++20 allows `constexpr` virtual functions and destructors if their entire dynamic dispatch chain is constexpr-safe.
- Member functions can be both `constexpr` and `consteval` depending on required guarantees.

### `constexpr if`

```cpp
template <typename T>
constexpr auto safe_abs(T value) {
    if constexpr (std::is_signed_v<T>) {
        return value < 0 ? -value : value;
    } else {
        return value;
    }
}
```

- `if constexpr` discards non-selected branches before instantiation, eliminating SFINAE boilerplate.
- Works inside `constexpr` and non-constexpr contexts alike.

### Interaction with the standard library

- Nearly all value-semantic containers (`std::array`, `std::span`, `std::optional`, `std::variant`, `std::tuple`) are constexpr-ready.
- Since C++20, many algorithms (`std::sort`, `std::accumulate`, ranges) have constexpr overloads, enabling compile-time transformations of small data sets.
- String-like types: `std::string_view` is constexpr-friendly; `std::string` gained constexpr constructors/mutating members in C++20/23 but still cannot allocate in constant expressions because heap allocation is forbidden.

## Best practices

1. **Design types for constexpr**: Prefer aggregate initialization, avoid virtual bases, and keep destructors trivial when possible.
2. **Provide runtime fallbacks only when needed**: If a function must never run at runtime, declare it `consteval` to fail loudly.
3. **Leverage `constexpr` tests**: Use `static_assert` to verify compile-time computations and invariants.
4. **Control compile-time cost**: Heavy constexpr computations slow down builds; cache results or move to code generation if necessary.
5. **Keep dependencies literal**: Any non-literal member (e.g., `std::mutex`) disqualifies the enclosing object from constexpr use.

## Common pitfalls

- **Odr-use vs. `constexpr`**: A `constexpr` variable with internal linkage may still violate the One Definition Rule if included across TUs without `inline`.
- **IO and system calls**: Anything requiring runtime state (files, sockets, clocks) is forbidden inside constant expressions.
- **Dynamic memory**: `new`/`delete` is generally disallowed in constant expressions except for limited forms added for literal types; prefer `std::array`.
- **`std::vector`/`std::string`**: Most operations remain non-constexpr due to heap usage; rely on `std::array` or custom fixed-capacity wrappers.
- **Compilers differ**: Always test under multiple compilers/standard versions; some vendors lag on constexpr-enabled standard library pieces.

## Debugging constexpr code

- Use `static_assert` with informative messages to pinpoint failures.
- Inspect intermediate values via `consteval` helper functions that produce type info, or by materializing `constexpr` objects and printing them through `consteval`-generated `constinit` strings.
- Many IDEs (e.g., clangd-based) show the constexpr evaluation result inline; enable these tools to reduce friction.

## Further resources

- [cppreference: constexpr specifier](https://en.cppreference.com/w/cpp/language/constexpr)
- [cppreference: Constant expressions](https://en.cppreference.com/w/cpp/language/constant_expression)
- [ISO C++ draft paper P2448R2 — Relaxing constexpr restrictions](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2448r2.pdf)
- [Herb Sutter — "CppCon constexpr ALL the Things!" (CppCon 2018 talk)](https://www.youtube.com/watch?v=5PaSp2knXRI)
- [Lewis Baker — Immediate functions (`consteval`) proposal P1073](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1073r3.html)
- [Barry Revzin — "`if constexpr` and Concepts" (blog post)](https://brevzin.github.io/c++/2017/12/16/if-constexpr/)
- [libc++ status page for constexpr algorithms](https://libcxx.llvm.org/Status/Cxx20.html)

## Checklist before using `constexpr`

- [ ] Confirm the type is literal (no virtual bases, no non-literal members).
- [ ] Ensure the function body avoids runtime-only constructs (`asm`, `reinterpret_cast` to unrelated types, blocking I/O).
- [ ] Add `static_assert` tests covering representative inputs.
- [ ] Benchmark build times if you introduce heavy compile-time work.
- [ ] Document the guarantees (`constexpr` vs. `consteval` vs. `constinit`) so future maintainers use the right interface.
