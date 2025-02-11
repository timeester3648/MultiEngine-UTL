// _______________ TEST FRAMEWORK & MODULE  _______________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "test.hpp"

#include "UTL/integral.hpp"

// _______________________ INCLUDES _______________________

// NOTE: STD INCLUDES

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

// ===========================
// --- Bit operation tests ---
// ===========================

using namespace utl;
using namespace utl::integral::literals;
using utl::integral::BigUint;

TEST_CASE("Integer bit operations behave as expected") {
    constexpr auto oooooooo = integral::math::narrow_cast<std::uint8_t>(BigUint("00000000").to_int());
    constexpr auto ooooxxxx = integral::math::narrow_cast<std::uint8_t>(BigUint("00001111").to_int());
    constexpr auto xxooooxx = integral::math::narrow_cast<std::uint8_t>(BigUint("11000011").to_int());

    static_assert(integral::bits::rotl(ooooxxxx, 0) == ooooxxxx);
    static_assert(integral::bits::rotl(ooooxxxx, 6) == xxooooxx);

    static_assert(integral::bits::rotr(ooooxxxx, 0) == ooooxxxx);
    static_assert(integral::bits::rotr(ooooxxxx, 2) == xxooooxx);

    static_assert(integral::bits::bit_width(oooooooo) == 0);
    static_assert(integral::bits::bit_width(ooooxxxx) == 4);
    static_assert(integral::bits::bit_width(xxooooxx) == 8);
    
    static_assert(integral::bits::bit_width(0u) == 0);
    static_assert(integral::bits::bit_width(1u) == 1);
    static_assert(integral::bits::bit_width(2u) == 2);
    static_assert(integral::bits::bit_width(3u) == 2);
    static_assert(integral::bits::bit_width(4u) == 3);
}

// ===========================
// --- Integral math tests ---
// ===========================

TEST_CASE("Integral math behaves as expected") {
    static_assert(integral::math::sign(7) == 1);
    static_assert(integral::math::sign(1) == 1);
    static_assert(integral::math::sign(0) == 0);
    static_assert(integral::math::sign(-1) == -1);
    static_assert(integral::math::sign(-7) == -1);
    
    static_assert(integral::math::sign_product(0, 0) == 0);
    static_assert(integral::math::sign_product(4, 0) == 0);
    static_assert(integral::math::sign_product(0, 4) == 0);
    static_assert(integral::math::sign_product(-4, 0) == 0);
    static_assert(integral::math::sign_product(0, -4) == 0);
    
    static_assert(integral::math::sign_product(4, 4) == 1);
    static_assert(integral::math::sign_product(-4, -4) == 1);
    
    static_assert(integral::math::sign_product(-4, 3) == -1);
    static_assert(integral::math::sign_product(3, -4) == -1);

    static_assert(integral::math::saturated_add(255_u8, 1_u8) == 255_u8);
    static_assert(integral::math::saturated_add(2_u8, 3_u8) == 5_u8);

    static_assert(integral::math::saturated_substract(0_u8, 1_u8) == 0_u8);
    static_assert(integral::math::saturated_substract(3_u8, 2_u8) == 1_u8);

    static_assert(integral::math::divide_ceil(6, 3) == 2);
    static_assert(integral::math::divide_ceil(5, 3) == 2);
    static_assert(integral::math::divide_ceil(4, 3) == 2);
    static_assert(integral::math::divide_ceil(3, 3) == 1);
    
    static_assert(integral::math::divide_ceil(-6, 3) == -2);
    static_assert(integral::math::divide_ceil(-5, 3) == -1);
    static_assert(integral::math::divide_ceil(-4, 3) == -1);
    static_assert(integral::math::divide_ceil(-3, 3) == -1);
    
    static_assert(integral::math::divide_ceil(6, -3) == -2);
    static_assert(integral::math::divide_ceil(5, -3) == -1);
    static_assert(integral::math::divide_ceil(4, -3) == -1);
    static_assert(integral::math::divide_ceil(3, -3) == -1);
    
    static_assert(integral::math::divide_ceil(-6, -3) == 2);
    static_assert(integral::math::divide_ceil(-5, -3) == 2);
    static_assert(integral::math::divide_ceil(-4, -3) == 2);
    static_assert(integral::math::divide_ceil(-3, -3) == 1);
    
    static_assert(integral::math::divide_floor(6, 3) == 2);
    static_assert(integral::math::divide_floor(5, 3) == 1);
    static_assert(integral::math::divide_floor(4, 3) == 1);
    static_assert(integral::math::divide_floor(3, 3) == 1);
    
    static_assert(integral::math::divide_floor(-6, 3) == -2);
    static_assert(integral::math::divide_floor(-5, 3) == -2);
    static_assert(integral::math::divide_floor(-4, 3) == -2);
    static_assert(integral::math::divide_floor(-3, 3) == -1);
    
    static_assert(integral::math::divide_floor(6, -3) == -2);
    static_assert(integral::math::divide_floor(5, -3) == -2);
    static_assert(integral::math::divide_floor(4, -3) == -2);
    static_assert(integral::math::divide_floor(3, -3) == -1);
    
    static_assert(integral::math::divide_floor(-6, -3) == 2);
    static_assert(integral::math::divide_floor(-5, -3) == 1);
    static_assert(integral::math::divide_floor(-4, -3) == 1);
    static_assert(integral::math::divide_floor(-3, -3) == 1);
}

// =====================
// --- BigUint tests ---
// =====================

TEST_CASE("BigUint getters behave as expected") {
    static_assert(BigUint("0001").to_int() == 1);
    static_assert(BigUint("0010").to_int() == 2);
    static_assert(BigUint("0100").to_int() == 4);
    static_assert(BigUint("1000").to_int() == 8);
    static_assert(BigUint("1011").to_int() == 11);
    static_assert(BigUint("1101").to_int() == 13);

    static_assert(BigUint("1011").significant_bits() == 4);
    static_assert(BigUint("0100").significant_bits() == 3);
    static_assert(BigUint("0111").significant_bits() == 3);
    static_assert(BigUint("0011").significant_bits() == 2);
    static_assert(BigUint("0001").significant_bits() == 1);
    static_assert(BigUint("0000").significant_bits() == 0);

    constexpr auto xoxx = BigUint("1011");

    static_assert(xoxx.bit_get(0) == true);
    static_assert(xoxx.bit_get(1) == true);
    static_assert(xoxx.bit_get(2) == false);
    static_assert(xoxx.bit_get(3) == true);
    static_assert(xoxx.bit_get(4) == false);
    static_assert(xoxx.bit_get(5) == false);
    static_assert(xoxx.bit_get(6) == false);
    static_assert(xoxx.bit_get(7) == false);
    static_assert(xoxx.bit_get(8) == false);
}

TEST_CASE("BigUint arithmetic comparison behaves as expected") {
    static_assert(BigUint("0001") == BigUint("0001"));
    static_assert(BigUint("1001") == BigUint("1001"));
    static_assert(BigUint("1001") != BigUint("1101"));
    static_assert(BigUint("1011") != BigUint("1101"));

    static_assert(BigUint(1) < BigUint(2));
    static_assert(BigUint(0) < BigUint(124));
    static_assert(BigUint(17) < BigUint(18));
    static_assert(BigUint(17) < BigUint(17) == false);
    static_assert(BigUint(17) < BigUint(12) == false);

    static_assert(BigUint(1) <= BigUint(2));
    static_assert(BigUint(0) <= BigUint(124));
    static_assert(BigUint(17) <= BigUint(18));
    static_assert(BigUint(17) <= BigUint(17));
    static_assert(BigUint(17) <= BigUint(12) == false);
}

TEST_CASE("BigUint bit-wise operations behave as expected") {
    static_assert((BigUint("0011") << 0) == BigUint("0011"));
    static_assert((BigUint("0010") << 1) == BigUint("0100"));
    static_assert((BigUint("0001") << 1) == BigUint("0010"));
    static_assert((BigUint("0011") << 1) == BigUint("0110"));
    static_assert((BigUint("0011") << 2) == BigUint("1100"));
    // [!] This will not work as expected since 'size' < 'bits'
    // static_assert((BigUint("0011") << 3) == BigUint("1000"));

    // 80-bit tests
    constexpr auto r1 = BigUint("00000000000000000000000000000000000000000000000000000000000000000000000000000000");
    constexpr auto r2 = BigUint("11111111111111111111111111111111111111111111111111111111111111111111111111111111");
    constexpr auto r3 = BigUint("00000000000000000000000000000000000000000000000000000000000000000000000000000001");

    static_assert((r2 >> 80) == r1);
    static_assert((r2 >> 79) == r3);

    static_assert(r1 < r2);
    static_assert(r1 < r3);
    static_assert(r3 <= r3);

    static_assert((BigUint("1110") >> 0) == BigUint("1110"));
    static_assert((BigUint("0010") >> 1) == BigUint("0001"));
    static_assert((BigUint("0001") >> 1) == BigUint("0000"));
    static_assert((BigUint("0011") >> 1) == BigUint("0001"));
    static_assert((BigUint("0011") >> 2) == BigUint("0000"));
    static_assert((BigUint("1110") >> 3) == BigUint("0001"));

    static_assert((BigUint("0001") & BigUint("0001")) == BigUint("0001"));
    static_assert((BigUint("1011") & BigUint("1110")) == BigUint("1010"));
    static_assert((BigUint("1001") & BigUint("1101")) == BigUint("1001"));
    static_assert((BigUint("1101") & BigUint("0010")) == BigUint("0000"));

    static_assert((BigUint("0001") | BigUint("0001")) == BigUint("0001"));
    static_assert((BigUint("1011") | BigUint("1110")) == BigUint("1111"));
    static_assert((BigUint("1001") | BigUint("1101")) == BigUint("1101"));
    static_assert((BigUint("0000") | BigUint("0000")) == BigUint("0000"));

    static_assert((BigUint("0001") ^ BigUint("0001")) == BigUint("0000"));
    static_assert((BigUint("1011") ^ BigUint("1110")) == BigUint("0101"));
    static_assert((BigUint("1001") ^ BigUint("1101")) == BigUint("0100"));
    static_assert((BigUint("0000") ^ BigUint("0000")) == BigUint("0000"));

    static_assert((~BigUint(7)) == BigUint(~7));
    static_assert((~BigUint(0)) == BigUint(~0));
    static_assert((~BigUint(7)) == BigUint(~7));
}

TEST_CASE("BigUint arithmetic operations behave as expected") {
    static_assert(BigUint(0) + BigUint(0) == BigUint(0 + 0));
    static_assert(BigUint(0) + BigUint(4) == BigUint(0 + 4));
    static_assert(BigUint(8) + BigUint(0) == BigUint(8 + 0));
    static_assert(BigUint(8) + BigUint(4) == BigUint(8 + 4));
    static_assert(BigUint(14) + BigUint(7) == BigUint(14 + 7));
    static_assert(BigUint(872) + BigUint(6312) == BigUint(872 + 6312));

    static_assert(BigUint(0) - BigUint(0) == BigUint(0 - 0));
    static_assert(BigUint(8) - BigUint(0) == BigUint(8 - 0));
    static_assert(BigUint(8) - BigUint(4) == BigUint(8 - 4));
    static_assert(BigUint(8) - BigUint(8) == BigUint(8 - 8));
    static_assert(BigUint(14) - BigUint(7) == BigUint(14 - 7));
    static_assert(BigUint(6312) - BigUint(872) == BigUint(6312 - 872));

    static_assert(BigUint(0) * BigUint(0) == BigUint(0 * 0));
    static_assert(BigUint(0) * BigUint(4) == BigUint(0 * 4));
    static_assert(BigUint(8) * BigUint(0) == BigUint(8 * 0));
    static_assert(BigUint(8) * BigUint(4) == BigUint(8 * 4));
    static_assert(BigUint(14) * BigUint(7) == BigUint(14 * 7));
    static_assert(BigUint(872) * BigUint(6312) == BigUint(872 * 6312));

    static_assert(BigUint(0) / BigUint(4) == BigUint(0 / 4));
    static_assert(BigUint(2) / BigUint(4) == BigUint(2 / 4));
    static_assert(BigUint(8) / BigUint(4) == BigUint(8 / 4));
    static_assert(BigUint(12) / BigUint(4) == BigUint(12 / 4));
    static_assert(BigUint(6421232) / BigUint(41237) == BigUint(6421232 / 41237));

    static_assert(BigUint(0) % BigUint(4) == BigUint(0 % 4));
    static_assert(BigUint(2) % BigUint(4) == BigUint(2 % 4));
    static_assert(BigUint(8) % BigUint(4) == BigUint(8 % 4));
    static_assert(BigUint(12) % BigUint(4) == BigUint(12 % 4));
    static_assert(BigUint(6421232) % BigUint(41237) == BigUint(6421232 % 41237));

    // Overflow tests
    using word_type         = BigUint<64>::word_type;
    constexpr auto word_max = std::numeric_limits<word_type>::max();

    static_assert(BigUint(word_max) + BigUint(2) == BigUint(1)); // overflows like a regular uint
    static_assert(BigUint(0) - BigUint(1) == BigUint(word_max)); // underflows like a regulara uint
}