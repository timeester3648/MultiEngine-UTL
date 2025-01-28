#include "module_json.hpp"
#include "module_log.hpp"
#include "module_math.hpp"
#include "module_mvl.hpp"
#include "module_parallel.hpp"
#include "module_predef.hpp"
#include "module_profiler.hpp"
#include "module_progressbar.hpp"
#include "module_random.hpp"
#include "module_shell.hpp"
#include "module_sleep.hpp"
#include "module_stre.hpp"
#include "module_table.hpp"
#include "module_timer.hpp"

#include <chrono>
#include <complex>
#include <utility>
#include <vector>

#define ANKERL_NANOBENCH_IMPLEMENT
#include "thirdparty/nanobench.h"

// Note:
// This is a common include for all benchmarks,
// it exists purely to reduce boilerplate and shouldn't be included anywhere else.

#define REPEAT(repeats_) for (int count_ = 0; count_ < repeats_; ++count_)
#define DO_NOT_OPTIMIZE_AWAY ankerl::nanobench::doNotOptimizeAway

namespace nb = ankerl::nanobench;

using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::seconds;

constexpr auto nanosecond  = nanoseconds(1);
constexpr auto microsecond = microseconds(1);
constexpr auto millisecond = milliseconds(1);
constexpr auto second      = seconds(1);

inline nb::Bench bench;

template <class Func>
void benchmark(const char* name, Func lambda) {
    bench.run(name, lambda);
}

using namespace utl;

// Set up fast random-stuff generation so we can benchmark on unpredictable workloads
// that can't be "folded" with optimization. The reason we don't just use 'utl::random'
// is because here we can select the fastest PRNG and do some stuff to generate strings faster.

namespace datagen {

// Generation params
constexpr std::size_t string_pregen_count = 500;

constexpr std::size_t min_string_size = 10;
constexpr std::size_t max_string_size = 120;

constexpr double min_double = -1e3;
constexpr double max_double = 1e3;

constexpr int min_int = -1500;
constexpr int max_int = 1500;

// PRNG & buffers
inline random::generators::RomuTrio32 gen;
inline std::vector<std::string>       pregen_strings;
// we want random generation to be as fast as possible to reduce its impact on benchmarks that measure
// overhead of calling something else, which is why we choose the fastest PRNG available and pregenerate
// strings. Number generation is fast enough that we don't really get much benefit from pregeneration.

// Datagen functions
inline auto rand_bool() { return static_cast<bool>(std::uniform_int_distribution{0, 1}(gen)); }
inline auto rand_char() { return static_cast<char>(std::uniform_int_distribution{'a', 'z'}(gen)); }
inline auto rand_int() { return std::uniform_int_distribution{min_int, max_int}(gen); }
inline auto rand_double() { return std::uniform_real_distribution{min_double, max_double}(gen); }
inline auto rand_complex() { return std::complex{rand_double(), rand_double()}; }
inline auto rand_string() {
    return pregen_strings[std::uniform_int_distribution<std::size_t>{0, string_pregen_count - 1}(gen)];
}
inline auto rand_vector_of_strings() { return std::vector{rand_string(), rand_string(), rand_string(), rand_string()}; }

// Run some init logic before 'main()' to set up pregenerated values
struct _init_helper {
    _init_helper() {
        pregen_strings.resize(string_pregen_count);
        for (auto& e : pregen_strings)
            e = shell::random_ascii_string(random::rand_uint(min_string_size, max_string_size));
    }
};
inline _init_helper _init;

} // namespace datagen