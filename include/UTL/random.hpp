// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::random
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_random.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_RANDOM)
#ifndef UTLHEADERGUARD_RANDOM
#define UTLHEADERGUARD_RANDOM

// _______________________ INCLUDES _______________________

#include <array>            // array<>
#include <cassert>          // assert()
#include <chrono>           // high_resolution_clock
#include <cstdint>          // uint64_t
#include <initializer_list> // initializer_list<>
#include <limits>           // numeric_limits<>::digits, numeric_limits<>::min(), numeric_limits<>::max()
#include <mutex>            // mutex, lock_guard<>
#include <random>           // random_device, std::uniform_int_distribution<>,
                            // std::uniform_real_distribution<>, generate_canonical<>
#include <type_traits>      // is_integral_v<>
#include <utility>          // declval<>()
#include <vector>           // vector<>, hash<>

// ____________________ DEVELOPER DOCS ____________________

// Implements a proper modern PRNG engine, compatible with std <random>.
// Adds 'sensible std <random> wrappers' for people who aren't fond of writing
// 3 lines code just to get a simple rand value.
//
// # XorShift64StarGenerator #
// Random 'std::uint64_t' generator that satisfies uniform random number generator requirements
// from '<random>' (see https://en.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator).
// Implementation "XorShift64* suggested by Marsaglia G. in 2003 "Journal of Statistical Software"
// (see https://www.jstatsoft.org/article/view/v008i14).
// Slightly faster than 'std::rand()', while providing higher quality random.
// Significantly faster than 'std::mt19937'.
// State consists of a single 'std::uint64_t', requires seed >= 1.
//
// # xorshift64star #
// Global instance of XorShift64StarGenerator.
//
// # ::seed(), ::seed_with_time(), ::seed_with_random_device() #
// Seeds random with value/current_time/random_device.
// Random device is a better source of entropy, however it's more expensive to initialize
// than just taking current time with <ctime>, in some cases a "worse by lightweigh" can
// be prefered.
//
// # ::rand_int(), ::rand_uint(), ::rand_float(), ::rand_double() #
// Random value in [min, max] range.
// Floats with no parameters assume range [0, 1].
//
// # ::rand_bool() #
// Randomly chooses 0 or 1.
//
// # ::rand_choice() #
// Randomly chooses a value from initializer list.
//
// # ::rand_linear_combination() #
// Produces "c A + (1-c) B" with random "0 < c < 1" assuming objects "A", "B" support arithmetic operations.
// Useful for vector and color operations.

// ____________________ IMPLEMENTATION ____________________

// ======================================
// --- Ugly platform-specific entropy ---
// ======================================

// MSVC
#if defined(UTL_RANDOM_USE_INTRINSICS) && !defined(utl_random_cpu_counter) && defined(_MSC_VER)
#if defined(_M_IX86)
#include <intrin.h>
#define utl_random_cpu_counter __rdtsc()
#endif
#endif

// GCC
#if defined(UTL_RANDOM_USE_INTRINSICS) && !defined(utl_random_cpu_counter) && defined(__GNUC__)
#if __has_builtin(__builtin_ia32_rdtsc)
#define utl_random_cpu_counter __builtin_ia32_rdtsc()
#endif
#endif

// clang
#if defined(UTL_RANDOM_USE_INTRINSICS) && !defined(utl_random_cpu_counter) && !defined(__GNUC__) && defined(__clang__)
#if __has_builtin(__builtin_readcyclecounter) && __has_include(<xmmintrin.h>)
#include <xmmintrin.h>
#define utl_random_cpu_counter __builtin_readcyclecounter()
#endif
#endif

// Fallback onto a constant that changes with each compilation,
// not good, but better than nothing
#if !defined(utl_random_cpu_counter)
#include <string>
#define utl_random_cpu_counter std::hash<std::string>{}(std::string(__TIME__))
#endif

namespace utl::random {

// ============================
// --- Implementation utils ---
// ============================


// --- Type traits ---
// -------------------

#define utl_random_define_trait(trait_name_, ...)                                                                      \
    template <class T, class = void>                                                                                   \
    struct trait_name_ : std::false_type {};                                                                           \
                                                                                                                       \
    template <class T>                                                                                                 \
    struct trait_name_<T, std::void_t<decltype(__VA_ARGS__)>> : std::true_type {};                                     \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value;                                                            \
                                                                                                                       \
    template <class T>                                                                                                 \
    using trait_name_##_enable_if = std::enable_if_t<trait_name_<T>::value, bool>


utl_random_define_trait(_is_seed_seq,
                        std::declval<T>().generate(std::declval<std::uint32_t*>(), std::declval<std::uint32_t*>()));
// this type trait is necessary to restrict template constructors & seed function that take 'SeedSeq&& seq', otherwise
// they will get pick instead of regular seeding methods for even for integer conversions. This is how standard library
// seems to do it (based on GCC implementation) so we follow their API.

#undef utl_random_define_trait

template <class>
constexpr bool _always_false_v = false;

template <bool Cond>
using _require = std::enable_if_t<Cond, bool>; // makes SFINAE a bit less cumbersome

// clang-format off
template<class T> struct _wider { static_assert(_always_false_v<T>, "Missing specialization."); };

template<> struct _wider<std::uint8_t > { using type = std::uint16_t; };
template<> struct _wider<std::uint16_t> { using type = std::uint32_t; };
template<> struct _wider<std::uint32_t> { using type = std::uint64_t; };
template<> struct _wider<std::uint64_t> { using type = void; };

template<class T> using _wider_t = typename _wider<T>::type;

template <class T> constexpr bool _has_wider = !std::is_void_v<_wider_t<T>>;
// clang-format on

// --- Bit twiddling utils ---
// ---------------------------

// Merging integers into the bits of a larger one
[[nodiscard]] constexpr std::uint64_t _merge_uint32_into_uint64(std::uint32_t a, std::uint32_t b) {
    return static_cast<std::uint64_t>(a) | (static_cast<std::uint64_t>(b) << 32);
}

// Helper method to crush large uints to uint32_t,
// inspired by Melissa E. O'Neil's randutils https://gist.github.com/imneme/540829265469e673d045
template <class T, std::enable_if_t<std::is_integral_v<T> && sizeof(T) <= 8, bool> = true>
[[nodiscard]] constexpr std::uint32_t _crush_to_uint32(T value) {
    if constexpr (sizeof(value) <= 4) {
        return std::uint32_t(value);
    } else {
        std::uint64_t res = value;
        res *= 0xbc2ad017d719504d;
        return static_cast<std::uint32_t>(res ^ (res >> 32));
    }
}

// Seed sequence helpers
template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
std::uint32_t _seed_seq_to_uint32(SeedSeq&& seq) {
    std::array<std::uint32_t, 1> temp;
    seq.generate(temp.begin(), temp.end());
    return temp[0];
}

template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
std::uint64_t _seed_seq_to_uint64(SeedSeq&& seq) {
    std::array<std::uint32_t, 2> temp;
    seq.generate(temp.begin(), temp.end());
    return _merge_uint32_into_uint64(temp[0], temp[1]);
}

// 'std::rotl()' from C++20, used by many PRNGs,
// have to use long name because platform-specific includes declare '_rotl' as a macro
template <class T>
[[nodiscard]] constexpr T _rotl_value(T x, int k) noexcept {
    return (x << k) | (x >> (std::numeric_limits<T>::digits - k));
}

// Some generators shouldn't be zero initialized, in a perfect world the user would never
// do that, but in case they happened to do so regardless we can remap 0 to some "weird"
// value that isn't like to intersect with any other seeds generated by the user. Rejecting
// zero seeds completely wouldn't be appropriate for compatibility reasons.
template <class T, std::size_t N>
[[nodiscard]] constexpr bool _is_zero_state(const std::array<T, N>& state) {
    for (const auto& e : state)
        if (e != 0) return false;
    return true;
}

template <class T>
constexpr T _default_seed = std::numeric_limits<T>::max() / 2 + 1;
// an "overall decent" default seed - doesn't gave too many zeroes,
// unlikely to accidentaly match with a user-defined seed


// =========================
// --- Random Generators ---
// =========================

// Implementation of several "good" PRNGS.
//
// All generators meets uniform random number generator requirements
// (C++17 and below, see https://en.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator)
// (C++20 and above, see https://en.cppreference.com/w/cpp/numeric/random/uniform_random_bit_generator)

// Note:
// Here PRNGs take 'SeedSeq' as a forwaring reference 'SeedSeq&&', while standard PRNGS take 'SeedSeq&',
// this is how it should've been done in the standard too, but for some reason they only standardized
// l-value references, perfect forwarding probably just wasn't in use at the time.

namespace generators {

// --- 16-bit PRNGs ---
// --------------------

// Implementation of 16-bit Romu Mono engine from paper by "Mark A. Overton",
// see https://www.romu-random.org/
//     https://www.romu-random.org/romupaper.pdf
//
// Performance: Excellent
// Quality:     2/5
// State:       4 bytes
//
// Romu family provides extemely fast non-linear PRNGs, "RomuMono16" is the fastest 16-bit option available
// that still provides some resemblance of quality. There has been some concerns over the math used
// in its original paper (see https://news.ycombinator.com/item?id=22447848), however I'd yet to find
// a faster 16-bit PRNG, so if speed is needed at all cost this one provides it.
//
class RomuMono16 {
public:
    using result_type = std::uint16_t;

private:
    std::uint32_t s{}; // notice 32-bit value as a state rather than two 16-bit ints

public:
    constexpr explicit RomuMono16(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit RomuMono16(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        this->s = (seed & 0x1fffffffu) + 1156979152u; // accepts 29 seed-bits
    }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s = _seed_seq_to_uint32(seq);

        if (this->s == 0) this->seed(_default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type result = this->s >> 16;
        this->s *= 3611795771u;
        this->s = _rotl_value(this->s, 12);
        return result;
    }
};

// --- 32-bit PRNGs ---
// --------------------

// Implementation of 32-bit splitmix adopted from MurmurHash3, based on paper by Guy L. Steele,
// Doug Lea, and Christine H. Flood. 2014. "Fast splittable pseudorandom number generators"
// see http://marc-b-reynolds.github.io/shf/2017/09/27/LPRNS.html
//     https://gee.cs.oswego.edu/dl/papers/oopsla14.pdf
//     https://github.com/umireon/my-random-stuff/blob/e7b17f992955f4dbb02d4016682113b48b2f6ec1/xorshift/splitmix32.c
//
// Performance: Excellent
// Quality:     3/5
// State:       4 bytes
//
// One of the fastest 32-bit generators that requires only a single 'std::uint32_t' of state,
// making it the smallest state available. Some other PRNGs recommend using it for seeding their state.
// 32-bit version is somewhat lacking in terms of quality estimate data (relative to the widely used
// 64-bit version), however it still seems to be quite decent.
//
class SplitMix32 {
public:
    using result_type = std::uint64_t;

private:
    result_type s{};

public:
    constexpr explicit SplitMix32(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit SplitMix32(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept { this->s = seed; }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s = _seed_seq_to_uint32(seq);
    }

    constexpr result_type operator()() noexcept {
        result_type result = (this->s += 0x9e3779b9);
        result             = (result ^ (result >> 16)) * 0x21f0aaad;
        result             = (result ^ (result >> 15)) * 0x735a2d97;
        return result ^ (result >> 15);
    }
};

// Implementation of Xoshiro128++ suggested by David Blackman and Sebastiano Vigna,
// see https://prng.di.unimi.it/
//     https://prng.di.unimi.it/xoshiro256plusplus.c
//
// Performance: Good
// Quality:     4/5
// State:       16 bytes
//
// Excellent choice as a general purpose 32-bit PRNG.
// Battle-tested and provides a good statistical quality at an excellent speed.
//
class Xoshiro128PP {
public:
    using result_type = std::uint32_t;

private:
    std::array<result_type, 4> s{};

public:
    constexpr explicit Xoshiro128PP(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit Xoshiro128PP(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    // while zero-state is considered invalid, PRNG can still produce 0 as a result
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        SplitMix32 splitmix{seed};
        this->s[0] = splitmix(); // Xoshiro family recommends using
        this->s[1] = splitmix(); // splitmix to initialize its state
        this->s[2] = splitmix();
        this->s[3] = splitmix();
    }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        seq.generate(this->s.begin(), this->s.end());

        // ensure we don't hit an invalid all-zero state
        if (_is_zero_state(this->s)) this->seed(_default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type result = _rotl_value(this->s[0] + this->s[3], 7) + this->s[0];
        const result_type t      = s[1] << 9;
        this->s[2] ^= this->s[0];
        this->s[3] ^= this->s[1];
        this->s[1] ^= this->s[2];
        this->s[0] ^= this->s[3];
        this->s[2] ^= t;
        this->s[3] = _rotl_value(this->s[3], 11);
        return result;
    }
};

// Implementation of 32-bit Romu Trio engine from paper by "Mark A. Overton",
// see https://www.romu-random.org/
//     https://www.romu-random.org/romupaper.pdf
//
// Performance: Excellent
// Quality:     2/5
// State:       12 bytes
//
// Romu family provides extemely fast non-linear PRNGs, "RomuTrio" is the fastest 32-bit option available
// that still provides some resemblance of quality. There has been some concerns over the math used
// in its original paper (see https://news.ycombinator.com/item?id=22447848), however I'd yet to find
// a faster 32-bit PRNG, so if speed is needed at all cost this one provides it.
//
class RomuTrio32 {
public:
    using result_type = std::uint32_t;

private:
    std::array<result_type, 3> s{};

public:
    constexpr explicit RomuTrio32(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit RomuTrio32(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        SplitMix32 splitmix{seed};
        this->s[0] = splitmix(); // Like Xoshiro, Romu recommends
        this->s[1] = splitmix(); // using SplitMix32 to initialize its state
        this->s[2] = splitmix();
    }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        seq.generate(this->s.begin(), this->s.end());

        // ensure we don't hit an invalid all-zero state
        if (_is_zero_state(this->s)) this->seed(_default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type xp = this->s[0], yp = this->s[1], zp = this->s[2];
        this->s[0] = 3323815723u * zp;
        this->s[1] = yp - xp;
        this->s[1] = _rotl_value(this->s[1], 6);
        this->s[2] = zp - yp;
        this->s[2] = _rotl_value(this->s[2], 22);
        return xp;
    }
};

// --- 64-bit PRNGs ---
// --------------------

// Implementation of fixed-increment version of Java 8's SplittableRandom generator SplitMix64,
// see http://dx.doi.org/10.1145/2714064.2660195
//     http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html
//     https://rosettacode.org/wiki/Pseudo-random_numbers/Splitmix64
//
// Performance: Excellent
// Quality:     4/5
// State:       8 bytes
//
// One of the fastest generators passing BigCrush that requires only a single 'std::uint64_t' of state,
// making it the smallest state available. Some other PRNGs recommend using it for seeding their state.
//
class SplitMix64 {
public:
    using result_type = std::uint64_t;

private:
    result_type s{};

public:
    constexpr explicit SplitMix64(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit SplitMix64(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept { this->s = seed; }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s = _seed_seq_to_uint64(seq);
    }

    constexpr result_type operator()() noexcept {
        std::uint64_t result = (this->s += 0x9E3779B97f4A7C15);
        result               = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
        result               = (result ^ (result >> 27)) * 0x94D049BB133111EB;
        return result ^ (result >> 31);
    }
};

// Implementation of Xoshiro256++ suggested by David Blackman and Sebastiano Vigna,
// see https://prng.di.unimi.it/
//     https://prng.di.unimi.it/xoshiro256plusplus.c
//
// Performance: Good
// Quality:     4/5
// State:       32 bytes
//
// Excellent choice as a general purpose PRNG.
// Used by several modern languages as their default.
//
class Xoshiro256PP {
public:
    using result_type = std::uint64_t;

private:
    std::array<result_type, 4> s{};

public:
    constexpr explicit Xoshiro256PP(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit Xoshiro256PP(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    // while zero-state is considered invalid, PRNG can still produce 0 as a result
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        SplitMix64 splitmix{seed};
        this->s[0] = splitmix(); // Xoshiro family recommends using
        this->s[1] = splitmix(); // splitmix to initialize its state
        this->s[2] = splitmix();
        this->s[3] = splitmix();
    }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s[0] = _seed_seq_to_uint64(seq); // since seed_seq produces 32-bit ints,
        this->s[1] = _seed_seq_to_uint64(seq); // we have to generate multiple and then
        this->s[2] = _seed_seq_to_uint64(seq); // join them into std::uint64_t's to
        this->s[3] = _seed_seq_to_uint64(seq); // properly initialize the entire state

        // ensure we don't hit an invalid all-zero state
        if (_is_zero_state(this->s)) this->seed(_default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type result = _rotl_value(this->s[0] + this->s[3], 23) + this->s[0];
        const result_type t      = this->s[1] << 17;
        this->s[2] ^= this->s[0];
        this->s[3] ^= this->s[1];
        this->s[1] ^= this->s[2];
        this->s[0] ^= this->s[3];
        this->s[2] ^= t;
        this->s[3] = _rotl_value(this->s[3], 45);
        return result;
    }
};

// Implementation of Romu DuoJr engine from paper by "Mark A. Overton",
// see https://www.romu-random.org/
//     https://www.romu-random.org/romupaper.pdf
//
// Performance: Excellent
// Quality:     2/5
// State:       16 bytes
//
// Romu family provides extemely fast non-linear PRNGs, "DuoJr" is the fastest 64-bit option available
// that still provides some resemblance of quality. There has been some concerns over the math used
// in its original paper (see https://news.ycombinator.com/item?id=22447848), however I'd yet to find
// a faster 64-bit PRNG, so if speed is needed at all cost this one provides it.
//
class RomuDuoJr64 {
public:
    using result_type = std::uint64_t;

private:
    std::array<result_type, 2> s{};

public:
    constexpr explicit RomuDuoJr64(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit RomuDuoJr64(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        SplitMix64 splitmix{seed};
        this->s[0] = splitmix(); // Like Xoshiro, Romu recommends
        this->s[1] = splitmix(); // using SplitMix64 to initialize its state
    }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s[0] = _seed_seq_to_uint64(seq); // seed_seq returns 32-bit ints, we have to generate
        this->s[1] = _seed_seq_to_uint64(seq); // multiple to initialize full state of 64-bit values

        // ensure we don't hit an invalid all-zero state
        if (_is_zero_state(this->s)) this->seed(_default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type res = this->s[0];
        this->s[0]            = 15241094284759029579u * this->s[1];
        this->s[1]            = this->s[1] - res;
        this->s[1]            = _rotl_value(this->s[1], 27);
        return res;
    }
};

// --- CSPRNGs ---
// ---------------

// Implementation of ChaCha20 CPRNG conforming to RFC 7539 standard
// see https://datatracker.ietf.org/doc/html/rfc7539
//     https://www.rfc-editor.org/rfc/rfc7539#section-2.4
//     https://en.wikipedia.org/wiki/Salsa20

// Quarted-round operation for ChaCha20 stream cipher
constexpr void _quarter_round(std::uint32_t& a, std::uint32_t& b, std::uint32_t& c, std::uint32_t& d) {
    a += b, d ^= a, d = _rotl_value(d, 16);
    c += d, b ^= c, b = _rotl_value(b, 12);
    a += b, d ^= a, d = _rotl_value(d, 8);
    c += d, b ^= c, b = _rotl_value(b, 7);
}

template <std::size_t rounds>
[[nodiscard]] constexpr std::array<std::uint32_t, 16> _chacha_rounds(const std::array<std::uint32_t, 16>& input) {
    auto state = input;

    static_assert(rounds % 2 == 0, "ChaCha rounds happen in pairs, total number should be divisible by 2.");

    constexpr std::size_t alternating_round_pairs = rounds / 2;
    // standard number of ChaCha rounds as per RFC 7539 is 20 (ChaCha20 variation),
    // however there is a strong case for ChaCha12 being a more sensible default,
    // at the moment the upper bound of what seems "crackable" is somewhere around 7 rounds,
    // which is why ChaCha8 is also widely used whenever speed is necessary.

    for (std::size_t i = 0; i < alternating_round_pairs; ++i) {
        // Column rounds
        _quarter_round(state[0], state[4], state[8], state[12]);
        _quarter_round(state[1], state[5], state[9], state[13]);
        _quarter_round(state[2], state[6], state[10], state[14]);
        _quarter_round(state[3], state[7], state[11], state[15]);

        // Diagonal rounds
        _quarter_round(state[0], state[5], state[10], state[15]);
        _quarter_round(state[1], state[6], state[11], state[12]);
        _quarter_round(state[2], state[7], state[8], state[13]);
        _quarter_round(state[3], state[4], state[9], state[14]);
    }

    for (std::size_t i = 0; i < state.size(); ++i) state[i] += input[i];
    return state;
}

template <std::size_t rounds>
class ChaCha {
public:
    using result_type = std::uint32_t;

private:
    // Initial state components
    std::array<result_type, 8> key{};     // 256-bit key
    std::array<result_type, 3> nonce{};   // 96-bit nonce
    std::uint32_t              counter{}; // 32-bit counter

    // Block
    std::array<result_type, 16> block{};    // holds next 16 random numbers
    std::size_t                 position{}; // current position in the block

    constexpr static std::array<result_type, 4> constant = {0x61707865, 0x3320646e, 0x79622d32, 0x6b206574};
    // "Magic constants" for ChaCha20 are defined through bit representations of the following char arrays:
    // { "expa", "nd 3", "2-by", "te k" },
    // what we have here is exactly that except written as 'std::uint32_t'

    constexpr void generate_new_block() {
        // Set ChaCha20 initial state as per RFC 7539
        //
        //          [ const   const const const ]
        // State    [ key     key   key   key   ]
        // matrix = [ key     key   key   key   ]
        //          [ counter nonce nonce nonce ]
        //
        const std::array<std::uint32_t, 16> input = {
            this->constant[0], this->constant[1], this->constant[2], this->constant[3], //
            this->key[0],      this->key[1],      this->key[2],      this->key[3],      //
            this->key[4],      this->key[5],      this->key[6],      this->key[7],      //
            this->counter,     this->nonce[0],    this->nonce[1],    this->nonce[2]     //
        };

        // Fill new block
        this->block = _chacha_rounds<rounds>(input);
        ++this->counter;
    }

public:
    constexpr explicit ChaCha(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit ChaCha(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) {
        // Use some other PRNG to setup initial state
        SplitMix32 splitmix{seed};
        for (auto& e : this->key) e = splitmix();
        for (auto& e : this->nonce) e = splitmix();
        this->counter  = 0; // counter can be set to any number, but usually 0 or 1 is used
        this->position = 0;

        this->generate_new_block();
    }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        // Seed sequence allows user to introduce more entropy into the state

        seq.generate(this->key.begin(), this->key.end());
        seq.generate(this->nonce.begin(), this->nonce.end());

        this->counter  = 0; // counter can be set to any number, but usually 0 or 1 is used
        this->position = 0;

        this->generate_new_block();
    }

    constexpr result_type operator()() noexcept {
        // Generate new block if necessary
        if (this->position >= 16) {
            this->generate_new_block();
            this->position = 0;
        }

        // Get random value from the block and advance position cursor
        return this->block[this->position++];
    }
};

using ChaCha8  = ChaCha<8>;
using ChaCha12 = ChaCha<12>;
using ChaCha20 = ChaCha<20>;


} // namespace generators

// ===========================
// --- Default global PRNG ---
// ===========================

using default_generator_type = generators::Xoshiro256PP;
using default_result_type    = default_generator_type::result_type;

inline default_generator_type default_generator;

inline std::seed_seq entropy_seq() {
    // Ensure thread safery of our entropy source, it should generally work fine even without
    // it, but with this we can be sure things never race
    static std::mutex     entropy_mutex;
    const std::lock_guard entropy_guard(entropy_mutex);

    // Hardware entropy (if implemented),
    // some platfroms (mainly MinGW) implements random device as a regular PRNG that
    // doesn't change from run to run, this is horrible, but we can somewhat improve
    // things by mixing other sources of entropy. Since hardware entropy is a rather
    // limited resource we only call it once.
    static std::uint32_t          seed_rd = std::random_device{}();
    // after that we just scramble it with a regular PRNG
    static generators::SplitMix32 splitmix{seed_rd};
    seed_rd = splitmix();

    // Time in nanoseconds (on some platforms microseconds)
    const auto seed_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    // Heap address (tends to be random each run on most platforms)
    std::vector<std::uint32_t> dummy_vec(1, seed_rd);
    const std::size_t          heap_address_hash = std::hash<std::uint32_t*>{}(dummy_vec.data());

    // Stack address (also tends to be random)
    const std::size_t stack_address_hash = std::hash<decltype(heap_address_hash)*>{}(&heap_address_hash);

    // CPU counter (if available, hashed compilation time otherwise)
    const auto cpu_counter = static_cast<std::uint64_t>(utl_random_cpu_counter);

    // Note:
    // There are other sources of entropy, such as function adresses,
    // but those can be rather "constant" on some platforms

    return {seed_rd, _crush_to_uint32(seed_time), _crush_to_uint32(heap_address_hash),
            _crush_to_uint32(stack_address_hash), _crush_to_uint32(cpu_counter)};
}

inline std::uint32_t entropy() {
    auto seq = entropy_seq();
    return _seed_seq_to_uint32(seq);
    // returns 'std::uint32_t' to mimic the return type of 'std::random_device', if we return uint64_t
    // brace-initializers will complain about narrowing conversion on some generators. If someone want
    // more entropy than that they can always use the whole sequence as a generic solution.
    // Also having one 'random::entropy()' is much nicer than 'random::entropy_32()' & 'random::entropy_64()'.
}

inline void seed(default_result_type random_seed) noexcept { default_generator.seed(random_seed); }

inline void seed_with_entropy() {
    auto seq = entropy_seq();
    default_generator.seed(seq);
    // for some god-forsaken reason seeding sequence constructors std:: generators take only l-value sequences
}

// =====================
// --- Distributions ---
// =====================

// --- Uniform int distribution ---
// --------------------------------

template <class T, class Gen, _require<std::is_integral_v<T> && std::is_unsigned_v<T>> = true>
constexpr T _uniform_uint_lemier(Gen& gen, T range) noexcept(noexcept(gen())) {
    using W = _wider_t<T>;

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

template <class T, class Gen, _require<std::is_integral_v<T> && std::is_unsigned_v<T>> = true>
constexpr T _uniform_uint_modx1(Gen& gen, T range) noexcept(noexcept(gen())) {
    T x{}, r{};
    do {
        x = gen();
        r = x % range;
    } while (x - r > T(-range));
    return r;
} // slightly slower than lemier, but doesn't require a wider type

// Reimplementation of libc++ `std::uniform_int_distribution<>` except
// - constexpr
// - const-qualified (relative to distribution parameters)
// - noexcept as long as 'Gen::operator()' is noexcept, which is true for all generators in this module
// - supports `std::uint8_t`, `std::int8_t`, `char`
// - produces the same sequence on each platform
// Performance is exactly the same a libc++ version of `std::uniform_int_distribution<>`,
// in fact, it is likely to return the exact same sequence for all types except 64-bit integers
template <class T, class Gen, _require<std::is_integral_v<T>> = true>
constexpr T _generate_uniform_int(Gen& gen, T min, T max) noexcept {
    using result_type    = T;
    using unsigned_type  = std::make_unsigned_t<result_type>;
    using generated_type = typename Gen::result_type;
    using common_type    = std::common_type_t<unsigned_type, generated_type>;

    constexpr common_type prng_min = Gen::min();
    constexpr common_type prng_max = Gen::max();

    static_assert(prng_min < prng_max, "UniformRandomBitGenerator requires 'min() < max()'");

    constexpr common_type prng_range = prng_max - prng_min;
    constexpr common_type type_range = std::numeric_limits<common_type>::max();
    const common_type     range      = common_type(max) - common_type(min);

    common_type res{};

    // PRNG has enough state for the range
    if (prng_range > range) {
        const common_type ext_range = range + 1; // range can be zero

        // PRNG uses all 'common_type' bits uniformly
        // => use Lemier's algorithm if possible, fallback onto modx1 otherwise,
        //    libc++ uses conditionally compiled lemier's with 128-bit ints intead of doing a fallback,
        //    this is slightly faster, but leads to platforms-dependant sequences
        if constexpr (prng_range == type_range) {
            if constexpr (_has_wider<common_type>) res = _uniform_uint_lemier(gen, ext_range);
            else res = _uniform_uint_modx1(gen, ext_range);
        }
        // PRNG doesn't use all bits uniformly (usually because 'prng_min' is '1')
        // => fallback onto a 2-division algorithm
        else {
            const common_type scaling = prng_range / ext_range;
            const common_type past    = ext_range * scaling;

            do { res = common_type(gen()) - prng_min; } while (res >= past);
            res /= scaling;
        }
    }
    // PRNG needs several invocations to aquire enough state for the range
    else if (prng_range < range) {
        common_type temp{};
        do {
            constexpr common_type ext_prng_range = (prng_range < type_range) ? prng_range + 1 : type_range;
            temp = ext_prng_range * _generate_uniform_int<common_type>(gen, 0, range / ext_prng_range);
            res  = temp + (common_type(gen()) - prng_min);
        } while (res >= range || res < temp);
    } else {
        res = common_type(gen()) - prng_min;
    }

    return min + res;

    // Note 1:
    // 'static_cast<>()' preserves bit pattern of signed/unsigned integers of the same size as long as
    // those integers are two-complement (see https://en.wikipedia.org/wiki/Two's_complement), this is
    // true for most platforms and is in fact guaranteed for standard fixed-width types like 'uint32_t'
    // on any platform (see https://en.cppreference.com/w/cpp/types/integer)
    //
    // This means signed integer distribution can simply use unsigned algorithm and reinterpret the result internally.
    // This would be a bit nicer semantically with C++20 `std::bit_cast<>`, but not ultimately any different.

    // Note 2:
    // 'ext_prng_range' has a ternary purely to silence a false compiler warning from about division by zero due to
    // 'prng_range + 1' overflowing into '0' when 'prng_range' is equal to 'type_range'. Falling into this runtime
    // branch requires 'prng_range < range <= type_range' making such situation impossible, here we simply clamp the
    // value to 'type_range' so it doesn't overflow and trip the compiler when analyzing constexpr for potential UB.
}

// 'static_cast<>()' preserves bit pattern of signed/unsigned integers of the same size as long as
// those integers are two-complement (see https://en.wikipedia.org/wiki/Two's_complement), this is
// true for most platforms and is in fact guaranteed for standard fixed-width types like 'uint32_t'
// on any platform (see https://en.cppreference.com/w/cpp/types/integer)
//
// This means signed integer distribution can simply use unsigned algorithm and reinterpret the result internally.
// This would be a bit nicer semantically with C++20 `std::bit_cast<>`, but not ultimately any different.
template <class T, _require<std::is_integral_v<T>> = true>
struct UniformIntDistribution {
    using result_type = T;

    struct param_type {
        result_type min = 0;
        result_type max = std::numeric_limits<result_type>::max();
    };

    constexpr UniformIntDistribution() = default;
    constexpr UniformIntDistribution(T min, T max) noexcept : pars({min, max}) { assert(min < max); }
    constexpr UniformIntDistribution(const param_type& p) noexcept : pars(p) { assert(p.min < p.max); }

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept(noexcept(gen())) {
        return _generate_uniform_int<result_type>(gen, this->pars.min, this->pars.max);
    }

    template <class Gen>
    constexpr T operator()(Gen& gen, const param_type& p) const noexcept(noexcept(gen())) {
        assert(p.min < p.max);
        return _generate_uniform_int<result_type>(gen, p.min, p.max);
    } // for std-compatibility

    constexpr result_type               reset() const noexcept {} // nothing to reset, provided for std-compatibility
    [[nodiscard]] constexpr param_type  params() const noexcept { return this->pars; }
    constexpr void                      params(const param_type& p) noexcept { *this = UniformIntDistribution(p); }
    [[nodiscard]] constexpr result_type a() const noexcept { return this->pars.min; }
    [[nodiscard]] constexpr result_type b() const noexcept { return this->pars.max; }
    [[nodiscard]] constexpr result_type min() const noexcept { return this->pars.min; }
    [[nodiscard]] constexpr result_type max() const noexcept { return this->pars.max; }

private:
    param_type pars{};
};

template <class T>
constexpr bool operator==(const UniformIntDistribution<T>& lhs, const UniformIntDistribution<T>& rhs) noexcept {
    return lhs.a() == rhs.a() && lhs.b() == rhs.b();
}

// --- Uniform real distribution ---
// ---------------------------------

// Can't really make things work without making some reasonable assumptions about the size of primitive types,
// this should be satisfied for the vast majority of platforms. Esoteric architectures can manually adapt the
// algorithm if that is necessary.
static_assert(std::numeric_limits<double>::digits == 53, "Platform not supported, 'float' is expected to be 32-bit.");
static_assert(std::numeric_limits<float>::digits == 24, "Platform not supported, 'double' is expected to be 64-bit.");

template <class T>
constexpr int _bit_width(T value) noexcept {
    int width = 0;
    while (value >>= 1) ++width;
    return width;
}

// Constexpr reimplementation of 'std::generate_canonical<>()'
template <class T, class Gen>
constexpr T _generate_canonical_generic(Gen& gen) noexcept(noexcept(gen())) {
    using float_type     = T;
    using generated_type = typename Gen::result_type;

    constexpr int float_bits = std::numeric_limits<float_type>::digits;
    // always produce enough bits of randomness for the whole mantissa

    constexpr generated_type prng_max = Gen::max();
    constexpr generated_type prng_min = Gen::min();

    constexpr generated_type prng_range = prng_max - prng_min;
    constexpr generated_type type_range = std::numeric_limits<generated_type>::max();

    constexpr int prng_bits = (prng_range < type_range) ? _bit_width(prng_range + 1) : 1 + _bit_width(prng_range);
    // how many full bits of randomness PRNG produces on each invocation, prng_bits == floor(log2(prng_range + 1)),
    // ternary handles the case that would overflow when (prng_range == type_range)

    constexpr int invocations_needed = [&]() {
        int count = 0;
        for (int generated_bits = 0; generated_bits < float_bits; generated_bits += prng_bits) ++count;
        return count;
    }();
    // GCC and MSVC use runtime conversion to floating point and std::ceil() & std::log() to obtain
    // this value, in MSVC for example we have something like this:
    //    > invocations_needed = std::ceil( float_type(float_bits) / std::log2( float_type(prng_range) + 1 ) )
    // which is not constexpr due to math functions, we can do a similar thing much easier by just counting bits
    // generated per each invocation. This returns the same thing for any sane PRNG, except since it only counts
    // "full bits" esoteric ranges such as [1, 3] which technically have 1.5 bits of randomness will be counted
    // as 1 bit of randomness, thus overestimating the invocations a little. In practice this makes 0 difference
    // since its only matters for exceedingly small 'prng_range' and such PRNGs simply don't exist in nature, and
    // even if they are theoretically used they will simply use a few more invocation to produce a proper result

    constexpr float_type prng_float_max   = static_cast<float_type>(prng_max);
    constexpr float_type prng_float_min   = static_cast<float_type>(prng_min);
    constexpr float_type prng_float_range = (prng_float_max - prng_float_min) + float_type(1);

    float_type res    = float_type(0);
    float_type factor = float_type(1);

    for (int i = 0; i < invocations_needed; ++i) {
        res += (static_cast<float_type>(gen()) - static_cast<float_type>(prng_min)) * factor;
        factor *= prng_float_range;
    } // same algorithm is used by 'std::generate_canonical<>' in all major compilers as of 2025

    return res / factor;
}

// Wrapper that adds special case optimizations for `_generate_canonical_generic<>()'
template <class T, class Gen>
constexpr T generate_canonical(Gen& gen) noexcept(noexcept(gen())) {
    using float_type     = T;
    using generated_type = typename Gen::result_type;

    constexpr generated_type prng_min = Gen::min();
    constexpr generated_type prng_max = Gen::max();

    static_assert(prng_min < prng_max, "UniformRandomBitGenerator requires 'min() < max()'");

    constexpr generated_type prng_range          = prng_max - prng_min;
    constexpr generated_type type_range          = std::numeric_limits<generated_type>::max();
    constexpr bool           prng_is_bit_uniform = (prng_range == type_range);

    constexpr int exponent_bits_64 = 11;
    constexpr int exponent_bits_32 = 8;

    constexpr double mantissa_hex_64 = 0x1.0p-53;  // == 2^-53, corresponds to 53 significant bits of double
    constexpr float  mantissa_hex_32 = 0x1.0p-24f; // == 2^-24, corresponds to 24 significant bits of float

    constexpr double pow2_minus_64 = 0x1.0p-64; // == 2^-64
    constexpr double pow2_minus_32 = 0x1.0p-32; // == 2^-32

    // Note 1: Note hexadecimal float literals, 'p' separates hex-base from the exponent
    // Note 2: Floats have 'mantissa_size + 1' significant bits due to having a sign bit

    // Bit-uniform PRNGs can be simply bitmasked & shifted to obtain mantissa
    // 64-bit float, 64-bit uniform PRNG
    // => multiplication algorithm, see [https://prng.di.unimi.it/]
    if constexpr (prng_is_bit_uniform && sizeof(float_type) == 8 && sizeof(generated_type) == 8) {
        return (gen() >> exponent_bits_64) * mantissa_hex_64;
    }
    // 64-bit float, 32-bit uniform PRNG
    // => "low-high" algorithm, see [https://www.doornik.com/research/randomdouble.pdf]
    else if constexpr (prng_is_bit_uniform && sizeof(T) == 8 && sizeof(generated_type) == 4) {
        return (gen() * pow2_minus_64) + (gen() * pow2_minus_32);
    }
    // 32-bit float, 64-bit uniform PRNG
    // => discard bits + multiplication algorithm
    else if constexpr (prng_is_bit_uniform && sizeof(T) == 4 && sizeof(generated_type) == 8) {
        return (static_cast<std::uint32_t>(gen()) >> exponent_bits_32) * mantissa_hex_32;
    }
    // 32-bit float, 32-bit uniform PRNG
    // => multiplication algorithm tweaked for 32-bit
    else if constexpr (prng_is_bit_uniform && sizeof(T) == 4 && sizeof(generated_type) == 4) {
        return (gen() >> exponent_bits_32) * mantissa_hex_32;
    }
    // Generic case, no particular optimizations can be made
    else {
        return _generate_canonical_generic<T>(gen);
    }
}

template <class T, _require<std::is_floating_point_v<T>> = true>
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

    template <class Gen>
    constexpr T operator()(Gen& gen, const param_type& p) const noexcept(noexcept(gen())) {
        assert(p.min < p.max);
        return p.min + generate_canonical<result_type>(gen) * (p.max - p.min);
    } // for std-compatibility

    constexpr result_type reset() const noexcept {} // there is nothing to reset, provided for std-API compatibility
    constexpr param_type  params() const noexcept { return this->pars; }
    constexpr void        params(const param_type& p) noexcept { *this = UniformRealDistribution(p); }
    constexpr result_type a() const noexcept { return this->pars.min; }
    constexpr result_type b() const noexcept { return this->pars.max; }
    constexpr result_type min() const noexcept { return this->pars.min; }
    constexpr result_type max() const noexcept { return this->pars.max; }
};

template <class T>
constexpr bool operator==(const UniformRealDistribution<T>& lhs, const UniformRealDistribution<T>& rhs) noexcept {
    return lhs.a() == rhs.a() && lhs.b() == rhs.b();
}

// ========================
// --- Random Functions ---
// ========================

// Note 1:
// Despite the intuitive judgement, benchmarks don't seem to indicate that creating
// new distribution objects on each call introduces any noticeble overhead
//
// sizeof(std::uniform_int_distribution<int>)     ==  8
// sizeof(std::uniform_real_distribution<double>) == 16
// sizeof(std::normal_distribution<double>)       == 32
//
// and same thing for 'UniformIntDistribution', 'UniformRealDistribution'

// Note 2:
// No '[[nodiscard]]' since random functions inherently can't be pure due to advancing the generator state.
// Discarding return values while not very sensible, can still be done for the sake of advancing state.
// Ideally we would want users to advance the state directly, but I'm not sure how to communicate that in
// '[[nodiscard]]' warnings.

inline int rand_int(int min, int max) noexcept {
    const UniformIntDistribution<int> distr{min, max};
    return distr(default_generator);
}

inline int rand_uint(unsigned int min, unsigned int max) noexcept {
    const UniformIntDistribution<unsigned int> distr{min, max};
    return distr(default_generator);
}

inline float rand_float() noexcept { return generate_canonical<float>(default_generator); }

inline float rand_float(float min, float max) noexcept {
    const UniformRealDistribution<float> distr{min, max};
    return distr(default_generator);
}

inline float rand_normal_float() {
    std::normal_distribution<float> distr;
    return distr(default_generator);
}

inline double rand_double() noexcept { return generate_canonical<double>(default_generator); }

inline double rand_double(double min, double max) noexcept {
    const UniformRealDistribution<double> distr{min, max};
    return distr(default_generator);
}

inline double rand_normal_double() {
    std::normal_distribution<double> distr;
    return distr(default_generator);
}

inline bool rand_bool() noexcept { return static_cast<bool>(rand_uint(0, 1)); }

template <class T>
const T& rand_choice(std::initializer_list<T> objects) noexcept {
    const int random_index = rand_int(0, static_cast<int>(objects.size()) - 1);
    return objects.begin()[random_index];
}

template <class T>
T rand_linear_combination(const T& A, const T& B) noexcept(noexcept(A + B) && noexcept(A * 1.)) {
    const auto coef = rand_double();
    return A * coef + B * (1. - coef);
} // random linear combination of 2 colors/vectors/etc

} // namespace utl::random

#endif
#endif // module utl::random
