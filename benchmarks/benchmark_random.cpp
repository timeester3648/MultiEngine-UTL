// __________ BENCHMARK FRAMEWORK & LIBRARY  __________

#include "benchmark.hpp"
#include <cstdint>
#include <fstream>
#include <random>
#include <vector>

// _____________ BENCHMARK IMPLEMENTATION _____________

// =======================
// --- PRNG Benchmarks ---
// =======================

constexpr std::uint32_t rand_seed = 15;
constexpr std::size_t   data_size = 5'000'000;

template <class Generator>
void benchmark_prng(const char* name) {
    Generator                      gen{rand_seed};
    std::uniform_real_distribution dist{0., 1.};

    std::vector<double> data(data_size);

    benchmark(name, [&] {
        for (auto& e : data) e = dist(gen);
    });
}

void benchmark_prngs() {
    using namespace utl;

    log::println("\n\n====== BENCHMARKING: PRNG uniform double ditribution ======\n");
    log::println("N                 -> ", data_size);
    log::println("Data memory usage -> ", math::memory_size<double>(data_size), " MiB");

    bench.title("PRNG uniform double ditribution")
        .timeUnit(millisecond, "ms")
        .minEpochIterations(5)
        .warmup(10)
        .relative(true);

    // 'std::' prngs
    // benchmark_prng<std::minstd_rand0>("std::minstd_rand0"); // same results as minstd_rand
    benchmark_prng<std::minstd_rand>("std::minstd_rand");
    benchmark_prng<std::mt19937>("std::mt19937");
    benchmark_prng<std::mt19937_64>("std::mt19937_64");
    // benchmark_prng<std::ranlux24_base>("ranlux24_base");
    // benchmark_prng<std::ranlux48_base>("ranlux48_base");
    // benchmark_prng<std::ranlux24>("ranlux24"); // too slow to even measure, about ~5% performance of minstd_rand
    // benchmark_prng<std::ranlux48>("ranlux48"); // too slow to even measure, about ~3% performance of minstd_rand
    benchmark_prng<std::knuth_b>("knuth_b");

    // 'utl::' prngs

    benchmark_prng<random::generators::RomuTrio32>("RomuTrio32");
    benchmark_prng<random::generators::JSF32>("JSF32");
    benchmark_prng<random::generators::RomuDuoJr>("RomuDuoJr");
    benchmark_prng<random::generators::JSF64>("JSF64");
    benchmark_prng<random::generators::Xoshiro256PlusPlus>("Xoshiro256++");
    benchmark_prng<random::generators::Xorshift64Star>("Xorshift64*");
}

int main() {


    benchmark_prngs();

    return 0;
}