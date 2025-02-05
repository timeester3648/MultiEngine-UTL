// __________ BENCHMARK FRAMEWORK & LIBRARY  __________

#include "benchmark.hpp"
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <random>
#include <string_view>
#include <type_traits>
#include <unordered_map>
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

    std::vector<typename Generator::result_type> data(data_size);

    benchmark(name, [&] {
        for (auto& e : data) e = gen();
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

    // 'std::' PRNGs
    // benchmark_prng<std::minstd_rand0>("std::minstd_rand0"); // same results as minstd_rand
    benchmark_prng<std::minstd_rand>("std::minstd_rand");
    benchmark_prng<std::mt19937>("std::mt19937");
    benchmark_prng<std::mt19937_64>("std::mt19937_64");
    // benchmark_prng<std::ranlux24_base>("ranlux24_base");
    // benchmark_prng<std::ranlux48_base>("ranlux48_base");
    // benchmark_prng<std::ranlux24>("ranlux24"); // too slow to even measure, about ~5% performance of minstd_rand
    benchmark_prng<std::ranlux48>("ranlux48"); // too slow to even measure, about ~3% performance of minstd_rand
    benchmark_prng<std::knuth_b>("knuth_b");

    // 'utl::' PRNGs
    benchmark_prng<random::generators::RomuTrio32>("RomuTrio32");
    benchmark_prng<random::generators::JSF32>("JSF32");
    benchmark_prng<random::generators::RomuDuoJr>("RomuDuoJr");
    benchmark_prng<random::generators::JSF64>("JSF64");
    benchmark_prng<random::generators::Xoshiro256PlusPlus>("Xoshiro256++");
    benchmark_prng<random::generators::Xorshift64Star>("Xorshift64*");
    benchmark_prng<random::generators::ChaCha20>("ChaCha20");
}

// ===============================
// --- Distribution benchmarks ---
// ===============================

// clang-format off
template<class T> struct wider { static_assert(math::_always_false_v<T>, "Missing specialization"); };

template<> struct wider<std::uint8_t > { using type = std::uint16_t; };
template<> struct wider<std::uint16_t> { using type = std::uint32_t; };
template<> struct wider<std::uint32_t> { using type = std::uint64_t; };
template<> struct wider<std::uint64_t> { using type = __uint128_t  ; }; // compiler-specific, BAD (!)
template<> struct wider<std::int8_t  > { using type =  std::int16_t; };
template<> struct wider<std::int16_t > { using type =  std::int32_t; };
template<> struct wider<std::int32_t > { using type =  std::int64_t; };
template<> struct wider<std::int64_t > { using type =  __int128_t  ; }; // compiler-specific, BAD (!)

template<class T> using wider_t = typename wider<T>::type;

template<class T> constexpr auto type_name = "unknown";

template<> constexpr auto type_name<std::uint8_t > = "uint8"  ;
template<> constexpr auto type_name<std::uint16_t> = "uint16" ;
template<> constexpr auto type_name<std::uint32_t> = "uint32" ;
template<> constexpr auto type_name<std::uint64_t> = "uint64" ;
template<> constexpr auto type_name<__uint128_t  > = "uint128";
template<> constexpr auto type_name<std::int8_t  > =  "int8"  ;
template<> constexpr auto type_name<std::int16_t > =  "int16" ;
template<> constexpr auto type_name<std::int32_t > =  "int32" ;
template<> constexpr auto type_name<std::int64_t > =  "int64" ;
template<> constexpr auto type_name<__int128_t   > =  "int128";

template<> constexpr auto type_name<float      > =  "float"      ;
template<> constexpr auto type_name<double     > =  "double"     ;
template<> constexpr auto type_name<long double> =  "long double";
// clang-format on

// --- Distributions ---
// ---------------------

template <class T>
struct uint_dist_lemier {
    using narrow = T;
    using wide   = wider_t<T>;

    narrow range;

    constexpr uint_dist_lemier(T range) noexcept : range(range) {}

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept {
        wide   product = wide(gen()) * wide(range);
        narrow low     = narrow(product);
        if (low < range) {
            const narrow threshold = -range % range;
            while (low < threshold) {
                product = wide(gen()) * wide(range); // Note: Caching this value as a member makes no difference
                low     = narrow(product);
            }
        }
        return product >> std::numeric_limits<narrow>::digits;
    }
};

template <class T>
struct uint_dist_lemier_cached_threshold {
    using narrow = T;
    using wide   = wider_t<T>;

    narrow range;
    narrow threshold;

    constexpr uint_dist_lemier_cached_threshold(narrow range) noexcept : range(range), threshold(-range % range) {}

    template <class Gen>
    constexpr narrow operator()(Gen& gen) const noexcept {
        wide   product = wide(gen()) * wide(range);
        narrow low     = narrow(product);
        if (low < range) {
            while (low < threshold) {
                product = wide(gen()) * wide(range);
                low     = narrow(product);
            }
        }
        return product >> std::numeric_limits<narrow>::digits;
    }
};

template <class T>
struct uint_dist_lemier_tweak {
    using narrow = T;
    using wide   = wider_t<T>;

    narrow range;

    constexpr uint_dist_lemier_tweak(narrow range) noexcept : range(range) {}

    template <class Gen>
    constexpr narrow operator()(Gen& gen) const noexcept {
        wide   product = wide(gen()) * wide(range);
        narrow low     = narrow(product);
        if (low < range) {
            narrow threshold = -range;

            if (threshold >= range && (threshold -= range) >= range) threshold %= range;

            while (low < threshold) {
                product = wide(gen()) * wide(range); // Note: Caching this value as a member makes no difference
                low     = narrow(product);
            }
        }
        return product >> std::numeric_limits<narrow>::digits;
    }
};

template <class T>
struct uint_dist_mod_1x {
    T range;

    constexpr uint_dist_mod_1x(T range) noexcept : range(range) {}

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept {
        T x{}, r{};
        do {
            x = gen();
            r = x % range;
        } while (x - r > T(-range));
        return r;
    }
};

template <class T>
struct uint_dist_mod_1x_tweak {
    T range;

    constexpr uint_dist_mod_1x_tweak(T range) noexcept : range(range) {}

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept {
        T x{}, r{};
        do {
            r = x = gen();
            if (r >= range && (r -= range) >= range) r %= range;

        } while (x - r > T(-range));
        return r;
    }
};

template <class T>
struct float_dist_std_canonical {
    T min, max;

    constexpr float_dist_std_canonical(T min, T max) noexcept : min(min), max(max) {}

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept {
        return min + std::generate_canonical<T, std::numeric_limits<T>::digits>(gen) * (max - min);
    }
};

template <class T>
struct float_dist_mult {
    T min, max;

    constexpr float_dist_mult(T min, T max) noexcept : min(min), max(max) {}

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept {
        const T canonical = (gen() >> 11) * 0x1.0p-53;
        return min + canonical * (max - min);
    }
}; // this is only 64-bit PRNG & 64-bit float implementation

// MARK:
// --- Uniform uint distribution ---
// ---------------------------------
template <class T, class Gen>
constexpr T _uniform_uint_lemier(Gen& gen, T range) noexcept(noexcept(gen())) {
    using W = wider_t<T>;

    W product = W(gen()) * W(range);
    T low     = T(product);
    if (low < range) {
        while (low < -range % range) {
            product = W(gen()) * W(range);
            low     = T(product);
        }
    }
    return product >> std::numeric_limits<T>::digits;
}

template <class T, class Gen>
constexpr T _uniform_uint_modx1(Gen& gen, T range) noexcept(noexcept(gen())) {
    T x{}, r{};
    do {
        x = gen();
        r = x % range;
    } while (x - r > T(-range));
    return r;
}

template <class T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, bool> = true>
struct UniformUintDistribution {
    using result_type = T;

    struct param_type {
        result_type min = 0;
        result_type max = std::numeric_limits<result_type>::max();
    } pars{};

    constexpr UniformUintDistribution() = default;
    constexpr UniformUintDistribution(T min, T max) noexcept : pars({min, max}) { assert(min < max); }
    constexpr UniformUintDistribution(const param_type& p) noexcept : pars(p) { assert(p.min < p.max); }

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept(noexcept(gen())) {
        const result_type range = this->pars.max - this->pars.min;
        // Use lemier if possible, fallback to a generic algo that doesn't require a wider type otherwise
        if constexpr (sizeof(T) >= sizeof(std::uint64_t)) return this->pars.min + _uniform_uint_modx1<T>(gen, range);
        else return this->pars.min + _uniform_uint_lemier<std::uint32_t>(gen, range);
    }
    // Note 1:
    // Generating 32-bit int and casting to a smaller type is usually faster than generating it "properly"

    // Note 2:
    // 64-bit lemier is faster, but require '__uint128' support which is a GCC extension and is awkward to check for

    constexpr result_type reset() const noexcept {} // there is nothing to reset, provided for std-API compatibility
    constexpr param_type  params() const noexcept { return this->pars; }
    constexpr void        params(const param_type& p) noexcept { this->pars = p; }
    constexpr result_type a() const noexcept { return this->pars.min; }
    constexpr result_type b() const noexcept { return this->pars.max; }
    constexpr result_type min() const noexcept { return this->pars.min; }
    constexpr result_type max() const noexcept { return this->pars.max; }
};

// --- Float uniform distribution ---
// ----------------------------------

static_assert(sizeof(std::uint32_t) == 4, "Platform not supported.");
static_assert(sizeof(std::uint64_t) == 8, "Platform not supported.");
static_assert(sizeof(std::int32_t) == 4, "Platform not supported.");
static_assert(sizeof(std::int64_t) == 8, "Platform not supported.");
static_assert(sizeof(float) == 4, "Platform not supported.");
static_assert(sizeof(double) == 8, "Platform not supported.");

template <class T, class Gen>
constexpr T generate_canonical(Gen& gen) noexcept(noexcept(gen())) {
    using gen_result_type = typename Gen::result_type;

    constexpr int exponent_bits_64 = 11;
    constexpr int exponent_bits_32 = 8;

    constexpr double mantissa_hex_64 = 0x1.0p-53;  // == 2^-53, corresponds to 53 significant bits of double
    constexpr float  mantissa_hex_32 = 0x1.0p-24f; // == 2^-24, corresponds to 24 significant bits of float

    constexpr double pow2_minus_64 = 0x1.0p-64; // == 2^-64
    constexpr double pow2_minus_32 = 0x1.0p-32; // == 2^-32

    // Note 1: Note hexadecimal float literals, 'p' separates hex-base from the exponent
    // Note 2: Floats have 'mantissa_size + 1' significant bits due to having a sign bit

    // 64-bit float, 64-bit PRNG
    // => multiplication algorithm, see [https://prng.di.unimi.it/]
    if constexpr (sizeof(T) == 8 && sizeof(gen_result_type) == 8) return (gen() >> exponent_bits_64) * mantissa_hex_64;
    // 64-bit float, 32-bit PRNG
    // => "low-high" algorithm, see [https://www.doornik.com/research/randomdouble.pdf]
    else if constexpr (sizeof(T) == 8 && sizeof(gen_result_type) == 4)
        return (gen() * pow2_minus_64) + (gen() * pow2_minus_32);
    // 32-bit float, 64-bit PRNG
    // => discard bits + multiplication algorithm
    else if constexpr (sizeof(T) == 4 && sizeof(gen_result_type) == 8)
        return (static_cast<std::uint32_t>(gen()) >> exponent_bits_32) * mantissa_hex_32;
    // 32-bit float, 32-bit PRNG
    // => multiplication algorithm tweaked for 32-bit
    else if constexpr (sizeof(T) == 4 && sizeof(gen_result_type) == 4)
        return (gen() >> exponent_bits_32) * mantissa_hex_32;
    // 128-bit / 96-bit float and others
    else return std::generate_canonical<T, std::numeric_limits<T>::digits>(gen);

    // Note 3:
    // 'std::generate_canonical' doesn't violate 'noexcept' guarantees despite technically not being marked as such
    // (see "exceptions" [https://en.cppreference.com/w/cpp/numeric/random/generate_canonical]),
    // it does however prevent function from being constexpr
}

template <class T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
struct UniformRealDistribution {
    using result_type = T;

    struct param_type {
        result_type min = 0;
        result_type max = std::numeric_limits<result_type>::max();
    } pars{};

    constexpr UniformRealDistribution() = default;
    constexpr UniformRealDistribution(T min, T max) noexcept : pars({min, max}) { assert(min < max); }
    constexpr UniformRealDistribution(const param_type& p) noexcept : pars(p) { assert(p.min < p.max); }

    template <class Gen>
    constexpr result_type operator()(Gen& gen) const noexcept(noexcept(gen())) {
        return this->pars.min + generate_canonical<result_type>(gen) * (this->pars.max - this->pars.min);
    }

    constexpr result_type reset() const noexcept {} // there is nothing to reset, provided for std-API compatibility
    constexpr param_type  params() const noexcept { return this->pars; }
    constexpr void        params(const param_type& p) noexcept { this->pars = p; }
    constexpr result_type a() const noexcept { return this->pars.min; }
    constexpr result_type b() const noexcept { return this->pars.max; }
    constexpr result_type min() const noexcept { return this->pars.min; }
    constexpr result_type max() const noexcept { return this->pars.max; }
};

template <class T>
constexpr bool operator==(const UniformUintDistribution<T>& lhs, const UniformUintDistribution<T>& rhs) noexcept {
    return lhs.min() == rhs.min() && lhs.max() == rhs.max();
}

template <class T>
constexpr bool operator!=(const UniformUintDistribution<T>& lhs, const UniformUintDistribution<T>& rhs) noexcept {
    return !(lhs == rhs);
}

// Typedef that selects appropriate uniform distribution automatically
// MARK:

// --- Benchmark ---
// -----------------

constexpr std::uint64_t uint_min = 3;
constexpr std::uint64_t uint_max = 47;

constexpr double real_min = -3.15;
constexpr double real_max = 147.54;

template <class Generator>
void benchmark_distributions_for_prng(const char* name) {
    Generator gen{rand_seed};

    // uints
    [[maybe_unused]] const auto bench_uints = [&](auto min, auto max) {
        using uint = decltype(min);

        const uint range = max - min;

        std::vector<uint> data(data_size);

        bench.title(type_name<uint> + std::string(" distribution with ") + name)
            .timeUnit(millisecond, "ms")
            .minEpochIterations(5)
            .warmup(10)
            .relative(true);

        benchmark("std::uniform_int_distribution", [&] {
            if constexpr (sizeof(uint) >= sizeof(unsigned short)) {
                std::uniform_int_distribution dist{min, max};
                for (auto&& e : data) e = dist(gen);
            } else {
                std::uniform_int_distribution<int> dist{min, max};
                for (auto&& e : data) e = dist(gen);
            }
        });

        benchmark("uint_dist_lemier", [&] {
            const uint_dist_lemier dist{range};
            for (auto&& e : data) e = min + dist(gen);
        });

        benchmark("uint_dist_lemier_cached_threshold", [&] {
            const uint_dist_lemier_cached_threshold dist{range};
            for (auto&& e : data) e = min + dist(gen);
        });

        benchmark("uint_dist_lemier_tweak", [&] {
            const uint_dist_lemier_tweak dist{range};
            for (auto&& e : data) e = min + dist(gen);
        });

        benchmark("uint_dist_mod_1x", [&] {
            const uint_dist_mod_1x dist{range};
            for (auto&& e : data) e = min + dist(gen);
        });

        benchmark("uint_dist_mod_1x_tweak", [&] {
            const uint_dist_mod_1x_tweak dist{range};
            for (auto&& e : data) e = min + dist(gen);
        });

        benchmark("UniformUintDistribution", [&] {
            const UniformUintDistribution dist{min, max};
            for (auto&& e : data) e = dist(gen);
        });
    };

    [[maybe_unused]] const auto bench_reals = [&](auto min, auto max) {
        using real = decltype(min);

        std::vector<real> data(data_size);

        bench.title(type_name<real> + std::string(" distribution with ") + name)
            .timeUnit(millisecond, "ms")
            .minEpochIterations(5)
            .warmup(10)
            .relative(true);

        benchmark("std::uniform_real_distribution", [&] {
            std::uniform_real_distribution<real> dist{min, max};
            for (auto&& e : data) e = dist(gen);
        });

        benchmark("float_dist_std_canonical", [&] {
            const float_dist_std_canonical<real> dist{min, max};
            for (auto&& e : data) e = dist(gen);
        });

        if constexpr (sizeof(real) == 8 && sizeof(typename Generator::result_type) == 8) {
            benchmark("float_dist_mult", [&] {
                const float_dist_mult<real> dist{min, max};
                for (auto&& e : data) e = dist(gen);
            });
        }

        benchmark("UniformRealDistribution", [&] {
            const UniformRealDistribution<real> dist{min, max};
            for (auto&& e : data) e = dist(gen);
        });
    };

    // bench_uints(std::uint64_t(uint_min), std::uint64_t(uint_max));
    // bench_uints(std::uint32_t(uint_min), std::uint32_t(uint_max));
    // bench_uints(std::uint16_t(uint_min), std::uint16_t(uint_max));
    // bench_uints( std::uint8_t(uint_min),  std::uint8_t(uint_max));

    bench_reals((long double)(real_min), (long double)(real_max));
    bench_reals(double(real_min), double(real_max));
    bench_reals(float(real_min), float(real_max));
}

void benchmark_distributions() {
    log::println("\n\n====== BENCHMARKING: distributions ======\n");
    log::println("N                          -> ", data_size);
    log::println("uint range                 -> [", uint_min, ", ", uint_max, "]");
    log::println("Memory usage (uint64)      -> ", math::memory_size<std::uint64_t>(data_size), " MiB");
    log::println("Memory usage (uint32)      -> ", math::memory_size<std::uint32_t>(data_size), " MiB");
    log::println("Memory usage (uint16)      -> ", math::memory_size<std::uint16_t>(data_size), " MiB");
    log::println("Memory usage (uint8 )      -> ", math::memory_size<std::uint8_t>(data_size), " MiB");
    log::println("real range                 -> [", real_min, ", ", real_max, "]");
    log::println("Memory usage (long double) -> ", math::memory_size<long double>(data_size), " MiB");
    log::println("Memory usage (double     ) -> ", math::memory_size<double>(data_size), " MiB");
    log::println("Memory usage (float      ) -> ", math::memory_size<float>(data_size), " MiB");

    benchmark_distributions_for_prng<std::minstd_rand>("std::minstd_rand");
    benchmark_distributions_for_prng<std::mt19937>("std::mt19937");
    // benchmark_distributions_for_prng<std::mt19937_64>("std::mt19937_64");
    benchmark_distributions_for_prng<random::generators::RomuTrio32>("RomuTrio32");
    benchmark_distributions_for_prng<random::generators::JSF32>("JSF32");
    benchmark_distributions_for_prng<random::generators::RomuDuoJr>("RomuDuoJr");
    benchmark_distributions_for_prng<random::generators::JSF64>("JSF64");
    benchmark_distributions_for_prng<random::generators::Xoshiro256PlusPlus>("Xoshiro256++");
    // benchmark_distributions_for_prng<random::generators::Xorshift64Star>("Xorshift64*");
    // benchmark_distributions_for_prng<random::generators::ChaCha20>("ChaCha20");

    // Note 1:
    //
    // Caching threshhold improves:
    // - RomuTrio32 / uint64 (280 -> 340)
    // - Xoshiro256++ / uint64 (227 -> 315)
    // degrades:
    // - JSF32 / uint64 (200 -> 145)
    // - JSF32 / uint32 (102 -> 92)
    // - RomuDuoJr / uint32 (464 -> 396)
    // Verdict:
    // Don't cache, reduces sizeof()

    // Note 2:
    // For 8-bit lemier vs mod x1:
    // - 'mod 1x' seems to do better on slow PRNGs, but worse on fast ones
    // - on uint64, uint32 lemier consistently wins
    // - 'mod x1' still seems to be faster than std- uint distribution for most cases, except JSF32
}


int main() {

    benchmark_prngs();
    //benchmark_distributions();

    return 0;
}