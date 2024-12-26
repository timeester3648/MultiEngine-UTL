// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::random
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_random.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_RANDOM)
#ifndef UTLHEADERGUARD_RANDOM
#define UTLHEADERGUARD_RANDOM

// _______________________ INCLUDES _______________________

#include <cstdint>          // uint64_t
#include <initializer_list> // initializer_list<>
#include <limits>           // numeric_limits<>::digits, numeric_limits<>::min(), numeric_limits<>::max()
#include <random>           // random_device, std::uniform_int_distribution<>,
                            // std::uniform_real_distribution<>, generate_canonical<>
#include <array>            // array<>
#include <chrono>           // high_resolution_clock
#include <mutex>            // mutex, lock_guard<>
#include <type_traits>      // is_integral_v<>
#include <utility>          // declval<>()

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
// # ::rand_choise() #
// Randomly chooses a value from initializer list.
//
// # ::rand_linear_combination() #
// Produces "c A + (1-c) B" with random "0 < c < 1" assuming objects "A", "B" support arithmetic operations.
// Useful for vector and color operations.

// ____________________ IMPLEMENTATION ____________________

namespace utl::random {

// ============================
// --- Implementation utils ---
// ============================

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
// this type trait is necessary to restrict template constructors & seed function that take 'SeedSeq& seq', otherwise
// they will get pick instead of regular seeding methods for even for integer conversions. This is how standard library
// seems to do it (based on GCC implementation) so we follow their API.

#undef utl_random_define_trait

// A 'lightweight' PRNG used by some other engines to initialize their state
constexpr std::uint32_t _splitmix32(std::uint32_t& state) noexcept {
    uint32_t result = (state += 0x9e3779b9);
    result          = (result ^ (result >> 16)) * 0x21f0aaad;
    result          = (result ^ (result >> 15)) * 0x735a2d97;
    return result ^ (result >> 15);
}

constexpr std::uint64_t _splitmix64(std::uint64_t& state) noexcept {
    std::uint64_t result = (state += 0x9E3779B97f4A7C15);
    result               = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
    result               = (result ^ (result >> 27)) * 0x94D049BB133111EB;
    return result ^ (result >> 31);
}

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
std::uint32_t _seed_seq_to_uint32(SeedSeq& seq) {
    std::array<std::uint32_t, 1> temp;
    seq.generate(temp.begin(), temp.end());
    return temp[0];
}

template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
std::uint64_t _seed_seq_to_uint64(SeedSeq& seq) {
    std::array<std::uint32_t, 2> temp;
    seq.generate(temp.begin(), temp.end());
    return _merge_uint32_into_uint64(temp[0], temp[1]);
}

// 'std::rotl()' from C++20, used by many PRNGs
template <class T>
[[nodiscard]] constexpr T _rotl(T x, int k) noexcept {
    return (x << k) | (x >> (std::numeric_limits<T>::digits - k));
}

template <class T>
constexpr T _default_seed = std::numeric_limits<T>::max() / 2;
// an "overall decent" default seed - doesn't gave too many zeroes,
// unlikely to accidentaly match with a user-defined seed

template <class T>
[[nodiscard]] constexpr T _ensure_nonzero(T value) noexcept {
    return (value) ? value : _default_seed<T> + 1;
}
// some generators shouldn't be zero initialized, in a perfect world the user would never
// do that, but in case they happened to do so regardless we can remap 0 to some "weird"
// value that isn't like to intersect with any other seeds generated by the user. Rejecting
// zero seeds completely wouldn't be appropriate for compatibility reasons.

// =========================
// --- Random Generators ---
// =========================

// Implementation of several "good" PRNGS.
//
// All generators meets uniform random number generator requirements
// (C++17 and below, see https://en.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator)
// (C++20 and above, see https://en.cppreference.com/w/cpp/numeric/random/uniform_random_bit_generator)

// Note:
// As of C++17 there is little point in declaring generator constructors and methods as 'constexpr',
// however in newer standards it would allow PRNGs to be used in constexpr context.
//
// Unfortunately, we can't do the same for "convenient random functions"
// since  they use std distributions which have no constexpr.

namespace generators {

// --- 32-bit PRNGs ---
// --------------------

// Implementation of 32-bit Romu Trio engine from paper by "Mark A. Overton",
// see https://www.romu-random.org/
//     https://www.romu-random.org/romupaper.pdf
//
// Performance: Excellent
// Quality:     3/5
// State:       16 bytes
//
// Not the best one for general use, but provides bleeding edge performance.
// Depending on 32/64-bit CPU instruction set can lead to huge speedup.
//
class RomuTrio32 {
public:
    using result_type = std::uint32_t;

private:
    std::array<result_type, 3> s{};

public:
    constexpr explicit RomuTrio32(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit RomuTrio32(SeedSeq& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 1; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        result_type splitmix32_state = _ensure_nonzero(seed); // RomuTrio shouln't be zero-initialized
        this->s[0]                   = _splitmix32(splitmix32_state);
        this->s[1]                   = _splitmix32(splitmix32_state);
        // Like Xoshiro, Romu recommends using SplitMix32 to initialize its state
    }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq& seq) {
        seq.generate(this->s.begin(), this->s.end());
    }

    constexpr result_type operator()() noexcept {
        const result_type xp = this->s[0], yp = this->s[1], zp = this->s[2];
        this->s[0] = 3323815723u * zp;
        this->s[1] = yp - xp;
        this->s[1] = _rotl(this->s[1], 6);
        this->s[2] = zp - yp;
        this->s[2] = _rotl(this->s[2], 22);
        return xp;
    }
};

// Implementation of 32-bit Bob Jenkins' small prng ,
// see https://burtleburtle.net/bob/rand/smallprng.html
//     https://gist.github.com/imneme/85cff47d4bad8de6bdeb671f9c76c814
//
// Performance: Excellent
// Quality:     3/5
// State:       16 bytes
//
// Not the best one for general use, but provides bleeding edge performance.
// Depending on 32/64-bit CPU instruction set can lead to huge speedup.
//
class JSF32 {
public:
    using result_type = std::uint32_t;

private:
    std::array<result_type, 4> s{};

public:
    constexpr explicit JSF32(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit JSF32(SeedSeq& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return std::numeric_limits<result_type>::min(); }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        this->s[0] = 0xf1ea5eed;
        this->s[1] = this->s[2] = this->s[3] = _ensure_nonzero(seed);
        // JSF shouln't struggle with zero-seeding but just in case
        for (int i = 0; i < 20; ++i) static_cast<void>(this->operator()());
    }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq& seq) {
        seq.generate(this->s.begin(), this->s.end());
    }

    constexpr result_type operator()() noexcept {
        const result_type e = this->s[0] - _rotl(this->s[1], 27);
        this->s[0]          = this->s[1] ^ _rotl(this->s[2], 17);
        this->s[1]          = this->s[2] + this->s[3];
        this->s[2]          = this->s[3] + e;
        this->s[3]          = e + this->s[0];
        return this->s[3];
    }
};

// --- 64-bit PRNGs ---
// --------------------

// Implementation of Romu DuoJr engine from paper by "Mark A. Overton",
// see https://www.romu-random.org/
//     https://www.romu-random.org/romupaper.pdf
//
// Performance: Excellent
// Quality:     3/5
// State:       16 bytes
//
// Not the best one for general use, but provides bleeding edge performance.
//
class RomuDuoJr {
public:
    using result_type = std::uint64_t;

private:
    std::array<result_type, 2> s{};

public:
    constexpr explicit RomuDuoJr(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit RomuDuoJr(SeedSeq& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 1; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        result_type splitmix64_state = _ensure_nonzero(seed); // RomuDuoJr shouln't be zero-initialized
        this->s[0]                   = _splitmix64(splitmix64_state);
        this->s[1]                   = _splitmix64(splitmix64_state);
        // Like Xoshiro, Romu recommends using SplitMix64 to initialize its state
    }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq& seq) {
        this->s[0] = _seed_seq_to_uint64(seq);
        this->s[1] = _seed_seq_to_uint64(seq);
        // we have to generate multiple std::uint32_t's and then join them into std::uint64_t't to properly initialize
    }

    constexpr result_type operator()() noexcept {
        const result_type res = this->s[0];
        this->s[0]            = 15241094284759029579u * this->s[1];
        this->s[1]            = this->s[1] - res;
        this->s[1]            = _rotl(this->s[1], 27);
        return res;
    }
};

// Implementation of 64-bit Bob Jenkins' small prng ,
// see https://burtleburtle.net/bob/rand/smallprng.html
//     https://gist.github.com/imneme/85cff47d4bad8de6bdeb671f9c76c814
//
// Performance: Excellent
// Quality:     4/5
// State:       32 bytes
//
// Excellent choise as a general purpose PRNG.
// Lack theoretical proof for period length, but brute-force testing indicates >= 2^64.
//
class JSF64 {
public:
    using result_type = std::uint64_t;

private:
    std::array<result_type, 4> s{};

public:
    constexpr explicit JSF64(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit JSF64(SeedSeq& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return std::numeric_limits<result_type>::min(); }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        this->s[0] = 0xf1ea5eed;
        this->s[1] = this->s[2] = this->s[3] = _ensure_nonzero(seed);
        // JSF shouln't struggle with zero-seeding but just in case
        for (int i = 0; i < 20; ++i) static_cast<void>(this->operator()());
    }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq& seq) {
        this->s[0] = _seed_seq_to_uint64(seq);
        this->s[1] = _seed_seq_to_uint64(seq);
        this->s[2] = _seed_seq_to_uint64(seq);
        this->s[3] = _seed_seq_to_uint64(seq);
        // we have to generate multiple std::uint32_t's and then join them into std::uint64_t't to properly initialize
    }

    constexpr result_type operator()() noexcept {
        const result_type e = this->s[0] - _rotl(this->s[1], 7);
        this->s[0]          = this->s[1] ^ _rotl(this->s[2], 13);
        this->s[1]          = this->s[2] + _rotl(this->s[3], 37);
        this->s[2]          = this->s[3] + e;
        this->s[3]          = e + this->s[0];
        return this->s[3];
    }
};

// Implementation of Xoshiro256++ suggested by David Blackman and Sebastiano Vigna,
// see https://prng.di.unimi.it/
//     https://prng.di.unimi.it/xoshiro256plusplus.c
//
// Performance: Excellent
// Quality:     4/5
// State:       32 bytes
//
// Excellent choise as a general purpose PRNG.
// Used by several modern languages as their default.
//
class Xoshiro256PlusPlus {
public:
    using result_type = std::uint64_t;

private:
    std::array<result_type, 4> s{};

public:
    constexpr explicit Xoshiro256PlusPlus(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit Xoshiro256PlusPlus(SeedSeq& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 1; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        result_type splitmix64_state = _ensure_nonzero(seed); // Xoshiro256++ shouln't be zero-initialized
        this->s[0]                   = _splitmix64(splitmix64_state);
        this->s[1]                   = _splitmix64(splitmix64_state);
        this->s[2]                   = _splitmix64(splitmix64_state);
        this->s[3]                   = _splitmix64(splitmix64_state);
    }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq& seq) {
        this->s[0] = _seed_seq_to_uint64(seq);
        this->s[1] = _seed_seq_to_uint64(seq);
        this->s[2] = _seed_seq_to_uint64(seq);
        this->s[3] = _seed_seq_to_uint64(seq);
        // we have to generate multiple std::uint32_t's and then join them into std::uint64_t't to properly initialize
    }

    constexpr result_type operator()() noexcept {
        const result_type result = _rotl(this->s[0] + this->s[3], 23) + this->s[0];
        const result_type t      = this->s[1] << 17;
        this->s[2] ^= this->s[0];
        this->s[3] ^= this->s[1];
        this->s[1] ^= this->s[2];
        this->s[0] ^= this->s[3];
        this->s[2] ^= t;
        this->s[3] = _rotl(this->s[3], 45);
        return result;
    }
};

// Implementation XorShift64* suggested by Marsaglia G. in 2003 "Journal of Statistical Software",
// see https://www.jstatsoft.org/article/view/v008i14
//     https://en.wikipedia.org/wiki/Xorshift
//
// Performance: Good
// Quality:     4/5
// State:       8 bytes
//
// Generally overshadowed by newer PRNGs in quality & performance,
// however it still quite good and seems to be among we can get at just 8 bytes of state.
//
class Xorshift64Star {
public:
    using result_type = std::uint64_t;

private:
    result_type s{};

public:
    constexpr explicit Xorshift64Star(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit Xorshift64Star(SeedSeq& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 1; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) {
        this->s = _ensure_nonzero(seed); // XorShift64* shouln't be zero-initialized
    }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq& seq) {
        this->s = _seed_seq_to_uint32(seq);
    }

    constexpr result_type operator()() noexcept {
        this->s ^= this->s >> 12;
        this->s ^= this->s << 25;
        this->s ^= this->s >> 27;
        return this->s * 0x2545F4914F6CDD1DULL;
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
    a += b, d ^= a, d = _rotl(d, 16);
    c += d, b ^= c, b = _rotl(b, 12);
    a += b, d ^= a, d = _rotl(d, 8);
    c += d, b ^= c, b = _rotl(b, 7);
}

[[nodiscard]] constexpr std::array<std::uint32_t, 16> _chacha20_rounds(const std::array<std::uint32_t, 16>& input) {
    auto state = input;

    constexpr std::size_t round_count = 10;
    // standard number of ChaCha20 rounds as per RFC 7539
    // (20 rounds total, alterating between column rounds and diagonal rounds)

    for (std::size_t i = 0; i < round_count; ++i) {
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

class ChaCha20 {
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
        this->block = _chacha20_rounds(input);
        ++this->counter;
    }

public:
    constexpr explicit ChaCha20(result_type seed = _default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    explicit ChaCha20(SeedSeq& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return std::numeric_limits<result_type>::min(); }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) {
        // Use some other PRNG to setup initial state
        result_type splitmix32_state = _ensure_nonzero(seed); // just in case

        for (auto& e : this->key) e = _splitmix32(splitmix32_state);
        for (auto& e : this->nonce) e = _splitmix32(splitmix32_state);
        this->counter  = 0; // counter can be set to any number, but usually 0 or 1 is used
        this->position = 0;

        this->generate_new_block();
    }

    template <class SeedSeq, _is_seed_seq_enable_if<SeedSeq> = true>
    void seed(SeedSeq& seq) {
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

} // namespace generators

// ===========================
// --- Default global PRNG ---
// ===========================

using default_generator_type = generators::Xoshiro256PlusPlus;
using default_result_type    = default_generator_type::result_type;

inline default_generator_type default_generator;

inline std::seed_seq entropy_seq() {
    // Ensure thread safery of our entropy source, it should generally work fine even without
    // it, but with this we can be sure things never race
    std::mutex                  entropy_mutex;
    std::lock_guard<std::mutex> entropy_guard(entropy_mutex);

    // Hardware entropy (if implemented),
    // some platfroms (mainly MinGW) implements random device as a regular PRNG that
    // doesn't change from run to run, this is horrible, but we can somewhat improve
    // things by mixing other sources of entropy. Since hardware entropy is a rather
    // limited resource we only call it once.
    static std::uint32_t seed_rd          = std::random_device{}();
    // after that we just scramble it with a regular PRNG
    static std::uint32_t splitmix32_state = seed_rd;
    seed_rd                               = _splitmix32(splitmix32_state);

    // Time in nanoseconds (on some platforms microseconds)
    const auto seed_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    // Counter that gets incremented each time
    static std::uint32_t seed_counter = 0;
    ++seed_counter;

    // Note:
    // There are other sources of entropy, such as memory space, heap/stack/function adresses,
    // but those can be rather "constant" on some platforms, using intrinsics could also be good
    // but that isn't portable

    return {seed_rd, _crush_to_uint32(seed_time), seed_counter};
}

inline std::uint32_t entropy() {
    auto seq = entropy_seq();
    return _seed_seq_to_uint32(seq);
    // returns 'std::uint32_t' to mimic the return type of 'std::random_device', if we return uint64_t
    // brace-initializers will complain about narrowing conversion on some generators. If someone want
    // more entropy than that they can always use the whole sequence as a generic solution.
    // Also having one 'random::entropy()' is much nicer than 'random::entropy_32()' & 'random::entropy_64()'.
}

inline void seed(default_result_type random_seed) { default_generator.seed(random_seed); }

inline void seed_with_entropy() {
    auto seq = entropy_seq();
    default_generator.seed(seq);
    // for some god-forsaken reason seeding sequence constructors std:: generators take only l-value sequences
}

// ========================
// --- Random Functions ---
// ========================

// Note 1:
// Despite the intuitive judgement, benchmarks don't seem to indicate that creating
// 'std::uniform_..._distribution<>' on each call introduces any noticeble overhead
//
// sizeof(std::uniform_int_distribution<int>) == 8
// sizeof(std::uniform_real_distribution<double>) == 16

// Note 2:
// No '[[nodiscard]]' since random functions inherently can't be pure due to advancing the generator state.
// Discarding return values while not very sensible, can still be done for the sake of advancing state.
// Ideally we would want users to advance the state directly, but I'm not sure how to communicate that in
// '[[nodiscard]]' warnings.

inline int rand_int(int min, int max) {
    std::uniform_int_distribution<int> distr{min, max};
    return distr(default_generator);
}

inline int rand_uint(unsigned int min, unsigned int max) {
    std::uniform_int_distribution<unsigned int> distr{min, max};
    return distr(default_generator);
}

inline float rand_float() {
    return std::generate_canonical<float, std::numeric_limits<float>::digits>(default_generator);
}

inline float rand_float(float min, float max) {
    std::uniform_real_distribution<float> distr{min, max};
    return distr(default_generator);
}

inline float rand_normal_float() {
    std::normal_distribution<float> distr;
    return distr(default_generator);
}

inline double rand_double() {
    return std::generate_canonical<double, std::numeric_limits<double>::digits>(default_generator);
}

inline double rand_double(double min, double max) {
    std::uniform_real_distribution<double> distr{min, max};
    return distr(default_generator);
}

inline double rand_normal_double() {
    std::normal_distribution<double> distr;
    return distr(default_generator);
}

inline bool rand_bool() { return static_cast<bool>(rand_uint(0, 1)); }

template <class T>
const T& rand_choise(std::initializer_list<T> objects) {
    const int random_index = rand_int(0, static_cast<int>(objects.size()) - 1);
    return objects.begin()[random_index];
}

template <class T>
T rand_linear_combination(const T& A, const T& B) { // random linear combination of 2 colors/vectors/etc
    const auto coef = rand_double();
    return A * coef + B * (1. - coef);
}

} // namespace utl::random

#endif
#endif // module utl::random
