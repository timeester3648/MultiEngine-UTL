#include "proto_utils.hpp" // double include to check that it doesn't break anything

#include <array>
#include <chrono>
#include <cstddef>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <ios>
#include <iostream>
#include <limits>
#include <list>
#include <numeric>
#include <random>
#include <set>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <vector>


UTL_DEFINE_ENUM_WITH_STRING_CONVERSION(Sides, LEFT, RIGHT, TOP, BOTTOM)
// like a regular enum must be declared outside of function

UTL_DEFINE_IS_FUNCTION_PRESENT(localtime_s, int, tm*, const time_t*)
UTL_DEFINE_IS_FUNCTION_PRESENT(localtime_r, tm*, const time_t*, tm*)

int main(int argc, char* argv[]) {
    using namespace utl;



    // ### utl::voidstream:: ##
    std::cout << "\n\n### utl::voidstream:: ###\n\n";

    std::cout << "std::cout will print:\n";
    std::cout << "<hello there!>\n\n";

    std::cout << "voidstream::vout will not:\n";
    voidstream::vout << "<hello there!>\n\n";



    // ### utl::table:: ###
    std::cout << "\n\n### utl::table:: ###\n\n";

    table::create({16, 16, 16, 16, 20});
    table::set_formats({table::NONE, table::DEFAULT(), table::FIXED(2), table::SCIENTIFIC(3), table::BOOL});

    table::hline();
    table::cell("Method", "Threads", "Speedup", "Error", "Err. within range");
    table::hline();
    table::cell("Gauss", 16, 11.845236, 1.96e-4, false);
    table::cell("Jacobi", 16, 15.512512, 1.37e-5, false);
    table::cell("Seidel", 16, 13.412321, 1.74e-6, true);
    table::cell("Relaxation", 16, 13.926783, 1.17e-6, true);
    table::hline();



    // ### utl::sleep:: ##
    std::cout << "\n\n### utl::sleep:: ###\n\n";

    constexpr int    repeats           = 6;
    constexpr double sleep_duration_ms = 16.67;

    std::cout << "Sleeping for " << sleep_duration_ms << " ms.\n";

    std::cout << "\nstd::this_thread::sleep_for():\n";
    for (int i = 0; i < repeats; ++i) {
        auto start = std::chrono::steady_clock::now();

        std::this_thread::sleep_for(std::chrono::nanoseconds(static_cast<int64_t>(sleep_duration_ms * 1e6)));

        auto end = std::chrono::steady_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6 << " ms\n";
    }

    std::cout << "\nsleep::spinlock():\n";
    for (int i = 0; i < repeats; ++i) {
        auto start = std::chrono::steady_clock::now();

        sleep::spinlock(sleep_duration_ms);

        auto end = std::chrono::steady_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6 << " ms\n";
    }

    std::cout << "\nsleep::hybrid():\n";
    for (int i = 0; i < repeats; ++i) {
        auto start = std::chrono::steady_clock::now();

        sleep::hybrid(sleep_duration_ms);

        auto end = std::chrono::steady_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6 << " ms\n";
    }

    std::cout << "\nsleep::system():\n";
    for (int i = 0; i < repeats; ++i) {
        auto start = std::chrono::steady_clock::now();

        sleep::system(sleep_duration_ms);

        auto end = std::chrono::steady_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6 << " ms\n";
    }



    // ### utl::random:: ##
    std::cout << "\n\n### utl::random:: ###\n\n";

    random::seed_with_time();
    std::cout << "rand_int(0, 100) = " << random::rand_int(0, 100) << "\n"
              << "rand_double() = " << random::rand_double() << "\n"
              << "rand_double(-5, 5) = " << random::rand_double(-5, 5) << "\n"
              << "rand_bool() = " << random::rand_bool() << "\n"
              << "rand_choise({1, 2, 3}) = " << random::rand_choise({1, 2, 3}) << "\n"
              << "rand_linear_combination(2., 3.) = " << random::rand_linear_combination(2., 3.) << "\n";

    // Using XorShift64* with <random>
    std::random_device               rd{};
    random::XorShift64StarGenerator  gen{rd()};
    std::normal_distribution<double> distr;

    std::cout << "Random value from N(0, 1) = " << distr(gen) << "\n";

    // Benchmark different random generators
    // constexpr int N = 900'000'000;
    // constexpr std::uint64_t seed = 15;

    // double sum = 0.;

    // // Profile generating N doubles with uniform [0, 1] distribution
    // UTL_PROFILER_LABELED("std::rand()") {
    //     srand(seed);
    // 	for (int i = 0; i < N; ++i) sum += std::rand() / (static_cast<double>(RAND_MAX) + 1.);
    // }

    // UTL_PROFILER_LABELED("std::minstd_rand") {
    //     std::minstd_rand gen{seed};
    //     std::uniform_real_distribution dist{0., 1.};
    //     for (int i = 0; i < N; ++i) sum += dist(gen);
    // }

    // UTL_PROFILER_LABELED("std::mt19937") {
    //     std::mt19937 gen{seed};
    //     std::uniform_real_distribution dist{0., 1.};
    //     for (int i = 0; i < N; ++i) sum += dist(gen);
    // }

    // UTL_PROFILER_LABELED("random::XorShift64StarGenerator") {
    //     utl::random::XorShift64StarGenerator gen{seed};
    //     std::uniform_real_distribution dist{0., 1.};
    //     for (int i = 0; i < N; ++i) sum += dist(gen);
    // }

    // UTL_PROFILER_LABELED("random::rand_double()") {
    //     random::xorshift64star.seed(seed);
    //     for (int i = 0; i < N; ++i) sum += random::rand_double();
    // }

    // // Prevent compiler from optimizing away "unused" sum
    // std::cout << sum << "\n";


    // ### utl::math:: ###
    std::cout << "\n\n### utl::math:: ###\n\n";

    // Type traits
    std::cout << std::boolalpha << "are doubles addable?    -> " << math::is_addable_with_itself<double>::value << "\n"
              << "are std::pairs addable? -> " << math::is_addable_with_itself<std::pair<int, int>>::value << "\n";

    std::cout << "\n";

    // Using math functions
    std::cout << "All methods below are constexpr and type agnostic:\n"
              << "abs(-4) = " << math::abs(-4) << "\n"
              << "sign(-4) = " << math::sign(-4) << "\n"
              << "sqr(-4) = " << math::sqr(-4) << "\n"
              << "cube(-4) = " << math::cube(-4) << "\n"
              << "deg_to_rad(180.) = " << math::deg_to_rad(180.) << "\n"
              << "rad_to_deg(PI) = " << math::rad_to_deg(math::PI) << "\n"
              << "midpoint(4., 5.) = " << math::midpoint(4., 5.) << "\n"
              << "\n"
              << "linspace(0., 1., math::Points(3) = " << stre::to_str(math::linspace(0., 1., math::Points(3))) << "\n"
              << "integrate_trapezoidal(|x|, -1., 1., math::Intervals(20) = "
              << math::integrate_trapezoidal([](double x) { return std::abs(x); }, -1., 1., math::Intervals(20)) << "\n"
              << "\n"
              << "uint_difference(5u, 17u) = " << math::uint_difference(5u, 17u) << "\n"
              << "\n"
              << "ternary_branchless(true, 3.12, -4.17) = " << math::ternary_branchless(true, 3.12, -4.17) << "\n"
              << "ternary_bitselect(true, 15, -5) = " << math::ternary_bitselect(true, 15, -5) << "\n"
              << "ternary_bitselect(false, 15) = " << math::ternary_bitselect(false, 15) << "\n";

    // ### utl::shell:: ###
    std::cout << "\n\n### utl::shell:: ###\n\n";

    // Create temp file
    const auto temp_file_path = shell::generate_temp_file();
    const auto temp_file_text = "~~~FILE CONTENTS~~~";
    std::ofstream(temp_file_path, std::ios_base::app) << temp_file_text;

    std::cout << "Temp. file path: " << temp_file_path << "\n";
    std::cout << "Temp. file text: " << temp_file_text << "\n";

    // Run command to show file contents (bash command in this example)
    const auto command        = "cat " + temp_file_path;
    const auto command_result = shell::run_command(command);

    std::cout << "shell::run_command(" << command << "):\n"
              << "command_result.status = " << command_result.status << "\n"
              << "command_result.stdout_output = " << command_result.stdout_output << "\n"
              << "command_result.stderr_output = " << command_result.stderr_output << "\n";

    // We can clear files manually, but otherwise they will be automatically cleared upon exit
    // ('exit' is triggered by calling std::exit() or returning from main())
    // shell::clear_temp_files();

    std::cout << "Exe path:\n" << shell::get_exe_path(argv) << "\n\n";

    std::cout << "Command line arguments (if present):\n";
    for (const auto& arg : shell::get_command_line_args(argc, argv)) std::cout << arg << "\n";

    // ### utl::stre:: ###
    std::cout << "\n\n### utl::stre:: ###\n\n";

    // Type traits
    std::cout << std::boolalpha << "is_printable< int >                    = " << stre::is_printable<int>::value << "\n"
              << "is_iterable_through< std::list<int> >  = " << stre::is_iterable_through<std::list<int>>::value << "\n"
              << "is_tuple_like< std::string >           = " << stre::is_tuple_like<std::string>::value << "\n"
              << "is_string< const char* >               = " << stre::is_string<const char*>::value << "\n"
              << "is_to_str_convertible< std::set<int> > = " << stre::is_to_str_convertible<std::set<int>>::value
              << "\n"
              << "\n";

    // Declare objects
    std::tuple<int, double, std::string> tup         = {2, 3.14, "text"};
    std::vector<std::vector<int>>        vec_of_vecs = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    std::unordered_map<std::string, int> map = {
        {"key_1", 1},
        {"key_2", 2}
    };
    std::tuple<std::vector<bool>, std::vector<std::string>, std::vector<std::pair<int, int>>> tup_of_vecs = {
        {true, false, true},
        {"text_1", "text_2"},
        {{4, 5}, {7, 8}}
    };

    // Print objects
    std::cout << "to_str(tuple):\n"
              << stre::to_str(tup) << "\n\n"
              << "to_str(vector of vectors):\n"
              << stre::to_str(vec_of_vecs) << "\n\n"
              << "to_str(unordered_map):\n"
              << stre::to_str(map) << "\n\n"
              << "to_str(tuple of vectors with bools, strings and pairs):\n"
              << stre::to_str(tup_of_vecs) << "\n"
              << "\n";

    // Inline stringstream
    const std::string str = stre::InlineStream() << "Value " << 3.14 << " is smaller than " << 6.28;
    std::cout << str << "\n\n";

    // Other utilities
    std::cout << "repeat_symbol('h',  7) = " << stre::repeat_symbol('h', 7) << "\n"
              << "repeat_string(\"xo\", 5) = " << stre::repeat_string("xo-", 5) << "\n"
              << "pad_with_zeroes(15) = " << stre::pad_with_zeroes(15) << "\n";

    // ### utl::timer:: ###
    std::cout << "\n\n### utl::timer:: ###\n\n";

    std::cout << "Current time is: " << timer::datetime_string() << "\n\n";

    timer::start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Time elapsed during sleep_for(100 ms):\n"
              << timer::elapsed_string_ms() << "\n"
              << timer::elapsed_string_sec() << "\n"
              << timer::elapsed_string_min() << "\n"
              << timer::elapsed_string_hours() << "\n"
              << timer::elapsed_string_fullform() << "\n";


    // ### utl macros ###
    std::cout << "\n\n### utl macros ###\n\n";

    UTL_LOG_SET_OUTPUT(std::cerr);

    // LOG compiles always
    UTL_LOG("Block ", 17, ": responce in order. Proceeding with code ", 0, ".");

    // DLOG compiles only in Debug
    UTL_LOG_DEBUG("Texture with ID ", 15037, " seems to be corrupted.");

    // NOTE: stre::to_str() can be used to pass complex objects to logger

    std::cout << "Current platform: " << UTL_DEFINE_CURRENT_OS_STRING << "\n"
              << "Compilation mode: " << (UTL_DEFINE_IS_DEBUG ? "Debug" : "Release") << "\n";

    // Loop than repeats N times
    UTL_DEFINE_REPEAT(5) { std::cout << "Ho\n"; }

// Variadic macro that outputs size of __VA_ARGS__
#define VARIADIC_MACRO(...) UTL_DEFINE_VA_ARGS_COUNT(__VA_ARGS__)

    constexpr int args = VARIADIC_MACRO(1, 2., "3", A, B, (std::pair<int, int>{4, 5}), "12,3");

    std::cout << "\nSize of __VA_ARGS__: " << args << "\n";

    // Enum with string conversion
    std::cout << "(enum -> string) conversion:\n"
              << Sides::BOTTOM << " -> " << Sides::to_string(Sides::BOTTOM) << "\n"
              << "(string -> enum) conversion:\n"
              << "BOTTOM"
              << " -> " << Sides::from_string("BOTTOM") << "\n";

    // Check if function exists
    constexpr bool exists_localtime_s = is_function_present_localtime_s::value;
    constexpr bool exists_localtime_r = is_function_present_localtime_r::value;

    std::cout << "Windows 'localtime_s()' present: " << exists_localtime_s << "\n"
              << "Linux   'localtime_r()' present: " << exists_localtime_r << "\n";

    // Do some specific logic based on existing function
    if constexpr (exists_localtime_s) { std::cout << "\n~ Some localtime_s()-specific logic ~"; }
    if constexpr (exists_localtime_r) { std::cout << "\n~ Some localtime_r()-specific logic ~"; }

    // ### utl::progressbar:: ###
    std::cout << "\n\n### utl::progressbar:: ###\n\n";

    using ms = std::chrono::milliseconds;

    constexpr ms time(100);
    constexpr ms tau(10);

    // Create progress bar with style '[#####...] xx.xx%' and width 50 that updates every 0.05%
    auto bar = progressbar::Percentage('#', '.', 50, 0.05 * 1e-2, true);

    std::cout << "\n- progressbar::Percentage -";

    bar.start();
    for (ms t(0); t <= time; t += tau) {
        std::this_thread::sleep_for(tau); // simulate some work
        const double percentage = double(t.count()) / time.count();

        bar.set_progress(percentage);
    }
    bar.finish();

    std::cout << "\n- progressbar::Ruler -";

    // Create a primitive progress bar with ruler-like style
    auto ruler = progressbar::Ruler('#');

    ruler.start();
    for (ms t(0); t <= time; t += tau) {
        std::this_thread::sleep_for(tau); // simulate some work
        const double percentage = double(t.count()) / time.count();

        ruler.set_progress(percentage);
    }
    ruler.finish();

    // ### utl::config:: ###
    std::cout << "\n\n### utl::config:: ###\n\n";

    std::cout << "Saving config json...\n";

    std::filesystem::create_directory("temp");

    config::export_json(
        "temp/cfg.json",
        config::entry("date", std::string("2024.04.02")),
        config::entry("time_steps", 500),
        config::entry("time_period", 1.24709e+2),
        config::entry("auxiliary_info", true),
        config::entry("scaling_functions", { "identity", "log10" }), 
        config::entry("options", {17, 45}),
        config::entry("coefs", {0.125, 0.3}),
        config::entry("flags", {true, true, false}),
        config::entry("matrix", {{0.7, -1.3}, {3.1}}), // 2D, 3D, 4D arrays are also fine
        config::entry("bad numbers",
                      {std::numeric_limits<double>::infinity(), std::numeric_limits<double>::denorm_min(),
                       std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::signaling_NaN()}));
    
    // ### utl::mvl:: ###
    std::cout << "\n\n### utl::mvl:: ###\n\n";
                
    mvl::SparseMatrix<double> mat(3, 4,
    {
        {0, 0, 3.14    },
        {0, 1, 4.24    },
        {1, 1, 7.15    },
        {2, 2, 2.38    },
        {2, 3, 734.835 }
    });
    
    std::cout
        // Human-readable formats
        << "\n## as_vector() ##\n\n"     << mvl::format::as_vector(    mat)
        << "\n## as_matrix() ##\n\n"     << mvl::format::as_matrix(    mat)
        << "\n## as_dictionary() ##\n\n" << mvl::format::as_dictionary(mat)
        // Export formats
        << "\n## as_json_array() ##\n\n" << mvl::format::as_json_array(mat)
        << "\n## as_raw_text() ##\n\n"   << mvl::format::as_raw_text(  mat);

    // ### MACRO_PROFILER ###
    std::cout << "\n\n### UTL_PROFILER ###\n\n";

    UTL_PROFILER { std::this_thread::sleep_for(std::chrono::milliseconds(650)); }

    // Profilers can be nested, this is fine
    UTL_PROFILER_LABELED("outer loop")
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        UTL_PROFILER_LABELED("inner loop")
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    // we expect to see that 'inner loop' will measure about half the time of an 'outer loop'

    UTL_PROFILER_REROUTE_TO_FILE("temp/profiling.txt");

    return 0;
}
