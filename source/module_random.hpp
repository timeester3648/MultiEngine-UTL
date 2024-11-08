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
#include <ctime>            // time()
#include <initializer_list> // initializer_list<>
#include <limits>           // numeric_limits<>::digits
#include <limits>           // numeric_limits<>::min(), numeric_limits<>::max()
#include <random>           // random_device, std::uniform_int_distribution<>,
                            // std::uniform_real_distribution<>, generate_canonical<>

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

// =========================
// --- Random Generators ---
// =========================

class XorShift64StarGenerator {
    // meets uniform random number generator requirements
    // (see https://en.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator)
    //
    // implementation XorShift64* suggested by Marsaglia G. in 2003 "Journal of Statistical Software"
    // (see https://www.jstatsoft.org/article/view/v008i14)
public:
    using result_type = std::uint64_t;

private:
    result_type state;

public:
    XorShift64StarGenerator(result_type seed = 0) : state(seed + 1) {}

    [[nodiscard]] static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
    [[nodiscard]] static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

    void seed(result_type seed) { this->state = seed + 1; } // enforce seed >= 1 by always adding +1 to uint

    result_type operator()() {
        this->state ^= this->state >> 12;
        this->state ^= this->state << 25;
        this->state ^= this->state >> 27;
        return this->state * 0x2545F4914F6CDD1DULL;
    }
};

inline XorShift64StarGenerator xorshift64star;

// ===============
// --- Seeding ---
// ===============

inline void seed(XorShift64StarGenerator::result_type random_seed) { xorshift64star.seed(random_seed); }

inline void seed_with_time() { utl::random::seed(static_cast<XorShift64StarGenerator::result_type>(std::time(NULL))); }

inline void seed_with_random_device() {
    std::random_device rd;
    utl::random::seed(static_cast<XorShift64StarGenerator::result_type>(rd()));
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
    return distr(xorshift64star);
}

inline int rand_uint(unsigned int min, unsigned int max) {
    std::uniform_int_distribution<unsigned int> distr{min, max};
    return distr(xorshift64star);
}

inline float rand_float() { return std::generate_canonical<float, std::numeric_limits<float>::digits>(xorshift64star); }

inline float rand_float(float min, float max) {
    std::uniform_real_distribution<float> distr{min, max};
    return distr(xorshift64star);
}

inline double rand_double() {
    return std::generate_canonical<double, std::numeric_limits<double>::digits>(xorshift64star);
}

inline double rand_double(double min, double max) {
    std::uniform_real_distribution<double> distr{min, max};
    return distr(xorshift64star);
}

inline bool rand_bool() { return static_cast<bool>(xorshift64star() % 2); }

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
