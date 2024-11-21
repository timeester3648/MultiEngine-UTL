// __________ BENCHMARK FRAMEWORK & LIBRARY  __________

#include "benchmark.hpp"

#include "module_log.hpp"
#include <chrono>
#include <climits>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>


// _____________ BENCHMARK IMPLEMENTATION _____________

void benchmark_stringification() {
    // Benchmark stringification


    const auto get_rand_int               = []() { return utl::random::rand_int(-500, 500); };
    const auto get_rand_double            = []() { return utl::random::rand_double(-1e+7, 1e+3); };
    const auto get_rand_vector_of_strings = []() {
        return std::vector{utl::shell::random_ascii_string(8), utl::shell::random_ascii_string(8),
                           utl::shell::random_ascii_string(8), utl::shell::random_ascii_string(8)};
    };

    constexpr int repeats = 100'000;

    bench.timeUnit(millisecond, "ms").minEpochIterations(20);

    // Integer stringification
    bench.title("'int' Stringification").warmup(10).relative(true);

    benchmark("utl::log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) utl::log::append_stringified(str, get_rand_int());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= utl::log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += utl::log::stringify(get_rand_int());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= std::to_string()", [&]() {
        std::string str;
        REPEAT(repeats) str += std::to_string(get_rand_int());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream <<", [&]() {
        std::ostringstream oss;
        REPEAT(repeats) oss << get_rand_int();
        const std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    // Note:
    // += std::to_string() is surprisingly fast (I assume due to SSO) and sometimes even beats <charconv>
    // below ~6 digits (by ~5-10%). For large integers <charconv> seems to win in the end (~40% speedup
    // for values near INT_MAX).

    // Float stringification
    bench.title("'double' Stringification").warmup(10).relative(true);

    benchmark("utl::log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) utl::log::append_stringified(str, get_rand_double());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= utl::log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += utl::log::stringify(get_rand_double());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= std::to_string()", [&]() {
        std::string str;
        REPEAT(repeats) str += std::to_string(get_rand_double());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream <<", [&]() {
        std::ostringstream oss;
        REPEAT(repeats) oss << get_rand_double();
        const std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    // Vector stringification
    bench.title("std::vector<std::string> Stringification").warmup(10).relative(true);

    benchmark("utl::log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) utl::log::append_stringified(str, get_rand_vector_of_strings());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= utl::log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += utl::log::stringify(get_rand_vector_of_strings());
        DO_NOT_OPTIMIZE_AWAY(str);
    });
    
    benchmark("std::ostringstream << + loops", [&]() {
        std::ostringstream oss;
        REPEAT(repeats) {
            const auto vec = get_rand_vector_of_strings();
            oss << "{ ";
            for (auto it = vec.begin();;) {
                oss << *it;
                if (++it != vec.end()) oss << ", ";
                else break;
            }
            oss << " }";
        }
        const std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });
}

void benchmark_int_logging() {
    using namespace utl;

    // Benchmark logging
    bench.title("Logging").timeUnit(nanosecond, "ns").minEpochIterations(5).warmup(10).relative(true);

    constexpr int repeats = 500;

    log::add_file_sink("temp/log1.log");
    std::ofstream log_file("temp/log2.log");

    benchmark("utl::log", [&]() {
        REPEAT(repeats)
        UTL_LOG_INFO(utl::random::rand_int(-1000, 500), utl::shell::random_ascii_string(12),
                     utl::random::rand_double());
    });

    benchmark("flushed std::ostream::<<", [&]() {
        REPEAT(repeats)
        log_file << "2024-11-19 21:37:55 (    0.003)[     1]           benchmark_log.cpp:91    INFO|"
                 << utl::random::rand_int(-1000, 500) << utl::shell::random_ascii_string(12)
                 << utl::random::rand_double() << '\n'
                 << std::flush;
    });

    benchmark("buffered std::ostream::<<", [&]() {
        REPEAT(repeats)
        log_file << "2024-11-19 21:37:55 (    0.003)[     1]           benchmark_log.cpp:91    INFO|"
                 << utl::random::rand_int(-1000, 500) << utl::shell::random_ascii_string(12)
                 << utl::random::rand_double() << '\n';
    });
}

int main() {
    using namespace utl;

    //benchmark_stringification();
    // benchmark_int_logging();

    log::add_terminal_sink(std::cout, log::Verbosity::TRACE);

    UTL_LOG_INFO("Value is ", 5);
    UTL_LOG_WARN("But it should be ", 4);
    UTL_LOG_ERR("IT SHOULD BE ", 4);
    UTL_LOG_INFO("Nonetheless we continue to look for a better value");
    UTL_LOG_TRACE("Perhaps we could find something else");
    UTL_LOG_TRACE("Like 2");
    UTL_LOG_TRACE("2 would work great, right?");
    UTL_LOG_TRACE("Perhaps we could even combine them if we find both");
    UTL_LOG_TRACE("And get 42...");
    UTL_LOG_TRACE("Nah, that would answer too many questions, can't have that");
    UTL_LOG_INFO("Here, have a vector: ", std::vector{1, 2, 3});
    UTL_LOG_INFO("Here, have a tuple: ", std::tuple{1, 'a', 3.14});
    UTL_LOG_INFO("Here, have a super tuple: ", std::tuple{
                                                   std::tuple{'k', 16},
                                                   "text", std::vector{4, 5, 6}
    });
    UTL_LOG_INFO("Here, have a map: ", std::map{
                                           std::pair{"key_1", 1},
                                           std::pair{"key_2", 2}
    });
}