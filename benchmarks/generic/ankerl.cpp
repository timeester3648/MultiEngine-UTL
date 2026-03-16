#include "benchmarks/common.hpp"

// _______________________ INCLUDES _______________________

// UTL dependencies
#include "include/UTL/random.hpp" // random::uniform_uint()

// Libraries to benchmarks against
#include "benchmarks/thirdparty/ankerl/unordered_dense.h" // ankerl::unordered_dense::set<>

// Standard headers
#include <cstdint> // uint64_t
#include <vector>  // vector<>

// ____________________ IMPLEMENTATION ____________________

// What are we trying to measure:
//    - What kind of slowdown can we expect from dense set construction relative to a vector
//    - Whether dense set construction can benefit from replacing the internal container at once

int main() {
    constexpr std::size_t N = 2'000;

    using value_type = std::uint64_t;

    std::vector<value_type> data(N);
    for (auto& e : data) e = utl::random::uniform_uint(0, 500'000);

    std::vector<value_type> source;

    bench.title("Cloning from vector").minEpochTime(25ms).warmup(20).relative(true);

    source = data;
    benchmark("std::vector -> copy -> std::vector", [&] {
        std::vector<value_type> target = source;

        DO_NOT_OPTIMIZE_AWAY(target);
    });

    source = data;
    benchmark("std::vector -> move -> std::vector", [&] {
        std::vector<value_type> target = std::move(source);
        DO_NOT_OPTIMIZE_AWAY(target);
    });

    source = data;
    benchmark("std::vector -> bulk copy-insert -> ankerl::unordered_dense::set", [&] {
        ankerl::unordered_dense::set<value_type> target;

        target.insert(source.begin(), source.end());

        DO_NOT_OPTIMIZE_AWAY(target);
    });

    source = data;
    benchmark("std::vector -> bulk move-insert -> ankerl::unordered_dense::set", [&] {
        ankerl::unordered_dense::set<value_type> target;

        target.insert(std::make_move_iterator(source.begin()), std::make_move_iterator(source.end()));

        DO_NOT_OPTIMIZE_AWAY(target);
    });

    source = data;
    benchmark("std::vector -> copy-replace -> ankerl::unordered_dense::set", [&] {
        ankerl::unordered_dense::set<value_type> target;

        std::vector<value_type> copy = source;

        target.replace(std::move(copy));

        DO_NOT_OPTIMIZE_AWAY(target);
    });

    source = data;
    benchmark("std::vector -> move-replace -> ankerl::unordered_dense::set", [&] {
        ankerl::unordered_dense::set<value_type> target;

        target.replace(std::move(source));

        DO_NOT_OPTIMIZE_AWAY(target);
    });
}