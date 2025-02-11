// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::integral
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_integral.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <climits>
#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_INTEGRAL)
#ifndef UTLHEADERGUARD_INTEGRAL
#define UTLHEADERGUARD_INTEGRAL

// _______________________ INCLUDES _______________________

#include <array>       // array<>
#include <cassert>     // assert()
#include <climits>     // CHAR_BIT
#include <cstddef>     // size_t
#include <cstdint>     // uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t
#include <exception>   //
#include <limits>      // numeric_limits<>::digits, numeric_limits<>::min(), numeric_limits<>::max()
#include <stdexcept>   // std::domain_error
#include <string>      // string, to_string()
#include <type_traits> // enable_if_t<>, is_integral_v<>, is_unsigned_v<>, make_unsigned_t<>

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

namespace utl::integral {

// --- Implementation utils ---
// ----------------------------

template <bool Cond>
using _require = std::enable_if_t<Cond, bool>; // makes SFINAE a bit less cumbersome

template <class T>
using _require_integral = _require<std::is_integral_v<T>>;

template <class T>
using _require_uint = _require<std::is_integral_v<T> && std::is_unsigned_v<T>>;

using _ull = unsigned long long;

// --- Bit twiddling ---
// ---------------------

using bit_type = bool;

template <class T>
constexpr std::size_t bit_sizeof = sizeof(T) * CHAR_BIT;

// Note:
// With C++20 following functions will be added into 'std::':
// - bit_width()
// - rotl()
// - rotr()

namespace bits {

// Get individual bits,
// undefined behavior if 'bit >= bit_sizeof<T>'
template <class T, _require_integral<T> = true>
[[nodiscard]] constexpr T get(T value, std::size_t bit) noexcept {
    assert(bit < bit_sizeof<T>);
    return static_cast<bit_type>((value >> bit) & T(1));
}

// Set individual bits,
// undefined behavior if 'bit >= bit_sizeof<T>'
template <class T, _require_integral<T> = true>
constexpr void set(T& value, std::size_t bit, bit_type state) noexcept {
    assert(bit < bit_sizeof<T>);
    value |= (T(state) << bit);
}

template <class T, _require_uint<T> = true>
[[nodiscard]] constexpr std::size_t bit_width(T value) noexcept {
    std::size_t count = 0;
    while (value) ++count, value >>= 1;
    return count;
}

// Circular left rotate,
// undefined behavior if 'shift >= bit_sizeof<T>'
template <class T, _require_integral<T> = true>
[[nodiscard]] constexpr T rotl(T value, std::size_t shift) noexcept {
    assert(shift < bit_sizeof<T>);
    return (value << shift) | (value >> (std::numeric_limits<T>::digits - shift));
}

// Circular right rotate,
// undefined behavior if 'shift >= bit_sizeof<T>'
template <class T, _require_integral<T> = true>
[[nodiscard]] constexpr T rotr(T value, std::size_t shift) noexcept {
    assert(shift < bit_sizeof<T>);
    return (value << (std::numeric_limits<T>::digits - shift)) | (value >> shift);
}

} // namespace bits

// --- Integral math functions ---
// -------------------------------

// Note:
// With C++20 following functions will be added into 'std::':
// - cmp_equal()
// - cmp_not_equal
// - cmp_less
// - cmp_greater
// - cmp_less_equal
// - cmp_greater_equal

using sign_type = int;

namespace math {

// {-1, 0, 1} variation of sign()
template <class T>
[[nodiscard]] constexpr sign_type sign(T value) noexcept {
    return (value > 0) ? 1 : (value == 0) ? 0 : -1;
}

template <class T>
[[nodiscard]] constexpr sign_type sign_product(T lhs, T rhs) noexcept {
    if (lhs == T(0) || rhs == T(0)) return 0;
    if ((lhs < T(0) && rhs < T(0)) || (lhs > T(0) && rhs > T(0))) return 1;
    return -1;
}

// ceil(lhs / rhs)
template <class T, _require_integral<T> = true>
[[nodiscard]] constexpr T divide_ceil(T dividend, T divisor) noexcept {
    assert(divisor != T(0));

    const bool quotient_positive = (dividend < T(0)) == (divisor < T(0));
    return dividend / divisor + (dividend % divisor != T(0) && quotient_positive);
}

// floor(lhs / rhs)
template <class T, _require_integral<T> = true>
[[nodiscard]] constexpr T divide_floor(T dividend, T divisor) noexcept {
    assert(divisor != T(0));

    const bool quotient_negative = (dividend < T(0)) != (divisor < T(0));
    return dividend / divisor - (dividend % divisor != T(0) && quotient_negative);
}

template <class T, _require_integral<T> = true>
[[nodiscard]] constexpr bool addition_overflows(T lhs, T rhs) noexcept {
    if (rhs > T(0) && lhs > std::numeric_limits<T>::max() - rhs) return false;
    if (rhs < T(0) && lhs < std::numeric_limits<T>::min() - rhs) return false;
    return true;
}

template <class T, _require_integral<T> = true>
[[nodiscard]] constexpr bool substraction_underflows(T lhs, T rhs) noexcept {
    if (rhs < T(0) && lhs > std::numeric_limits<T>::max() + rhs) return false;
    if (rhs > T(0) && lhs < std::numeric_limits<T>::min() + rhs) return false;
    return true;
}

template <class T, _require_integral<T> = true>
[[nodiscard]] constexpr T saturated_add(T lhs, T rhs) noexcept {
    if (rhs > T(0) && lhs > std::numeric_limits<T>::max() - rhs) return std::numeric_limits<T>::max();
    if (rhs < T(0) && lhs < std::numeric_limits<T>::min() - rhs) return std::numeric_limits<T>::min();
    return lhs + rhs;
}

template <class T, _require_integral<T> = true>
[[nodiscard]] constexpr T saturated_substract(T lhs, T rhs) noexcept {
    if (rhs < T(0) && lhs > std::numeric_limits<T>::max() + rhs) return std::numeric_limits<T>::max();
    if (rhs > T(0) && lhs < std::numeric_limits<T>::min() + rhs) return std::numeric_limits<T>::min();
    return lhs - rhs;
}

// Integer comparators that properly handle differently signed integers
template <class T1, class T2>
[[nodiscard]] constexpr bool cmp_equal(T1 lhs, T2 rhs) noexcept {
    if constexpr (std::is_signed_v<T1> == std::is_signed_v<T2>) return lhs == rhs;
    else if constexpr (std::is_signed_v<T1>) return lhs >= 0 && std::make_unsigned_t<T1>(lhs) == rhs;
    else return rhs >= 0 && std::make_unsigned_t<T2>(rhs) == lhs;
}

template <class T1, class T2>
[[nodiscard]] constexpr bool cmp_not_equal(T1 lhs, T2 rhs) noexcept {
    return !cmp_equal(lhs, rhs);
}

template <class T1, class T2>
[[nodiscard]] constexpr bool cmp_less(T1 lhs, T2 rhs) noexcept {
    if constexpr (std::is_signed_v<T1> == std::is_signed_v<T2>) return lhs < rhs;
    else if constexpr (std::is_signed_v<T1>) return lhs < 0 || std::make_unsigned_t<T1>(lhs) < rhs;
    else return rhs >= 0 && lhs < std::make_unsigned_t<T2>(rhs);
}

template <class T1, class T2>
[[nodiscard]] constexpr bool cmp_greater(T1 lhs, T2 rhs) noexcept {
    return cmp_less(rhs, lhs);
}

template <class T1, class T2>
[[nodiscard]] constexpr bool cmp_less_equal(T1 lhs, T2 rhs) noexcept {
    return !cmp_less(rhs, lhs);
}

template <class T1, class T2>
[[nodiscard]] constexpr bool cmp_greater_equal(T1 lhs, T2 rhs) noexcept {
    return !cmp_less(lhs, rhs);
}

// Returns if 'value' is in range of type 'To'
template <class To, class From>
[[nodiscard]] constexpr bool in_range(From value) noexcept {
    return cmp_greater_equal(value, std::numeric_limits<To>::min()) &&
           cmp_less_equal(value, std::numeric_limits<To>::max());
}

// Integer-to-integer cast that throws if conversion overflow/underflows the result,
// no '[[nodiscard]]' because cast may be used for the side effect of throwing
template <class To, class From, _require_integral<To> = true, _require_integral<From> = true>
constexpr To narrow_cast(From value) {
    if (!in_range<To>(value)) throw std::domain_error("narrow_cast() overflows the result.");
    return static_cast<To>(value);
}

// Utility used to reverse indexation logic, mostly useful when working with unsigned indeces
template <class T, _require_integral<T> = true>
[[nodiscard]] constexpr T reverse_idx(T idx, T size) noexcept {
    return size - T(1) - idx;
}

} // namespace math

// --- Literals ---
// ----------------

namespace literals {

// Literals for all fixed-size and commonly used integer types, 'narrow_cast()'
// ensures there is no overflow during initialization from 'unsigned long long'
// clang-format off
[[nodiscard]] constexpr auto operator"" _i8( _ull v) noexcept { return math::narrow_cast<std::int8_t  >(v); }
[[nodiscard]] constexpr auto operator"" _u8( _ull v) noexcept { return math::narrow_cast<std::uint8_t >(v); }
[[nodiscard]] constexpr auto operator"" _i16(_ull v) noexcept { return math::narrow_cast<std::int16_t >(v); }
[[nodiscard]] constexpr auto operator"" _u16(_ull v) noexcept { return math::narrow_cast<std::uint16_t>(v); }
[[nodiscard]] constexpr auto operator"" _i32(_ull v) noexcept { return math::narrow_cast<std::int32_t >(v); }
[[nodiscard]] constexpr auto operator"" _u32(_ull v) noexcept { return math::narrow_cast<std::uint32_t>(v); }
[[nodiscard]] constexpr auto operator"" _i64(_ull v) noexcept { return math::narrow_cast<std::int64_t >(v); }
[[nodiscard]] constexpr auto operator"" _u64(_ull v) noexcept { return math::narrow_cast<std::uint64_t>(v); }
[[nodiscard]] constexpr auto operator"" _sz( _ull v) noexcept { return math::narrow_cast<std::size_t  >(v); }
// clang-format on

} // namespace literals

// --- Big int ---
// ---------------

// 'Bit' int emulates an integer type sufficiently large to represent <SIZE> bits,
// highly advised to use with 'SIZE' in multiples of '64'

template <std::size_t bits_to_fit>
struct BigUint {
    using self                             = BigUint;
    using word_type                        = std::uint64_t;
    constexpr static std::size_t size      = bits_to_fit;
    constexpr static std::size_t word_size = std::numeric_limits<word_type>::digits;
    constexpr static std::size_t words     = math::divide_ceil(size, word_size);
    constexpr static std::size_t bits      = words * word_size;
    using storage_type                     = std::array<word_type, words>;

    storage_type s{};

    constexpr BigUint()                    = default;
    constexpr BigUint(const self&)         = default;
    constexpr BigUint(self&&)              = default;
    constexpr self& operator=(const self&) = default;
    constexpr self& operator=(self&&)      = default;

    constexpr explicit BigUint(word_type number) noexcept { this->word(0) = number; }

    template <std::size_t chars>
    constexpr explicit BigUint(const char (&str)[chars]) noexcept {
        for (std::size_t i = 0; i < self::size; ++i)
            this->bit_set(math::reverse_idx(i, self::size), str[i] == '0' ? false : true);
    }

    // --- Getters ---
    // ---------------

    constexpr word_type& word(std::size_t idx) noexcept {
        assert(idx < self::words);
        return this->s[idx];
    }
    constexpr const word_type& word(std::size_t idx) const noexcept {
        assert(idx < self::words);
        return this->s[idx];
    }
    constexpr bit_type bit_get(std::size_t bit) const noexcept {
        assert(bit < self::bits);
        const std::size_t word_idx = bit / self::word_size;
        const std::size_t bit_idx  = bit % self::word_size;
        return bits::get(this->word(word_idx), bit_idx);
    }
    constexpr void bit_set(std::size_t bit, bit_type value) noexcept {
        assert(bit < self::bits);
        const std::size_t word_idx = bit / self::word_size;
        const std::size_t bit_idx  = bit % self::word_size;
        return bits::set(this->word(word_idx), bit_idx, value);
    }
    constexpr std::size_t significant_bits() const noexcept {
        for (std::size_t i = 0; i < self::words; ++i) {
            const std::size_t reverse_i     = math::reverse_idx(i, self::words);
            const std::size_t word_sig_bits = bits::bit_width(this->word(reverse_i));
            if (word_sig_bits != 0) return i * self::word_size + word_sig_bits;
        }
        return 0;
    }

    constexpr explicit operator bool() const noexcept {
        for (const auto& e : this->s)
            if (e != 0) return true;
        return false;
    }

    // --- Bit-wise operators ---
    // --------------------------

    constexpr self operator<<(std::size_t shift) const noexcept {
        if (shift == 0) return *this;

        // Implementation based on libstd++ 'std::bitset' l-shift operator
        const std::size_t wshift = shift / self::word_size;
        const std::size_t offset = shift % self::word_size;
        self              res    = *this;

        if (offset == 0) {
            for (std::size_t i = self::words - 1; i >= wshift; --i) res.word(i) = res.word(i - wshift);
        } else {
            const std::size_t suboffset = self::word_size - offset;
            for (std::size_t i = self::words - 1; i > wshift; --i)
                res.word(i) = ((res.word(i - wshift) << offset) | (res.word(i - wshift - 1) >> suboffset));
            res.word(wshift) = res.word(0) << offset;
        }

        // Zero-fill shifted-from region
        for (std::size_t i = 0; i < wshift; ++i) res.word(i) = word_type(0);

        // [?] Zero-fill shifted-to region outside 'size'

        return res;
    }

    constexpr self operator>>(std::size_t shift) const noexcept {
        if (shift == 0) return *this;

        // Implementation based on libstd++ 'std::bitset' r-shift operator
        const std::size_t wshift = shift / self::word_size;
        const std::size_t offset = shift % self::word_size;
        const std::size_t limit  = self::words - wshift - 1;
        self              res    = *this;

        if (offset == 0) {
            for (std::size_t i = 0; i <= limit; ++i) res.word(i) = res.word(i + wshift);
        } else {
            const std::size_t suboffset = self::word_size - offset;
            for (std::size_t i = 0; i < limit; ++i)
                res.word(i) = ((res.word(i + wshift) >> offset) | (res.word(i + wshift + 1) << suboffset));
            res.word(limit) = res.word(self::words - 1) >> offset;
        }

        // Zero-fill shifted-from region
        for (std::size_t i = limit + 1; i < self::words; ++i) res.word(i) = word_type(0);

        return res;
    }

    constexpr self operator&(const self& other) const noexcept {
        self res = *this;
        for (std::size_t i = 0; i < self::words; ++i) res.word(i) &= other.word(i);
        return res;
    }
    constexpr self operator|(const self& other) const noexcept {
        self res = *this;
        for (std::size_t i = 0; i < self::words; ++i) res.word(i) |= other.word(i);
        return res;
    }
    constexpr self operator^(const self& other) const noexcept {
        self res = *this;
        for (std::size_t i = 0; i < self::words; ++i) res.word(i) ^= other.word(i);
        return res;
    }
    constexpr self operator~() const noexcept {
        self res = *this;
        for (auto& e : res.s) e = ~e;
        return res;
    };

    // --- Arithmetic operators ---
    // ----------------------------

    constexpr self operator+(const self& other) const noexcept {
        self res   = *this ^ other;
        self carry = *this & other;
        while (carry) {
            self shifted_carry = carry << 1;
            carry              = res & shifted_carry;
            res ^= shifted_carry;
        }
        return res;
    }

    constexpr self operator-(const self& other) const noexcept {
        self x = *this;
        self y = other;
        return x + (~y + self(1));
    }

    constexpr self operator*(const self& other) const noexcept {
        self x = *this;
        self y = other;
        self res{};
        while (y) {
            if (y.bit_get(0)) res += x;
            x <<= 1;
            y >>= 1;
        }
        return res;
    }

    constexpr std::pair<self, self> long_divide(const self& other) const noexcept {
        assert(other); // prevent division by zero

        // Standard long division algorithm from [https://en.wikipedia.org/wiki/Division_algorithm]
        const self& numerator   = *this;
        self        denominator = other;
        self        quotient{};
        self        remainder{};
        std::size_t sig_bits = numerator.significant_bits();

        for (std::size_t i = 0; i < sig_bits; ++i) {
            remainder <<= 1;
            remainder.bit_set(0, numerator.bit_get(math::reverse_idx(i, sig_bits)));
            if (remainder >= denominator) {
                remainder -= denominator;
                quotient.bit_set(math::reverse_idx(i, sig_bits), true);
            }
        }

        return {quotient, remainder};
    }

    constexpr self operator/(const self& other) const noexcept { return this->long_divide(other).first; }
    constexpr self operator%(const self& other) const noexcept { return this->long_divide(other).second; }

    // --- Augmented assignment ---
    // ----------------------------

    constexpr self& operator<<=(std::size_t shift) { return *this = (*this << shift); }
    constexpr self& operator>>=(std::size_t shift) { return *this = (*this >> shift); }
    constexpr self& operator&=(const self& other) { return *this = (*this & other); }
    constexpr self& operator|=(const self& other) { return *this = (*this | other); }
    constexpr self& operator^=(const self& other) { return *this = (*this ^ other); }
    constexpr self& operator+=(const self& other) { return *this = (*this + other); }
    constexpr self& operator-=(const self& other) { return *this = (*this - other); }
    constexpr self& operator*=(const self& other) { return *this = (*this * other); }
    constexpr self& operator/=(const self& other) { return *this = (*this / other); }
    constexpr self& operator%=(const self& other) { return *this = (*this % other); }

    // --- Comparison ---
    // ------------------

    constexpr bool operator==(const self& other) const noexcept {
        for (std::size_t i = 0; i < self::words; ++i)
            if (this->word(i) != other.word(i)) return false;
        return true;
    }
    constexpr bool operator<=(const self& other) const noexcept {
        // Compare lexicographically from highest to lowest bits
        for (std::size_t i = 0; i < self::words; ++i)
            if (this->word(i) <= other.word(i)) return true;
        return false;
    }
    constexpr bool operator<(const self& other) const noexcept {
        // Compare lexicographically from highest to lowest bits
        for (std::size_t i = 0; i < self::words; ++i)
            if (this->word(i) < other.word(i)) return true;
        return false;
    }

    constexpr bool operator!=(const self& other) const noexcept { return !(*this == other); }
    constexpr bool operator>=(const self& other) const noexcept { return !(*this < other); }
    constexpr bool operator>(const self& other) const noexcept { return !(*this <= other); }

    // --- Serialization ---
    // ---------------------

    constexpr word_type to_int() const noexcept {
        assert(self::words <= 1);
        return this->s.front();
    }

    template <bool prettify>
    std::string to_string() const {
        constexpr auto color_red       = "\033[31m";
        constexpr auto color_blue      = "\033[34m";
        constexpr auto color_green     = "\033[32m";
        constexpr auto color_magenta   = "\033[35m";
        constexpr auto color_bold_gray = "\033[90;1m";
        constexpr auto color_reset     = "\033[0m";

        std::string str;

        if constexpr (prettify) {
            str += color_green;
            str += "BigInt<";
            str += std::to_string(self::size);
            str += ">";
            str += color_reset;
        }

        if constexpr (prettify) str += color_bold_gray;
        str += "[";
        if constexpr (prettify) str += color_reset;

        for (std::size_t i = 0; i < self::bits; ++i) {
            int bit = this->bit_get(math::reverse_idx(i, self::bits));
            if constexpr (prettify) str += bit ? color_red : color_blue;
            str += std::to_string(bit);
            if constexpr (prettify) str += color_reset;
        }

        if constexpr (prettify) str += color_bold_gray;
        str += "]";
        if constexpr (prettify) str += color_reset;

        if constexpr (prettify) {
            str += color_magenta;
            str += "(";
            str += std::to_string(this->significant_bits());
            str += " sig. bits)";
            str += "( words: ";
            for (const auto& e : s) (str += std::to_string(e)) += " ";
            str += ")";
            str += color_reset;
        }

        return str;
    }
};

// CTAD so we can deduce size from integer & string literals
BigUint(BigUint<0>::word_type)->BigUint<BigUint<0>::word_size>;

template <std::size_t chars>
BigUint(const char (&str)[chars]) -> BigUint<chars - 1>;

} // namespace utl::integral

#endif
#endif // module utl::integral
