// __________ BENCHMARK FRAMEWORK & LIBRARY  __________

#include "benchmark.hpp"

#include "module_log.hpp"
#include <chrono>
#include <climits>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>


// _____________ BENCHMARK IMPLEMENTATION _____________

void benchmark_int_stringification() {
    // Benchmark stringification
    bench.title("Integer Stringification").minEpochIterations(60).warmup(10).relative(true);
    
    const auto get_rand_int = [](){ return utl::random::rand_int(-500, 500); };
    
    constexpr int repeats = 100'000;
    
    benchmark("utl::log::stringify_integer()", [&]() {
        std::string str;
        REPEAT(repeats) utl::log::stringify_integer(str, get_rand_int());
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
}

void benchmark_int_logging() {
    using namespace  utl;
    
    // Benchmark logging
    bench.title("Logging").minEpochIterations(20).warmup(10).relative(true);
    
    constexpr int repeats = 20'000;
    
    log::Columns all_columns = {true, true, true, true, true, true};
    
    std::ofstream log_file("temp/log.log");
    log::Logger::instance().add_sink(log_file, log::Verbosity::INFO, log::Colors::DISABLE, all_columns,
                                     std::chrono::milliseconds(50));
    
    benchmark("utl::log", [&]() {
        REPEAT(repeats)
        UTL_LOG_INFO(utl::random::rand_int(-1000, 500), utl::shell::random_ascii_string(12),
                     utl::random::rand_double());
    });
    
    benchmark("flushed std::ostream::<<", [&]() {
        REPEAT(repeats)
        log_file << "2024-11-18 22:47:57 (    .013)[1]           benchmark_log.cpp:23   WARN |"
                 << utl::random::rand_int(-1000, 500) << utl::shell::random_ascii_string(12)
                 << utl::random::rand_double() << '\n'
                 << std::flush;
    });
    
    benchmark("buffered std::ostream::<<", [&]() {
        REPEAT(repeats)
        log_file << "2024-11-18 22:47:57 (    .013)[1]           benchmark_log.cpp:23   WARN |"
                 << utl::random::rand_int(-1000, 500) << utl::shell::random_ascii_string(12)
                 << utl::random::rand_double() << '\n';
    });
}

int main() {
    using namespace utl;
    
    // Set global benchmark options
    bench.timeUnit(millisecond, "ms");

    benchmark_int_stringification();
    benchmark_int_logging();

    log::Logger::instance().add_sink(std::cout, log::Verbosity::INFO, log::Colors::ENABLE, {true, true, true, true, true, true},
                                     std::chrono::milliseconds(0));

    UTL_LOG_INFO("Value is ", 5);
    UTL_LOG_WARN("But it should be ", 4);
    UTL_LOG_ERR("IT SHOULD BE ", 4);
    UTL_LOG_INFO("Nonetheless we continue to look for a better value");
    UTL_LOG_INFO("Perhaps we could find something else");
    UTL_LOG_INFO("Like 2");
    UTL_LOG_INFO("2 would work great, right?");
    UTL_LOG_INFO("Perhaps we could even combine them if we find both");
    UTL_LOG_INFO("And get 42...");
    UTL_LOG_INFO("Nah, that would answer too many questions, can't have that");
}