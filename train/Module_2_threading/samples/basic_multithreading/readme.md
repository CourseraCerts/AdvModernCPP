# Basic Multithreading Samples Notes

## Why `std::span` is used
`thread_pool_latest.cpp` forwards every segment to the worker as a `std::span<const double>` (see line 114). A span is a
lightweight view (pointer + length) that gives the worker read-only access to contiguous data without tying it to
`std::vector`. Because copying a span is as cheap as copying two pointers, passing it by value in the thread arguments is
as efficient as storing a reference, but it keeps the function interface generic and mutation-safe.

## Capturing `std::span` vs indexing `segments[idx]`
Each lambda currently captures the index (`idx = i`) and builds a span inside the thread (`std::span{segments[idx]}`).
This means the pointer-length pair is computed when the thread starts executing, so the lambda must dereference the
shared `segments` vector once. The overhead is negligible (just pointer arithmetic), but the lambda does still touch the
container.

If you prefer to avoid that extra indexing, you can capture the span itself when launching the thread:
```cpp
for (std::size_t i = 0; i < segments.size(); ++i) {
  auto view = std::span<const double>{segments[i]};
  guard.emplace_back([&, view] {
    // use `view` instead of `segments[idx]`
  });
}
```
This variant:
- Computes the span once on the launching thread
- Avoids every worker re-reading `segments[idx]`
- Still copies only two pointers into each lambda capture

Both approaches are valid; the difference is mainly readability. Capturing the span can make intent clearer and removes
any need for the worker to rely on `segments` at all (other than ensuring its lifetime), which may help if `segments`
needs to be moved or reallocated early.

## Further reading
- [`std::span` on cppreference](https://en.cppreference.com/w/cpp/container/span)
- [`std::thread` constructor and argument passing](https://en.cppreference.com/w/cpp/thread/thread/thread)
- [Lambda capture forms](https://en.cppreference.com/w/cpp/language/lambda#Lambda_capture)

## spal

> thread_pool_latest.cpp:114 wraps each std::vector<double> segment into std::span<const double>. A span is a lightweight, non‑owning view over
  a contiguous block of elements; it carries just a pointer + length, so compute_risk(std::span<const double>) can accept any container (vector,
  array, std::array, raw array, etc.) without overloads or templates. It also guarantees contiguous storage and size information, giving the
  function a uniform interface while preventing accidental mutation because the span is const.

In the original code (thread_pool.cpp:78) the worker receives const std::vector<double>& via std::ref. That approach ties the analyzer strictly to std::vector, which means riskAnalyzer cannot work with other contiguous containers or even a subrange of the vector without copying. std::ref is also only needed there because std::thread copies its arguments by value; the ref wrapper keeps the reference semantics.

With std::span, we avoid std::ref, because the span object itself is cheap to copy (it already behaves like a reference). It decouples the worker from the container type, lets us pass segments[idx] by value as a view, and keeps the call site cleaner while still ensuring the worker reads the same shared data.

# Lambda 2
in the function run_parallel_analysis, the line 112 seems to pass a lambda while the parameters passing to lambda does not seem to be passed.  the code compiles .. can you explain the concept and if possible give the link to the reading material on this concept

> run_parallel_analysis line 112 creates a lambda that captures what it needs rather than taking explicit parameters. The syntax [&, idx = i] { /* body */ } tells the compiler:

  - & capture default grabs every referenced outer variable by reference (e.g., segments, summary, processed, log_thread_safe).
  - idx = i is a copy capture (also called an init capture). It evaluates i when the lambda is created and stores that value inside the lambda
  object, so each worker thread keeps its own idx.

  Because everything the lambda uses comes via capture, the call operator doesn’t need parameters, so the lambda type is invocable as worker()
  without arguments. That’s why guard.emplace_back([...]{ ... }); is valid: std::thread just calls the lambda’s operator() with zero arguments.

  Good references:

  1. cppreference “Lambda expressions”: https://en.cppreference.com/w/cpp/language/lambda
  2. The section on Lambda captures there explains default capture, init capture, and why no parameters are required once captures are in place.

