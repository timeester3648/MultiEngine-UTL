# utl::integral

[<- to README.md](..)

[<- to implementation.hpp](../include/UTL/integral.hpp)

**integral** module implements various utilities for that deal with integer types & bit twiddling. Main highlights are:

- Integer bit operations
- Integer math
- Fixed-size literals
- Arbitrary precision integers / bitsets

Such functionality is mostly useful in `constexpr` context when dealing with optimizations such as compile-time creation of bit-masks, bit-buffers, computation of different alignments, coefficients and etc. `BigUint<>` also serves as a "better" alternative to `std::bitset<>`.

> [!Note]
> Significant part of this module gets added into the standard library with **C++20** `<bits>` and `<utility>`

## Definitions

```cpp
// Integer bit operations
using bit_type = bool;

namespace bits {
    template <class T> constexpr T    get(T  value, int bit)                 noexcept;
    template <class T> constexpr void set(T& value, int bit, bit_type state) noexcept;

    template <class T> constexpr std::size_t bit_width(T value) noexcept;

    template <class T> constexpr T rotl(T value, int shift) noexcept;
    template <class T> constexpr T rotr(T value, int shift) noexcept;
}

// Integer math
namespace math {
template <class T> constexpr T int_divide_ceil( T numerator, T denominator) noexcept;
template <class T> constexpr T int_divide_floor(T numerator, T denominator) noexcept;
    
template <class T> constexpr bool addition_overflows(     T lhs, T rhs) noexcept;
template <class T> constexpr bool substraction_underflows(T lhs, T rhs) noexcept;
    
template <class T> constexpr T clamped_add(      T lhs, T rhs) noexcept;
template <class T> constexpr T clamped_substract(T lhs, T rhs) noexcept;
    
template <class T1, class T2> constexpr bool cmp_equal(        T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_not_equal(    T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_less(         T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_greater(      T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_less_equal(   T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_greater_equal(T1 lhs, T2 rhs) noexcept;
    
template <class To, class From> constexpr bool in_range(From value) noexcept;
    
template <class To, class From> constexpr To narrow_cast(From value);
    
template <class T> constexpr T reverse_idx(T idx, T size) noexcept;
}

// Integer literals
namespace literals {
    constexpr std::int8_t   operator"" _i8( unsigned long long v) noexcept;
    constexpr std::uint8_t  operator"" _u8( unsigned long long v) noexcept;
    constexpr std::uint16_t operator"" _i16(unsigned long long v) noexcept;
    constexpr std::int16_t  operator"" _u16(unsigned long long v) noexcept;
    constexpr std::int32_t  operator"" _i32(unsigned long long v) noexcept;
    constexpr std::uint32_t operator"" _u32(unsigned long long v) noexcept;
    constexpr std::int64_t  operator"" _i64(unsigned long long v) noexcept;
    constexpr std::uint64_t operator"" _u64(unsigned long long v) noexcept;
    constexpr std::size_t   operator"" _sz( unsigned long long v) noexcept;
}

// Arbitrary sized integers & bitsets
template <std::size_t bits_to_fit>
struct BigUint {
    using word_type = std::uint64_t;
    
    constexpr static std::size_t word_size;
   	constexpr static std::size_t words;
   	constexpr static std::size_t bits;
    
    constexpr explicit BigUint(word_type number) noexcept;
};
```

## Methods

> ```cpp
> DEFINITION:
> ```

DESCRIPTION:

## Examples

### EXAMPLENAME:

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGe1wAyeAyYAHI%2BAEaYxCAArKQADqgKhE4MHt6%2BekkpjgJBIeEsUTHxdpgOaUIETMQEGT5%2BXLaY9nkM1bUEBWGR0XG2NXUNWc0KQ93BvcX9sQCUtqhexMjsHOYAzMHI3lgA1CYbbggEBAkKIAD0l8RMAO4AdMCECF4RXkorsowED2gslwAIixCMQ8BZUMB0IZUAA3S5yAAqAUuLCY42ilxSRnoAH1trtMAjkQ8EAkEodsCYNABBak04IEPZo4IQOYHADsVlpe15ew%2BwWAe2YbAUCSYq35BFoh25dJ5fLQDAxqgSxD2jL2xEwCUwTAICgOG0BewAbLL6Xy9kqVWq9uglhF6HsFPQdbj0Mt9WlcSxDYcTVxTQ9TRyLbTLXzxugQCglkzDm5EwczGYhG6EoK9vx1eYzEak0cXRmPV72r7/Udk3nmQoHiZYm4GHnw/KaVbo7G0F4E1Wiy3G8qM7GxcFaKhkABrNkgBtNlsbOVWnN7CCavBGk0aWUagtanV6g076zWPDskxcyNW3lMHuoF0TTcPmMoBDEASoEcEPXoACeuJ2CdJ1jBhUDuNlWyva9XUwHUR0zBhxynCAYPdT1bnLP05kghVrz2W8iD2Vwn07V931Ar8f3/QCpxAsCIMXKCOwIF9u17Qs3GfLs3w/WN0O9AQAPRAhE1I5AeIokBmFApQlXQBRKQgYiAFoHy6OY/iWQQ2T2S49i4TBTQLatU1rOdm1TVsrQvQF6SgsT42M/tLMHVCEljBBfwiMF0BncyFyXPkVzXQRdwDPZt0XMKi21XV9QUqKT0sM9OUCvCCPvcYuhIljuPIz8QAxJg/wApDgKk%2BjsMYiNcOg4cQE87y8F8tzSwwn0sJw9s8Pwu8iIYdActYiSCqKkqaPK0DwKqtLoNyuMeyczixJGviyx9URxlE%2BbxPykCYVkgR5MUlS1LqDTu209k9IMozExM/M/X8yzqu63kbLs2qHMW%2B7nLMcy3JHX8MRYPzBwCqDgvXJ9IosaLONiw8ErhpKLBSi9ZqtDKzt7E0Vr2wrv2K6iyro6aup64tYPcwrge/UHWv4zCFBmz63uvbHiPC/HeMJqjSqAsmGMxqMdsc37lp21aQCZjbhO24aCek5IKiOhLsCUga9lUrLzs0rwrt0/TDKWlNHoU8GXtmj6Iw5WzaQ4BZaE4WJeD8DgtFIT8OCTSxrBdJYVkwFMNh4UgCE0R2FmAjkuAeWOOQADlDyRE7MU0AE4uA0eJnY4SQ3cjr3OF4C4NHDyOFjgWAYEQOMWEzegyAoCB/kb/odkMYAgw0cuaFob9iAuCAIiLiJglqX9ODD8fmGIX8AHkIm0CoI%2B4Xh/jYQQF8QqePd4LB3mANwxFoC519ILA0RxNZPfwbVKlhTBz89zBVAqHtb94RlWiL2g8G8pPDwWAi4EDBCwaevAn7EAiCrQEmBr7AH/kYSufADDAAUAANTwJgO4C9dTuzDvwQQIgxDsCkDIQQigVDqH3qQXQzQDAoNMH7Sw%2BgAEXEgAsVACR2jn2UtGAMLCrCWC4BybWC8zC8DhNEHyz94ALHKJUZwmt3CeEaP4AaPQiglGyMkVIAgRhNESPo9o2i%2BgxDGK0VeVQJhGL0Eo9onQ6jmJmJYwYXR7FjAmK43RXBFGB1WBIJ2LtC50O9nsVQidTTKVNJIa0TChRBgeBoFJq5cCEBICHfxvA15aDmAsBAP5%2Bhsn0JwAupAIGxHLu7T23tS4gHLnkx2pBq513jAkHs5BKBtzoNEUIrA1hRJiXEhJXd9LBlSa/fARAfJ6GIcIUQ4gKELOoWoIuDDSB3FuO5deISOCu1ILU6RnAF49k6UyVAVBInRNifEzuRgJkpLSRADwDc%2Bm5jMKHOYuTK7RxAJIYMmdJBmC4NEnOGdY5BjKfnXgVSalF3qbYRpFd94FNIMBMwicHhYtBaaROsQxEbA0InDOgKYUbDCXUkuqL8n7KkUcxFNLmnougSkZwkggA%3D%3D%3D) ]
```cpp
using namespace utl;

integral::bits::rotl();
integral::bits::bit_width();

integral::math::divide_ceil();
integral::math::narrow_cast();
    
using namespace integral::literals;
using integral::BigUint;

// 1280-bit integer, can easily hold 2^648
const auto two_power_648 = BigUint<1280>(1) << 640;
```

Output:
```

```