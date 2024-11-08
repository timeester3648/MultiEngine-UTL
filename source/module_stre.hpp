// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::stre
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_stre.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_STRE)
#ifndef UTLHEADERGUARD_STRE
#define UTLHEADERGUARD_STRE

// _______________________ INCLUDES _______________________

#include <cstddef>     // size_t
#include <iomanip>     // setfill(), setw()
#include <ostream>     // ostream
#include <sstream>     // ostringstream
#include <string>      // string
#include <string_view> // string_view
#include <tuple>       // tuple<>, get<>()
#include <type_traits> // false_type, true_type, void_t<>, is_convertible<>, enable_if_t<>
#include <utility>     // declval<>(), index_sequence<>

// ____________________ DEVELOPER DOCS ____________________

// String utility extensions, mainly a template ::to_str() method which works with all STL containers,
// including maps, sets and tuples with any level of mutual nesting.
//
// Also includes some expansions of <type_traits> header, since we need them anyways to implement a generic ::to_str().
//
// # ::is_printable<Type> #
// Integral constant, returns in "::value" whether Type can be printed through std::cout.
// Criteria: Existance of operator 'ANY_TYPE operator<<(std::ostream&, Type&)'
//
// # ::is_iterable_through<Type> #
// Integral constant, returns in "::value" whether Type can be iterated through.
// Criteria: Existance of .begin() and .end() with applicable operator()++
//
// # ::is_const_iterable_through<Type> #
// Integral constant, returns in "::value" whether Type can be const-iterated through.
// Criteria: Existance of .cbegin() and .cend() with applicable operator()++
//
// # ::is_tuple_like<Type> #
// Integral constant, returns in "::value" whether Type has a tuple-like structure.
// Tuple-like structure include std::tuple, std::pair, std::array, std::ranges::subrange (since C++20)
// Criteria: Existance of applicable std::get<0>() and std::tuple_size()
//
// # ::is_string<Type> #
// Integral constant, returns in "::value" whether Type is a char string.
// Criteria: Type can be decayed to std::string or a char* pointer
//
// # ::is_to_str_convertible<Type> #
// Integral constant, returns in "::value" whether Type can be converted to string through ::to_str().
// Criteria: Existance of a valid utl::stre::to_str() overload
//
// # ::to_str() #
// Converts any standard container or a custom container with necessary member functions to std::string.
// Works with tuples and tuple-like classes.
// Works with nested containers/tuples through recursive template instantiation, which
// resolves as long as types at the end of recursion have a valid operator<<() for ostreams.
//
// Not particularly fast, but it doesn't really have to be since this kind of thing is mostly
// useful for debugging and other human-readable prints.
//
// # ::InlineStream #
// Inline 'std::ostringstream' construction with implicit conversion to 'std::string'.
// Rather unperformant, but convenient for using stream formating during string construction.
// Example: std::string str = (stre::InlineStream() << "Value " << 3.14 << " is smaller than " << 6.28);
//
// In retrospective the usefulness of this seems rather dubious. Perhaps I'll deprecate it later.
//
// # ::repeat_symbol(), ::repeat_string() #
// Repeats character/string a given number of times.
//
// # ::pad_with_zeroes() #
// Pads given integer with zeroes untill a certain lenght.
// Useful when saving data in files like 'data_0001.txt', 'data_0002.txt', '...' so they get properly sorted.

// ____________________ IMPLEMENTATION ____________________

namespace utl::stre {

// ===================
// --- Type Traits ---
// ===================

template <typename Type, typename = void>
struct is_printable : std::false_type {};

template <typename Type>
struct is_printable<Type, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<Type>())>>
    : std::true_type {};

template <typename Type, typename = void, typename = void>
struct is_iterable_through : std::false_type {};

template <typename Type>
struct is_iterable_through<Type, std::void_t<decltype(++std::declval<Type>().begin())>,
                           std::void_t<decltype(std::declval<Type>().end())>> : std::true_type {};

template <typename Type, typename = void, typename = void>
struct is_const_iterable_through : std::false_type {};

template <typename Type>
struct is_const_iterable_through<Type, std::void_t<decltype(++std::declval<Type>().cbegin())>,
                                 std::void_t<decltype(std::declval<Type>().cend())>> : std::true_type {};

template <typename Type, typename = void, typename = void>
struct is_tuple_like : std::false_type {};

template <typename Type>
struct is_tuple_like<Type, std::void_t<decltype(std::get<0>(std::declval<Type>()))>,
                     std::void_t<decltype(std::tuple_size<Type>::value)>> : std::true_type {};

template <typename Type>
struct is_string : std::is_convertible<Type, std::string_view> {};

// ================
// --- to_str() ---
// ================

// --- Delimers ---
// ----------------

constexpr auto _array_delimer_l = "[ ";
constexpr auto _array_delimer_m = ", ";
constexpr auto _array_delimer_r = " ]";
constexpr auto _tuple_delimer_l = "< ";
constexpr auto _tuple_delimer_m = ", ";
constexpr auto _tuple_delimer_r = " >";

// --- Predeclarations ---
// -----------------------

// 'false_type' half of 'is_to_str_convertible' should be declared before 'to_str()' to resolve circular dependency
template <typename Type, typename = void>
struct is_to_str_convertible : std::false_type {};


// 'to_str(tuple)' should be predeclared to resolve circular dependency between 'to_str(container)' and 'to_str(tuple)'
template <template <typename... Params> class TupleLikeType, typename... Args,
          std::enable_if_t<!utl::stre::is_printable<TupleLikeType<Args...>>::value, bool>              = true,
          std::enable_if_t<!utl::stre::is_const_iterable_through<TupleLikeType<Args...>>::value, bool> = true,
          std::enable_if_t<utl::stre::is_tuple_like<TupleLikeType<Args...>>::value, bool>              = true>
[[nodiscard]] std::string to_str(const TupleLikeType<Args...>& tuple);

// --- Implementation ---
// ----------------------

// - to_str(printable) -
template <typename Type, std::enable_if_t<utl::stre::is_printable<Type>::value, bool> = true>
[[nodiscard]] std::string to_str(const Type& value) {
    std::ostringstream ss;

    ss << value;

    return ss.str();
}

// - to_str(container) -
template <typename ContainerType, std::enable_if_t<!utl::stre::is_printable<ContainerType>::value, bool> = true,
          std::enable_if_t<utl::stre::is_const_iterable_through<ContainerType>::value, bool>                  = true,
          std::enable_if_t<utl::stre::is_to_str_convertible<typename ContainerType::value_type>::value, bool> = true>
[[nodiscard]] std::string to_str(const ContainerType& container) {

    std::ostringstream ss;

    // Special case for empty containers
    if (container.cbegin() == container.cend()) {
        ss << _array_delimer_l << _array_delimer_r;
        return ss.str();
    }

    // Iterate throught the container 'looking forward' by one step
    // so we can know not to place the last delimer. Using -- or std::prev()
    // is not and option since we only require iterators to be forward-iterable
    ss << _array_delimer_l;

    auto it_next = container.cbegin(), it = it_next++;
    for (; it_next != container.cend(); ++it_next, ++it) ss << utl::stre::to_str(*it) << _array_delimer_m;

    ss << utl::stre::to_str(*it) << _array_delimer_r;

    return ss.str();
}

// - to_str(tuple) helpers -
template <typename TupleElemType>
[[nodiscard]] std::string _deduce_and_perform_string_conversion(const TupleElemType& elem) {
    std::ostringstream temp_ss;

    if constexpr (utl::stre::is_to_str_convertible<TupleElemType>::value) temp_ss << utl::stre::to_str(elem);
    else temp_ss << elem;

    return temp_ss.str();
}

template <typename TupleLikeType, std::size_t... Is>
void _print_tuple_fold(std::ostringstream& ss, const TupleLikeType& tuple, std::index_sequence<Is...>) {
    ((ss << (Is == 0 ? "" : _tuple_delimer_m) << _deduce_and_perform_string_conversion(std::get<Is>(tuple))), ...);
} // prints tuple to stream

// - to_str(tuple) -
template <template <typename... Params> class TupleLikeType, typename... Args,
          std::enable_if_t<!utl::stre::is_printable<TupleLikeType<Args...>>::value, bool>,
          std::enable_if_t<!utl::stre::is_const_iterable_through<TupleLikeType<Args...>>::value, bool>,
          std::enable_if_t<utl::stre::is_tuple_like<TupleLikeType<Args...>>::value, bool>>
[[nodiscard]] std::string to_str(const TupleLikeType<Args...>& tuple) {

    std::ostringstream ss;

    // Print tuple using C++17 variadic folding with index sequence
    ss << _tuple_delimer_l;
    _print_tuple_fold(ss, tuple, std::index_sequence_for<Args...>{});
    ss << _tuple_delimer_r;

    return ss.str();
}

// - is_to_str_convertible -
template <typename Type>
struct is_to_str_convertible<Type, std::void_t<decltype(utl::stre::to_str(std::declval<Type>()))>> : std::true_type {};

// ===========================
// --- Inline stringstream ---
// ===========================

class InlineStream {
public:
    template <typename Type>
    InlineStream& operator<<(const Type& arg) {
        this->ss << arg;
        return *this;
    }

    inline operator std::string() const { return this->ss.str(); }

private:
    std::ostringstream ss;
};

// ===================
// --- Misc. Utils ---
// ===================

[[nodiscard]] inline std::string repeat_symbol(char symbol, size_t repeats) { return std::string(repeats, symbol); }

[[nodiscard]] inline std::string repeat_string(std::string_view str, size_t repeats) {
    std::string res;
    res.reserve(str.size() * repeats);
    while (repeats--) res += str;
    return res;
}

template <typename IntegerType, std::enable_if_t<std::is_integral<IntegerType>::value, bool> = true>
[[nodiscard]] std::string pad_with_zeroes(IntegerType number, std::streamsize total_size = 10) {
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(total_size) << number;
    return ss.str();
}

} // namespace utl::stre

#endif
#endif // module utl::stre
