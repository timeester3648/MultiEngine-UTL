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

#include "MACRO_PROFILER.hpp"
#include "thirdparty/nanobench.h"

#include <x86intrin.h> // testing

// ________________ BENCHMARK INCLUDES ________________

#include <cmath>
#include <vector>

// _____________ BENCHMARK IMPLEMENTATION _____________

constexpr int repeats = 50'000;

double compute_value() { return std::atan(std::pow(std::cos(std::sin(utl::random::rand_double())), 0.5)); }

double recursive_func(int recursion_level) {
    if (recursion_level == 0) return 1.;
    return (compute_value() + recursive_func(recursion_level - 1)) * recursive_func(recursion_level - 2);
}

int main() {

    bench.minEpochIterations(10).timeUnit(millisecond, "ms").relative(true);

    // Baseline
    benchmark("UTL_PROFILE()", [&]() {
        double s = 0.;
        REPEAT(repeats) { UTL_PROFILER("Work profiler") s += compute_value(); }
        DO_NOT_OPTIMIZE_AWAY(s);
    });

    benchmark("Theoretical best std::chrono profile", [&]() {
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

    benchmark("Theoretical best __rdtsc() profile", [&]() {
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
    
    benchmark("Runtime without profiling", [&]() {
        double   s = 0.;
        REPEAT(repeats) {
            s += compute_value();
        }
        DO_NOT_OPTIMIZE_AWAY(s);
    });
}