# UTL_PROFILER

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**URL_PROFILER** module contains macros for quick scope profiling and micro-benchmarking with x86 intrinsics (GCC/clang only).

## Definitions

```cpp
UTL_PROFILER(label);

UTL_PROFILER_EXCLUSIVE(label);
    
UTL_PROFILER_BEGIN(segment_label, label);
UTL_PROFILER_END(segment_label);

UTL_PROFILER_EXCLUSIVE_BEGIN(segment_label, label);
UTL_PROFILER_EXCLUSIVE_END(segment_label);

using clock; // alias for 'std::chrono::steady_clock' or a custom implementetion, depending on macro- options
using duration   = clock::duration;
using time_point = clock::time_point;
```

## Methods

> ```cpp
> UTL_PROFILER(label)
> ```

Profiles the following scope or expression. If profiled scope was entered at any point of the program, upon exiting `main()` the table with profiling results will be printed. Profiling results include:

- Total program runtime
- Total runtime of each profiled scope
- % of total runtime taken by each profiled scope
- Profiler **labels**
- Profiler call-sites: file, function, line

**Note:** Multiple profilers can exist at the same time. Profiled scopes can be nested. Profiler overhead corresponds to entering & exiting the profiled scope, while insignificant in most applications, it may affect runtime in a tight loop.

> ```cpp
> UTL_PROFILER_EXCLUSIVE(label)
> ```

Similar to a `UTL_PROFILER`, but unlike a regular case only one `UTL_PROFILER_EXCLUSIVE` can exit at the same time. This is useful for profiling recursive functions, see [recursion profiling example](#profiling-recursion) and [why recursion is a rather non-trivial thing to measure](#why-recursion-is-a-rather-non-trivial-thing-to-measure).

## Examples

### Profiling a Scope

[ [Run this code]() ]
```cpp
void computation_1() { std::this_thread::sleep_for(std::chrono::milliseconds(300)); }
void computation_2() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); }
void computation_3() { std::this_thread::sleep_for(std::chrono::milliseconds(400)); }
void computation_4() { std::this_thread::sleep_for(std::chrono::milliseconds(600)); }
void computation_5() { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }

// ...

// Profile a scope
UTL_PROFILER("Computation 1 & 2") {
    computation_1();
    computation_2();
}

// Profile a single statement
UTL_PROFILER("Computation 3") computation_3();

// Profile a code segment
UTL_PROFILER_BEGIN(segment_label, "Computation 4 & 5");
computation_4();
computation_5();
UTL_PROFILER_END(segment_label);
```

Output:
```

```

### Nested Profilers & Loops

[ [Run this code]() ]
```cpp
void some_function() {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

// ...

// Profile how much of a loop runtime is spent inside 'some_function()'
UTL_PROFILER("whole loop")
for (int i = 0; i < 5; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    UTL_PROFILER("some_function()") some_function();
}

// we expect to see that 'some_function()' will measure ~half the time of the 'whole loop'
```

Output:
```

```

### Profiling Recursion

[ [Run this code]() ]
```cpp
double some_computation() {
    utl::sleep::spinlock(1);
    return utl::random::rand_double();
}

double recursive_function(int recursion) {
    if (recursion > 5) return some_computation();
    
    UTL_PROFILER_BEGIN_EXCLUSIVE(segment_1, "1st recursion branch");
    const double s1 = recursive_function(recursion + 1);
    UTL_PROFILER_END_EXCLUSIVE(segment_1)
    
    UTL_PROFILER_BEGIN_EXCLUSIVE(segment_1, "2nd recursion branch");
    const double s2 = recursive_function(recursion + 1);
    const double s3 = recursive_function(recursion + 1);
    UTL_PROFILER_END_EXCLUSIVE(segment_1)
    
    return s1 + s2 + s3;
}

// ...

std::cout << "SUM = " << recursive_function(0);

// we expect that '1st recursion branch' will measure ~33% and
// '2nd recursion branch' will measure ~66%
```

Output:
```

```

## Why Recursion is a Rather Non-trivial Thing to Measure

Let's imagine we have a recursive function `f()` that calls 2 instances of itself recursively. Let's limit recursion depth to **2** and define following variables:

- $t$ —  time spent on a single recursion tail-call;
- $T$ — total time spent inside the recursive function `f()`;
- $T_1$ — total time spent inside the **1st** branch of recursion;
- $T_2$ — total time spent inside the **2nd** branch of recursion.

The function will end up with a following call graph:

<img src ="images/profiler_recursion_call_graph.svg">

If we profile  **1st** and **2nd** branches independently, we will measure following parts of the call graph:

<img src ="images/profiler_recursion_independent_profiling_branch_1.svg">

<img src ="images/profiler_recursion_independent_profiling_branch_2.svg">

Which means $T_1 + T_2 \neq T$, which goes against the logic of what were actually trying to measure. In essense, when profiling recursion, different profilers should be aware of each other and not invoke measurement while inside the call graph of a another already existing profiler.

This is exactly the problem solved by `UTL_PROFILER_EXCLUSIVE()`, as it guarantees no other profiler will be invoked under an already existing one. We will end up with a following call graph:

<img src ="images/profiler_recursion_exclusive_profiling_branch_1.svg">

<img src ="images/profiler_recursion_exclusive_profiling_branch_2.svg">

which corresponds to the parts we were trying to measure and satisfied $T_1 + T_2 = T$. The same logic can be generalized to an arbitrary recursion with $N$ different profilers.

## Microbenchmarking With x86 Intrinsics

To enable the use of intrinsics add folowing `#define` before including the `proto_utils` header:

```cpp
#define UTL_PROFILER_OPTION_USE_x86_INTRINSICS_FOR_FREQUENCY 3'300'000'000
// I have 3.3 GHz CPU in this example (AMD Ryzen 5 5600H)

#include "proto_utils.hpp"
```

This will switch `profiler::clock` from `std::chrono::steady_clock` to a custom [`<chrono>`](https://en.cppreference.com/w/cpp/chrono)-compatible implementation using GCC/clang [RDTSC x86 intrinsic](https://en.wikipedia.org/wiki/Time_Stamp_Counter) that is likely to be DRASTICALLY faster at getting time that std-lib solutions like `std::chrono::steady_clock` or `ctime()`.

This is exceedingly helpful when benchmarking code on a hot path, however it comes at a price of producing a non-portable executable. Below are a few [benchmarks](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/benchmarks/benchmark_profiler.cpp) showcasing the difference on a particular hardware:

```
======= USING std::chrono ========

| relative |               ms/op |                op/s |    err% |     total | benchmark
|---------:|--------------------:|--------------------:|--------:|----------:|:----------
|   100.0% |                5.15 |              194.16 |    1.2% |      0.62 | `UTL_PROFILE()`
|   102.5% |                5.02 |              199.04 |    0.3% |      0.61 | `Theoretical best std::chrono profiler`
|   209.8% |                2.45 |              407.38 |    0.2% |      0.30 | `Theoretical best __rdtsc() profiler`
|   233.8% |                2.20 |              453.91 |    1.4% |      0.26 | `Runtime without profiling`

====== USING x86 INTRINSICS ======

| relative |               ms/op |                op/s |    err% |     total | benchmark
|---------:|--------------------:|--------------------:|--------:|----------:|:----------
|   100.0% |                2.53 |              394.91 |    0.4% |      0.33 | `UTL_PROFILE()`
|    49.9% |                5.07 |              197.09 |    0.2% |      0.61 | `Theoretical best std::chrono profiler`
|   102.8% |                2.46 |              405.79 |    0.3% |      0.30 | `Theoretical best __rdtsc() profiler`
|   116.8% |                2.17 |              461.13 |    0.5% |      0.26 | `Runtime without profiling`
```

> [!Note]
> Here *"theoretical best"* refers to a hypothetical profiler that requires zero operations aside from measuring the time at two points  — before and after entering the code segment.