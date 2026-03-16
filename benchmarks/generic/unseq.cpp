#include "benchmarks/common.hpp"

// _______________________ INCLUDES _______________________

// UTL dependencies
#include "include/UTL/random.hpp" // random::uniform_uint()

// Libraries to benchmarks against
// None

// Standard headers
#include <algorithm> // for_each()
#include <array>     // array<>
#include <execution> // execution::seq, execution::unseq
#include <vector>    // vector<>

// ____________________ IMPLEMENTATION ____________________

// What are we trying to measure:
//    - What kind of slowdown can we expect from dense set construction relative to a vector
//    - Whether dense set construction can benefit from replacing the internal container at once

int main() {
    constexpr std::size_t N = 400'000;

    std::vector<double> data(N);
    for (auto& e : data) e = utl::random::normal_double();

    bench.title("Computing a floating point sum").minEpochTime(25ms).warmup(200).relative(true);

    benchmark("Range-based for", [&] {
        double sum = 0;
        for (const auto& e : data) sum += e;
        DO_NOT_OPTIMIZE_AWAY(sum);
    });

#ifdef  __cpp_lib_execution // because libc++ is extremely behind on C++17 support

    benchmark("std::for_each(std::execution::seq)", [&] {
        double sum = 0;
        std::for_each(std::execution::seq, data.begin(), data.end(), [&](double e) { sum += e; });
        DO_NOT_OPTIMIZE_AWAY(sum);
    });

    // Unsequenced execution means that we don't care about floating-point operation
    // happening in order, the compiler is allowed vectorize this loop
    // benchmark("std::for_each(std::execution::unseq)", [&] {
    //     double sum = 0;
    //     std::for_each(std::execution::unseq, data.begin(), data.end(), [&](double e) { sum += e; });
    //     DO_NOT_OPTIMIZE_AWAY(sum);
    // });
    
#endif

    // Explicitly 4-unroll the loop for vectorization
    benchmark("Manual 4-unrolled loop", [&] {
        std::array<double, 4> sums = { 0, 0, 0, 0 };
        std::size_t           i = 0;
        for (; i + 4 < data.size(); i += 4) {
            sums[0] += data[i + 0];
            sums[1] += data[i + 1];
            sums[2] += data[i + 2];
            sums[3] += data[i + 3];
        }
        for (; i < data.size(); ++i) sums[0] += data[i]; // non-divisible tail
        const double sum = sums[0] + sums[1] + sums[2] + sums[3];
        DO_NOT_OPTIMIZE_AWAY(sum);
    });
}