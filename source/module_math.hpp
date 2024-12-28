// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::math
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_math.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <algorithm>
#include <initializer_list>
#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_MATH)
#ifndef UTLHEADERGUARD_MATH
#define UTLHEADERGUARD_MATH

// _______________________ INCLUDES _______________________

#include <cassert>     // assert()
#include <cstddef>     // size_t
#include <functional>  // function<>
#include <type_traits> // enable_if_t<>, void_t<>, is_floating_point<>, is_arithmetic<>,
                       // conditional_t<>, is_integral<>, true_type, false_type
#include <utility>     // declval<>()
#include <vector>      // vector<>

// ____________________ DEVELOPER DOCS ____________________

// Coordinate transformations, mathematical constants and technical helper functions.
// A bit of a mix-bag-of-everything, but in the end pretty useful.
//
// # ::PI, ::PI_TWO, ::PI_HALF, ::E, ::GOLDEN_RATION #
// Constants.
//
// # ::is_addable_with_itself<Type> #
// Integral constant, returns in "::value" whether Type supports 'operator()+' with itself.
//
// # ::is_multipliable_by_scalar<Type> #
// Integral constant, returns in "::value" whether Type supports 'operator()*' with double.
//
// # ::is_sized<Type> #
// Integral constant, returns in "::value" whether Type supports '.size()' method.
//
// # ::abs(), ::sign(), ::sqr(), ::cube(), ::midpoint(), deg_to_rad(), rad_to_deg() #
// Constexpr templated math functions, useful when writing expressions with a "textbook form" math.
//
// # ::uint_difference() #
// Returns abs(uint - uint) with respect to uint size and possible overflow.
//
// # ::linspace() #
// Tabulates [min, max] range with N evenly spaced points and returns it as a vector.
//
// # ::ssize() #
// Returns '.size()' of the argument casted to 'int'.
// Essentially a shortcut for verbose 'static_cast<int>(container.size())'.
//
// # ::ternary_branchless() #
// Branchless ternary operator. Slightly slower that regular ternary on most CPUs.
// Should not be used unless branchess qualifier is necessary (like in GPU computation).
//
// # ::ternary_bitselect() #
// Faster branchless ternary for integer types.
// If 2nd return is ommited, 0 is assumed, which allows for significant optimization.

// ____________________ IMPLEMENTATION ____________________

namespace utl::math {

// =================
// --- Constants ---
// =================

constexpr double PI           = 3.14159265358979323846;
constexpr double PI_TWO       = 2. * PI;
constexpr double PI_HALF      = 0.5 * PI;
constexpr double E            = 2.71828182845904523536;
constexpr double GOLDEN_RATIO = 1.6180339887498948482;

// ===================
// --- Type Traits ---
// ===================

template <class Type, class = void>
struct is_addable_with_itself : std::false_type {};

template <class Type>
struct is_addable_with_itself<Type, std::void_t<decltype(std::declval<Type>() + std::declval<Type>())>
                              // perhaps check that resulting type is same as 'Type', but that can cause issues
                              // with classes like Eigen::MatrixXd that return "foldables" that convert back to
                              // objects in the end
                              > : std::true_type {};

template <class Type, class = void>
struct is_multipliable_by_scalar : std::false_type {};

template <class Type>
struct is_multipliable_by_scalar<Type, std::void_t<decltype(std::declval<Type>() * std::declval<double>())>>
    : std::true_type {};

template <class Type, class = void>
struct is_sized : std::false_type {};

template <class Type>
struct is_sized<Type, std::void_t<decltype(std::declval<Type>().size())>> : std::true_type {};

template <class FuncType, class Signature>
using is_function_with_signature = std::is_convertible<FuncType, std::function<Signature>>;

// ======================
// --- Math functions ---
// ======================

template <class Type, std::enable_if_t<std::is_scalar<Type>::value, bool> = true>
[[nodiscard]] constexpr Type abs(Type x) {
    return (x > Type(0)) ? x : -x;
}

template <class Type, std::enable_if_t<std::is_scalar<Type>::value, bool> = true>
[[nodiscard]] constexpr Type sign(Type x) {
    return (x > Type(0)) ? Type(1) : Type(-1);
}

template <class Type, std::enable_if_t<std::is_arithmetic<Type>::value, bool> = true>
[[nodiscard]] constexpr Type sqr(Type x) {
    return x * x;
}

template <class Type, std::enable_if_t<std::is_arithmetic<Type>::value, bool> = true>
[[nodiscard]] constexpr Type cube(Type x) {
    return x * x * x;
}

template <class Type, std::enable_if_t<utl::math::is_addable_with_itself<Type>::value, bool> = true,
          std::enable_if_t<utl::math::is_multipliable_by_scalar<Type>::value, bool> = true>
[[nodiscard]] constexpr Type midpoint(Type a, Type b) {
    return (a + b) * 0.5;
}

template <class IntegerType, std::enable_if_t<std::is_integral<IntegerType>::value, bool> = true>
[[nodiscard]] constexpr int kronecker_delta(IntegerType i, IntegerType j) {
    // 'IntegerType' here is necessary to prevent enforcing static_cast<int>(...) on the callsite
    return (i == j) ? 1 : 0;
}

template <class IntegerType, std::enable_if_t<std::is_integral<IntegerType>::value, bool> = true>
[[nodiscard]] constexpr int power_of_minus_one(IntegerType power) {
    return (power % IntegerType(2)) ? -1 : 1; // is there a faster way of doing it?
}


// --- deg/rad conversion ---
template <class FloatType, std::enable_if_t<std::is_floating_point<FloatType>::value, bool> = true>
[[nodiscard]] constexpr FloatType deg_to_rad(FloatType degrees) {
    constexpr FloatType FACTOR = FloatType(PI / 180.);
    return degrees * FACTOR;
}

template <class FloatType, std::enable_if_t<std::is_floating_point<FloatType>::value, bool> = true>
[[nodiscard]] constexpr FloatType rad_to_deg(FloatType radians) {
    constexpr FloatType FACTOR = FloatType(180. / PI);
    return radians * FACTOR;
}

// ====================
// --- Memory Units ---
// ====================

// Workaround for 'static_assert(false)' making program ill-formed even
// when placed inide an 'if constexpr' branch that never compiles.
// 'static_assert(_always_false_v<T>)' on the the other hand doesn't,
// which means we can use it to mark branches that should never compile.
template <class>
inline constexpr bool _always_false_v = false;

enum class MemoryUnit { BYTE, KiB, MiB, GiB, TiB, KB, MB, GB, TB };

template <class T, MemoryUnit units = MemoryUnit::MiB>
[[nodiscard]] constexpr double memory_size(std::size_t count) {
    const double size_in_bytes = count * sizeof(T); // cast to double is critical here
    if constexpr (units == MemoryUnit::BYTE) return size_in_bytes;
    else if constexpr (units == MemoryUnit::KiB) return size_in_bytes / 1024.;
    else if constexpr (units == MemoryUnit::MiB) return size_in_bytes / 1024. / 1024.;
    else if constexpr (units == MemoryUnit::GiB) return size_in_bytes / 1024. / 1024. / 1024.;
    else if constexpr (units == MemoryUnit::TiB) return size_in_bytes / 1024. / 1024. / 1024. / 1024.;
    else if constexpr (units == MemoryUnit::KB) return size_in_bytes / 1000.;
    else if constexpr (units == MemoryUnit::MB) return size_in_bytes / 1000. / 1000.;
    else if constexpr (units == MemoryUnit::GB) return size_in_bytes / 1000. / 1000. / 1000.;
    else if constexpr (units == MemoryUnit::TB) return size_in_bytes / 1000. / 1000. / 1000. / 1000.;
    else static_assert(_always_false_v<T>, "Function is a non-exhaustive visitor of enum class {MemoryUnit}.");
}

// ===============
// --- Meshing ---
// ===============

// Semantic helpers that allow user to directly pass both interval/point counts for grid subdivision,
// without thinking about whether function need +1 or -1 to its argument
struct Points {
    std::size_t count;

    Points() = delete;
    explicit Points(std::size_t count) : count(count) {}
};

struct Intervals {
    std::size_t count;

    Intervals() = delete;
    explicit Intervals(std::size_t count) : count(count) {}
    Intervals(Points points) : count(points.count - 1) {}
};

template <class FloatType, std::enable_if_t<std::is_floating_point<FloatType>::value, bool> = true>
[[nodiscard]] std::vector<FloatType> linspace(FloatType L1, FloatType L2, Intervals N) {
    assert(L1 < L2);
    assert(N.count >= 1);

    const FloatType step = (L2 - L1) / N.count;

    std::vector<FloatType> res(N.count + 1);

    res[0] = L1;
    for (std::size_t i = 1; i < res.size(); ++i) res[i] = res[i - 1] + step;

    return res;
}

template <class FloatType, class FuncType, std::enable_if_t<std::is_floating_point<FloatType>::value, bool> = true,
          std::enable_if_t<is_function_with_signature<FuncType, FloatType(FloatType)>::value, bool> = true>
[[nodiscard]] FloatType integrate_trapezoidal(FuncType f, FloatType L1, FloatType L2, Intervals N) {
    assert(L1 < L2);
    assert(N.count >= 1);

    const FloatType step = (L2 - L1) / N.count;

    FloatType sum = 0;
    FloatType x   = L1;

    for (std::size_t i = 0; i < N.count; ++i, x += step) sum += f(x) + f(x + step);

    return FloatType(0.5) * sum * step;
}

// ====================
// --- Permutations ---
// ====================

// template<class Idx = std::size_t>
// class Range {
//     Idx low;
//     Idx high;
    
//     constexpr Range(Idx low, Idx high) : low(low), high(high) {}
    
//     [[nodiscard]] constexpr Idx front() const noexcept { return this->low; }
//     [[nodiscard]] constexpr Idx back() const noexcept { return this->high; }
//     [[nodiscard]] constexpr Idx size() const noexcept { return this->high - this->low; }
//     [[nodiscard]] constexpr Idx operator[](Idx pos) const noexcept { return this->low + pos; }
    
//     // Iterator stuff
// };

template<class ArrayType>
bool is_permutation(const ArrayType& array) {
    std::vector<std::size_t> p(array.size()); // Note: "non-allocating range adapter" would fit like a glove here
    for (std::size_t i = 0; i < p.size(); ++i) p[i] = i;
    
    return std::is_permutation(array.begin(), array.end(), p.begin()); // I'm surprised it exists in the standard
}

template<class ArrayType, class PermutationType = std::initializer_list<std::size_t>>
void apply_permutation(ArrayType &vector, const PermutationType& permutation) {
    ArrayType res(vector.size());
    
    typename ArrayType::size_type emplace_idx = 0;
    for (auto i : permutation) res[emplace_idx++] = std::move(vector[i]);
    vector = std::move(res);
}

template<class ArrayType, class Compare = std::less<>>
std::vector<std::size_t> get_sorting_permutation(const ArrayType &array, Compare comp = Compare()) {
    std::vector<std::size_t> permutation(array.size());
    for (std::size_t i = 0; i < permutation.size(); ++i) permutation[i] = i;
    
    std::sort(permutation.begin(), permutation.end(), [&](const auto& lhs, const auto& rhs){
        return comp(array[lhs], array[rhs]);
    });
    
    return permutation;
}

template <class Array, class... SyncedArrays>
void sort_together(Array& array, SyncedArrays&... synced_arrays) {
    // Get permutation that would make the 1st array sorted
    auto permutation = get_sorting_permutation(array);

    // Apply permutation to all arrays to "sort them in sync"
    apply_permutation(array, permutation);
    (apply_permutation(synced_arrays, permutation), ...);
}

// ====================
// --- Misc helpers ---
// ====================

template <class UintType, std::enable_if_t<std::is_integral<UintType>::value, bool> = true>
[[nodiscard]] constexpr UintType uint_difference(UintType a, UintType b) {
    // Cast to widest type if there is a change values don't fit into a regular 'int'
    using WiderIntType = std::conditional_t<(sizeof(UintType) >= sizeof(int)), int64_t, int>;

    return static_cast<UintType>(utl::math::abs(static_cast<WiderIntType>(a) - static_cast<WiderIntType>(b)));
}

template <class SizedContainer, std::enable_if_t<utl::math::is_sized<SizedContainer>::value, bool> = true>
[[nodiscard]] int ssize(const SizedContainer& container) {
    return static_cast<int>(container.size());
}

template <class ArithmeticType, std::enable_if_t<std::is_arithmetic<ArithmeticType>::value, bool> = true>
[[nodiscard]] constexpr ArithmeticType ternary_branchless(bool condition, ArithmeticType return_if_true,
                                                          ArithmeticType return_if_false) {
    return (condition * return_if_true) + (!condition * return_if_false);
}

template <class IntType, std::enable_if_t<std::is_integral<IntType>::value, bool> = true>
[[nodiscard]] constexpr IntType ternary_bitselect(bool condition, IntType return_if_true, IntType return_if_false) {
    return (return_if_true & -IntType(condition)) | (return_if_false & ~(-IntType(condition)));
}

template <class IntType, std::enable_if_t<std::is_integral<IntType>::value, bool> = true>
[[nodiscard]] constexpr IntType ternary_bitselect(bool condition, IntType return_if_true) {
    return return_if_true & -IntType(condition);
}

} // namespace utl::math

#endif
#endif // module utl::math
