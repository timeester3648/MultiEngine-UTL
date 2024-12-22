// __________ BENCHMARK FRAMEWORK & LIBRARY  __________

#include "benchmark.hpp"
#include <cstdint>
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
    benchmark_prng<random::generators::RomuDuoJr>("RomuDuoJr");
    benchmark_prng<random::generators::JSF32>("JSF32");
    benchmark_prng<random::generators::JSF64>("JSF64");
    benchmark_prng<random::generators::Xoshiro256PlusPlus>("Xoshiro256++");
    benchmark_prng<random::generators::Xorshift64Star>("Xorshift64*");
}

int main() {

    benchmark_prngs();

    // // Grid
    // std::vector<double> matrix_data(N * N);


    // UTL_PROFILER("std::rand()") {
    //     srand(rand_seed);
    //     for (auto& e : matrix_data) e = std::rand() / (static_cast<double>(RAND_MAX) + 1.);
    // }

    // UTL_PROFILER("std::mt19937 + std::uniform_real_distribution") {
    //     std::mt19937                   gen{rand_seed};
    //     std::uniform_real_distribution dist{0., 1.};
    //     for (auto& e : matrix_data) e = dist(gen);
    // }

    // UTL_PROFILER("std::minstd_rand0 + std::uniform_real_distribution") {
    //     std::minstd_rand0              gen{rand_seed};
    //     std::uniform_real_distribution dist{0., 1.};
    //     for (auto& e : matrix_data) e = dist(gen);
    // }

    // UTL_PROFILER("xorshift64star + rand_double()") {
    //     rng::xorshift64star.seed(rand_seed);
    //     for (auto& e : matrix_data) e = rng::rand_double();
    // }

    // constexpr double min = -10.;
    // constexpr double max = 10.;

    // UTL_PROFILER("xorshift64star + std::uniform_real_distribution") {
    //     rng::xorshift64star.seed(rand_seed);
    //     std::uniform_real_distribution dist{min, max};
    //     for (auto& e : matrix_data) e = dist(rng::xorshift64star);
    // }

    // UTL_PROFILER("xorshift64star + rand_double() & formula (in loop)") {
    //     rng::xorshift64star.seed(rand_seed);
    //     for (auto& e : matrix_data) { e = min + (max - min) * rng::rand_double(); }
    // }

    // UTL_PROFILER("xorshift64star + std::uniform_real_distribution (in loop)") {
    //     rng::xorshift64star.seed(rand_seed);
    //     for (auto& e : matrix_data) {
    //         std::uniform_real_distribution dist{0., 1.};
    //         e = dist(rng::xorshift64star);
    //     }
    // }

    // // Use  matrix data so compiler doesn't decide to optimize away previous loops
    // const auto mean   = std::reduce(matrix_data.begin(), matrix_data.end()) / matrix_data.size();
    // const auto sq_sum = std::inner_product(matrix_data.begin(), matrix_data.end(), matrix_data.begin(), 0.0);
    // const auto stdev  = std::sqrt(sq_sum / matrix_data.size() - mean * mean);

    // std::cout << "mean = " << mean << "\n"
    //           << "stdev = " << stdev << "\n"
    //           << "min = " << *std::min_element(matrix_data.begin(), matrix_data.end()) << "\n"
    //           << "max = " << *std::max_element(matrix_data.begin(), matrix_data.end()) << "\n";
    
    return 0;
}