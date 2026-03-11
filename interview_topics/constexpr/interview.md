# constexpr Interview Deck

## Card 01 - Purpose of constexpr
What problem does `constexpr` solve in C++?

---
It ensures specific computations happen at compile time so their results can be embedded in templates, array bounds, or other constant-required contexts while reducing runtime cost.

## Card 02 - constexpr vs const
How is `constexpr` different from `const`?

---
`const` only prevents mutation, while `constexpr` additionally requires compile-time initialization, making the object usable wherever a constant expression is needed.

## Card 03 - When to use consteval
When would you choose `consteval` instead of `constexpr`?

---
Use `consteval` for immediate functions where runtime evaluation would be a bug, such as generating lookup tables or parsing literals during translation; it enforces compile-time execution and fails otherwise.

## Card 04 - constinit relationship
Explain `constinit` and how it relates to `constexpr`.

---
`constinit` guarantees compile-time initialization for objects with static storage but allows mutation afterward, whereas `constexpr` implies immutability and constant-expression usability.

## Card 05 - Runtime behavior
Can every `constexpr` function run at runtime?

---
Yes; `constexpr` functions can still execute at runtime when given non-constant inputs, unlike `consteval` which must always evaluate during compilation.

## Card 06 - Literal types
What are literal types and why do they matter for `constexpr`?

---
Literal types can appear in constant expressions; they require constexpr constructors, trivial destructors, and no virtual bases so the compiler can instantiate them at compile time.

## Card 07 - C++14 changes
How did C++14 change the usability of `constexpr` functions?

---
C++14 removed the single-return restriction, allowing loops, locals, and branching inside constexpr functions, making real algorithms feasible at compile time.

## Card 08 - if constexpr vs SFINAE
Give an example where `if constexpr` is preferable to SFINAE.

---
In a templated `serialize` routine, `if constexpr` can branch between streaming, tuple iteration, or fallback behavior based on traits without writing separate overloads or relying on substitution failure tricks.

## Card 09 - std::vector in constexpr
Can `std::vector` be used in constant expressions today?

---
No; despite gaining constexpr-friendly members, it still performs heap allocations which are forbidden in constant expressions, so vectors remain runtime-only.

## Card 10 - Testing constexpr code
How do you test constexpr code?

---
Create `constexpr` objects or call constexpr functions inside `static_assert` checks so the compiler validates the results during translation.

## Card 11 - Exceptions in constexpr
Is exception handling allowed inside a `constexpr` function?

---
Throwing is disallowed during constant evaluation, but C++23 allows `try`/`catch` syntax as long as no exception is actually thrown while evaluating as constexpr.

## Card 12 - I/O limitations
What happens if a `constexpr` function performs I/O?

---
Any I/O makes the expression non-constant; the compiler rejects such calls when they must be evaluated at compile time, though the same function can still run at runtime.

## Card 13 - Templates interaction
How do `constexpr` and templates interact?

---
Templates often need compile-time values for non-type parameters; constexpr helpers compute those values cleanly, reducing template metaprogramming boilerplate.

## Card 14 - Virtual functions
Are virtual functions allowed to be `constexpr`?

---
Since C++20, virtual functions may be `constexpr` provided each override remains constexpr-compatible and the dispatch can be resolved during constant evaluation.

## Card 15 - Constructors
What is a `constexpr` constructor useful for?

---
It lets complex objects—like geometry primitives or configuration tables—be created at compile time so they can participate in constant expressions and live in read-only memory.

## Card 16 - constexpr lambdas
Does marking a lambda `constexpr` change anything after C++17?

---
Lambdas are implicitly `constexpr` when possible starting in C++17, so the specifier mainly documents intent while still enforcing compile-time evaluability.

## Card 17 - Code generation impact
How does `constexpr` influence code generation?

---
Compile-time known values can be folded, inlined, or precomputed into lookup tables, eliminating branches and memory loads during execution.

## Card 18 - Data size limits
What limits the size of `constexpr` data structures?

---
The standard imposes no explicit limit, but compiler memory and build times can balloon, so large constexpr data may exhaust resources.

## Card 19 - Destructors
Can `constexpr` be applied to destructors?

---
Destructors themselves cannot be `constexpr`; they just need to be constexpr-compatible (typically trivial) so that temporary constexpr objects can be destroyed during evaluation.

## Card 20 - Mixing specifiers
Describe a scenario mixing `consteval`, `constexpr`, and `constinit`.

---
A `consteval` parser ingests a configuration literal, returns a `constexpr` object used throughout the program, and stores it in a `constinit inline` global so every translation unit shares the precomputed data.
