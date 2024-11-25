// __________ BENCHMARK FRAMEWORK & LIBRARY  __________

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <string_view>
#include <thread>
#include <variant>

#include "benchmark.hpp"

#include "thirdparty/nanobench.h"

#if defined(__x86_64__)
#include <x86intrin.h> // testing
#endif

// ________________ BENCHMARK INCLUDES ________________

#include <cmath>
#include <vector>

// _____________ BENCHMARK IMPLEMENTATION _____________

const auto compute_value = []() { return std::atan(std::pow(std::cos(std::sin(utl::random::rand_double())), 0.5)); };
// some kind of computation, just heavy enough to be measurable, yet fast enough to not overshadow profiling overhead

void benchmark_profiling_overhead() {
    constexpr int repeats = 50'000;

    bench.minEpochIterations(10).timeUnit(millisecond, "ms").relative(true);

    benchmark("UTL_PROFILE()", [&]() {
        double s = 0.;
        REPEAT(repeats) { UTL_PROFILER("Work profiler") s += compute_value(); }
        DO_NOT_OPTIMIZE_AWAY(s);
    });

    benchmark("Theoretical best std::chrono profiler", [&]() {
        double                   s = 0.;
        std::chrono::nanoseconds time{};
        REPEAT(repeats) {
            const auto start = std::chrono::steady_clock::now();
            s += compute_value();
            time += std::chrono::steady_clock::now() - start;
        }
        DO_NOT_OPTIMIZE_AWAY(s);
        DO_NOT_OPTIMIZE_AWAY(time);
    });

#if defined(__x86_64__)
    benchmark("Theoretical best __rdtsc() profiler", [&]() {
        double   s = 0.;
        uint64_t time{};
        REPEAT(repeats) {
            const uint64_t start = __rdtsc();
            s += compute_value();
            time += __rdtsc() - start;
        }
        DO_NOT_OPTIMIZE_AWAY(s);
        DO_NOT_OPTIMIZE_AWAY(time);
    });
#endif

    benchmark("Runtime without profiling", [&]() {
        double s = 0.;
        REPEAT(repeats) { s += compute_value(); }
        DO_NOT_OPTIMIZE_AWAY(s);
    });

    // Here 'theoretical best profiler' is the one that has no additional overhead besides
    // the time measurement at two points, can't get better than without swithing to a
    // completely different profiling method (like, for example, sampling or CPU instruction modeling)
}

void test_scope_profiler_precision() {
    UTL_PROFILER("Scope precision test:   50 ms") utl::sleep::spinlock(50);
    UTL_PROFILER("Scope precision test:  200 ms") utl::sleep::spinlock(200);
    UTL_PROFILER("Scope precision test: 1000 ms") utl::sleep::spinlock(1000);
}

void test_segment_profiler_precision() {
    UTL_PROFILER_BEGIN(segment_1, "Segment precision test:   50 ms");
    utl::sleep::spinlock(50);
    UTL_PROFILER_END(segment_1);
    
    UTL_PROFILER_BEGIN(segment_2, "Segment precision test:   200 ms");
    utl::sleep::spinlock(200);
    UTL_PROFILER_END(segment_2);
    
    UTL_PROFILER_BEGIN(segment_3, "Segment precision test:  1000 ms");
    utl::sleep::spinlock(1000);
    UTL_PROFILER_END(segment_3);
}

double recursive_function(int recursion_depth) {
    if (recursion_depth > 2) {
        utl::sleep::spinlock(1.25);
        return utl::random::rand_double();
    }

    double s1 = 0, s2 = 0, s3 = 0;

    UTL_PROFILER_EXCLUSIVE("Recursive function (1st branch)") { s1 = recursive_function(recursion_depth + 1); }

    UTL_PROFILER_EXCLUSIVE("Recursive function (2nd branch)") {
        s2 = recursive_function(recursion_depth + 1);
        s3 = recursive_function(recursion_depth + 1);
    }

    return s1 + s2 + s3;
}

void test_profiler_recursion_handling() {
    double sum = recursive_function(0);

    DO_NOT_OPTIMIZE_AWAY(sum);
}

void computation_1() { std::this_thread::sleep_for(std::chrono::milliseconds(300)); }
void computation_2() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); }
void computation_3() { std::this_thread::sleep_for(std::chrono::milliseconds(400)); }
void computation_4() { std::this_thread::sleep_for(std::chrono::milliseconds(600)); }
void computation_5() { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }

int main() {
    //benchmark_profiling_overhead();
    //test_scope_profiler_precision();
    //test_segment_profiler_precision();
    //test_profiler_recursion_handling();
    
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
}