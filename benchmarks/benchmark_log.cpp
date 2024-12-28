// __________ BENCHMARK FRAMEWORK & LIBRARY  __________

#include "benchmark.hpp"

#include <chrono>
#include <climits>
#include <complex>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>


// _____________ BENCHMARK IMPLEMENTATION _____________

// ==================================
// --- Stringification benchmarks ---
// ==================================

void benchmark_stringification() {
    using namespace utl;

    const auto get_rand_int               = []() { return random::rand_int(-5000, 5000); };
    const auto get_rand_double            = []() { return random::rand_double(-1e+7, 1e+3); };
    const auto get_rand_complex           = []() { return std::complex{random::rand_double(), random::rand_double()}; };
    const auto get_rand_bool              = []() { return random::rand_bool(); };
    const auto get_rand_char              = []() { return static_cast<char>(random::rand_int('a', 'z')); };
    const auto get_rand_vector_of_strings = []() {
        return std::vector{shell::random_ascii_string(8), shell::random_ascii_string(8), shell::random_ascii_string(8),
                           shell::random_ascii_string(8)};
    };
    const auto get_rand_string              = []() { return shell::random_ascii_string(8); };

    constexpr int repeats = 20'000;

    bench.timeUnit(nanosecond, "ms").minEpochIterations(20);

    // --- Integer stringification ---
    // -------------------------------
    bench.title("Stringify bulk 'int' data").warmup(10).relative(true);

    benchmark("log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) log::append_stringified(str, get_rand_int());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += log::stringify(get_rand_int());
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
    // += std::to_string() is surprisingly fast (I assume due to SSO) and sometimes even beats <charconv>.
    // I assume since there is no "variety" in how integers can be formatted, nothing prevent implementations
    // from using <charconv> algorithms inside the 'std::to_string()', in which case there is little room for
    // further optimization left.

    // --- Float stringification ---
    // ------------------------------
    bench.title("Stringify bulk 'double' data").warmup(10).relative(true);

    benchmark("log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) log::append_stringified(str, get_rand_double());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += log::stringify(get_rand_double());
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

    // --- Complex stringification ---
    // -------------------------------
    bench.title("Stringify bulk 'std::complex<double>' data").warmup(10).relative(true);

    benchmark("log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) log::append_stringified(str, get_rand_complex());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += log::stringify(get_rand_complex());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= std::to_string() (component-wise)", [&]() {
        std::string str;
        REPEAT(repeats) {
            str += std::to_string(get_rand_complex().real());
            str += " + ";
            str += std::to_string(get_rand_complex().imag());
            str += " i";
        }
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream <<", [&]() {
        std::ostringstream oss;
        REPEAT(repeats) oss << get_rand_complex();
        const std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    // --- Bool stringification ---
    // -------------------------------
    bench.title("Stringify bulk 'bool' data").warmup(10).relative(true);

    benchmark("log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) log::append_stringified(str, get_rand_bool());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += log::stringify(get_rand_bool());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream << (with std::boolalpha)", [&]() {
        std::ostringstream oss;
        oss << std::boolalpha;
        REPEAT(repeats) oss << get_rand_bool();
        const std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    // --- Vector of strings stringification ---
    // -----------------------------------------
    bench.title("Stringify bulk 'std::vector<std::string>' data").warmup(10).relative(true);

    benchmark("log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) log::append_stringified(str, get_rand_vector_of_strings());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += log::stringify(get_rand_vector_of_strings());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream << (iterating the vector)", [&]() {
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

    // --- Error message stringification ---
    // -------------------------------------

    // In this benchmark we want to format as simple error message and return it as 'std::string',
    // stringstream will have to be temporary for that purpose
    bench.title("Format an error message")
        .timeUnit(nanosecond, "ns")
        .minEpochIterations(1000)
        .warmup(10)
        .relative(true);

    benchmark("log::stringify()", [&]() {
        std::string str = log::stringify("JSON object node encountered unexpected symbol {", get_rand_char(),
                                         "} after the pair key at pos ", get_rand_int(), " (should be {:}).");
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::string + std::string", [&]() {
        using namespace std::string_literals;
        std::string str = "JSON object node encountered unexpected symbol {" + std::to_string(get_rand_int()) +
                          "} after the pair key at pos " + std::to_string(get_rand_int()) + " (should be {:}).";
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream <<", [&]() {
        std::ostringstream oss;
        oss << "JSON object node encountered unexpected symbol {" << get_rand_char() << "} after the pair key at pos "
            << get_rand_int() << " (should be {:}).";
        std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });
    
    // --- Left-pad formatting ---
    // ---------------------------
    bench.title("Left-pad a string")
        .timeUnit(nanosecond, "ns")
        .minEpochIterations(20)
        .warmup(10)
        .relative(true);
        
    constexpr std::size_t pad_size = 20;
    
    std::vector<std::string> strings(10'000);
    const auto reset_strings = [&]{ for (auto& e :strings) e = ""; };
    
    reset_strings();
    benchmark("log::stringify(log::PadLeft{})", [&]() {
        for (auto& e :strings) e = log::stringify(log::PadLeft{ get_rand_string(), pad_size });
        DO_NOT_OPTIMIZE_AWAY(strings);
    });
    
    reset_strings();
    benchmark("Manual alignment with std::string.append()", [&]() {
        for (auto& e :strings) {
            const std::string temp = get_rand_string();
            if (temp.size() < pad_size) e.append(pad_size - temp.size(), ' ');
            e += temp;
        }
        DO_NOT_OPTIMIZE_AWAY(strings);
    });
    
    reset_strings();
    benchmark("std::ostringstream << std::setw() << std::right", [&]() {
        for (auto& e :strings) {
            std::ostringstream oss;
            oss << std::setw(pad_size) << std::right << get_rand_string();
            e = oss.str();
        }
        DO_NOT_OPTIMIZE_AWAY(strings);
    });
    
    // --- Right-pad formatting ---
    // ---------------------------
    bench.title("Right-pad a string")
        .timeUnit(nanosecond, "ns")
        .minEpochIterations(20)
        .warmup(10)
        .relative(true);
        
    reset_strings();
    benchmark("log::stringify(log::PadRight{})", [&]() {
        for (auto& e :strings) e = log::stringify(log::PadRight{ get_rand_string(), pad_size });
        DO_NOT_OPTIMIZE_AWAY(strings);
    });
    
    reset_strings();
    benchmark("Manual alignment with std::string.append()", [&]() {
        for (auto& e :strings) {
            e = get_rand_string();
            if (e.size() < pad_size) e.append(pad_size - e.size(), ' ');
        }
        DO_NOT_OPTIMIZE_AWAY(strings);
    });
    
    reset_strings();
    benchmark("std::ostringstream << std::setw() << std::left", [&]() {
        for (auto& e :strings) {
            std::ostringstream oss;
            oss << std::setw(pad_size) << std::left << get_rand_string();
            e = oss.str();
        }
        DO_NOT_OPTIMIZE_AWAY(strings);
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

    benchmark_stringification();
    //  benchmark_int_logging();

    // log::add_terminal_sink(std::cout, log::Verbosity::INFO);

    // UTL_LOG_INFO("Value is ", 5);
    // UTL_LOG_WARN("But it should be ", 4);
    // UTL_LOG_ERR("IT SHOULD BE ", 4);
    // UTL_LOG_INFO("Nonetheless we continue to look for a better value");
    // UTL_LOG_TRACE("Perhaps we could find something else");
    // UTL_LOG_TRACE("Like 2");
    // UTL_LOG_TRACE("2 would work great, right?");
    // UTL_LOG_TRACE("Perhaps we could even combine them if we find both");
    // UTL_LOG_TRACE("And get 42...");
    // UTL_LOG_TRACE("Nah, that would answer too many questions, can't have that");
    // UTL_LOG_INFO("Here, have a vector: ", std::vector{1, 2, 3});
    // UTL_LOG_INFO("Here, have a tuple: ", std::tuple{1, 'a', 3.14});
    // UTL_LOG_INFO("Here, have a super tuple: ", std::tuple{
    //                                                std::tuple{'k', 16},
    //                                                "text", std::vector{4, 5, 6}
    // });
    // UTL_LOG_INFO("Here, have a map: ", std::map{
    //                                        std::pair{"key_1", 1},
    //                                        std::pair{"key_2", 2}
    // });
}