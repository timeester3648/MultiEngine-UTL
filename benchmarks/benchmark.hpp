#include "MACRO_DEFINE.hpp"
#include "module_json.hpp"
#include "module_math.hpp"
#include "module_mvl.hpp"
#include "module_profiler.hpp"
#include "module_random.hpp"
#include "module_shell.hpp"
#include "module_sleep.hpp"
#include <chrono>

#define ANKERL_NANOBENCH_IMPLEMENT
#include "thirdparty/nanobench.h"

#define REPEAT(repeats) UTL_DEFINE_REPEAT(repeats)
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