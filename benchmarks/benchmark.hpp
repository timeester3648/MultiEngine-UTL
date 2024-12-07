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
#include <utility>

#define ANKERL_NANOBENCH_IMPLEMENT
#include "thirdparty/nanobench.h"

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