// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::stre
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_stre.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <exception>
#include <stdexcept>
#include <vector>
#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_STRE)
#ifndef UTLHEADERGUARD_STRE
#define UTLHEADERGUARD_STRE

// _______________________ INCLUDES _______________________

#include <algorithm>   // transform()
#include <cctype>      // tolower(), toupper()
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

// ================
// --- Trimming ---
// ================

template <class T>
[[nodiscard]] std::string trim_left(T&& str, char trimmed_char = ' ') {
    std::string res = std::forward<T>(str);  // when 'str' is an r-value, we can avoid the copy
    res.erase(0, res.find_first_not_of(trimmed_char)); // seems to be the fastest way of doing it
    return res;
}

template <class T>
[[nodiscard]] std::string trim_right(T&& str, char trimmed_char = ' ') {
    std::string res = std::forward<T>(str);
    res.erase(res.find_last_not_of(trimmed_char) + 1);
    return res;
}

template <class T>
[[nodiscard]] std::string trim(T&& str, char trimmed_char = ' ') {
    return trim_right(trim_left(std::forward<T>(str), trimmed_char), trimmed_char);
}

// ===============
// --- Padding ---
// ===============

[[nodiscard]] std::string pad_left(std::string_view str, std::size_t length, char padding_char = ' ') {
    if (length > str.size()) {
        std::string res;
        res.reserve(length);
        res.append(length - str.size(), padding_char);
        res += str;
        return res;
    } else return std::string(str);
}

[[nodiscard]] std::string pad_right(std::string_view str, std::size_t length, char padding_char = ' ') {
    if (length > str.size()) {
        std::string res;
        res.reserve(length);
        res += str;
        res.append(length - str.size(), padding_char);
        return res;
    } else return std::string(str);
}

[[nodiscard]] std::string pad(std::string_view str, std::size_t length, char padding_char = ' ') {
    if (length > str.size()) {
        std::string res;
        res.reserve(length);
        const std::size_t left_pad_size = (length - str.size()) / 2;
        res.append(left_pad_size, padding_char);
        res += str;
        const std::size_t right_pad_size = length - str.size() - left_pad_size;
        res.append(right_pad_size, padding_char);
        return res;
        // we try to pad evenly on both sides, but one of the pads (the right one to be exact)
        // may be a character longer than the other if the length difference is odd
    } else return std::string(str);
}

[[nodiscard]] std::string pad_with_leading_zeroes(unsigned int number, std::size_t length = 12) {
    const std::string number_str = std::to_string(number);

    if (length > number_str.size()) {
        std::string res;
        res.reserve(length);
        res.append(length - number_str.size(), '0');
        res += number_str;
        return res;
    } else return number_str;
    // we do this instead of using 'std::ostringstream' with 'std::setfill('0')' + 'std::setw()'
    // so we don't need streams as a dependency. Plus it is faster that way.
}

// ========================
// --- Case conversions ---
// ========================

template <class T>
[[nodiscard]] std::string to_lower(T&& str) {
    std::string res = std::forward<T>(str); // when 'str' is an r-value, we can avoid the copy
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) { return std::tolower(c); });
    return res;
    // note that 'std::tolower()', 'std::toupper()' can only apply to unsigned chars, calling it on signed char
    // is UB. Implementation above was directly taken from https://en.cppreference.com/w/cpp/string/byte/tolower
}

template <class T>
[[nodiscard]] std::string to_upper(T&& str) {
    std::string res = std::forward<T>(str);
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) { return std::toupper(c); });
    return res;
}

// ========================
// --- Substring checks ---
// ========================

// Note:
// C++20 adds 'std::basic_string<T>::starts_with()', 'std::basic_string<T>::ends_with()',
// 'std::basic_string<T>::contains()', making these functions pointless in a new standard.

[[nodiscard]] bool starts_with(std::string_view str, std::string_view substr) {
    return str.size() >= substr.size() && str.compare(0, substr.size(), substr) == 0;
}

[[nodiscard]] bool ends_with(std::string_view str, std::string_view substr) {
    return str.size() >= substr.size() && str.compare(str.size() - substr.size(), substr.size(), substr) == 0;
}

[[nodiscard]] bool contains(std::string_view str, std::string_view substr) {
    return str.find(substr) != std::string_view::npos;
}

// ==========================
// --- Token manipulation ---
// ==========================

template <class T>
[[nodiscard]] std::string replace_all_occurences(T&& str, std::string_view from, std::string_view to) {
    std::string res = std::forward<T>(str);

    std::size_t i = 0;
    while ((i = res.find(from, i)) != std::string::npos) { // locate substring to replace
        res.replace(i, from.size(), to);                   // replace
        i += to.size();                                    // step over the replaced region
    }
    // Note: Not stepping over the replaced regions causes self-similar replacements
    // like "abc" -> "abcabc" to fall into an infinite loop, we don't want that.

    return res;
}

// Note:
// Most "split by delimer" implementations found online seem to be horrifically inefficient
// with unnecessary copying/erasure/intermediate tokens, stringstreams and etc.
//
// We can just scan through the string view once, while keeping track of the last segment between
// two delimiters, no unnecessary work, the only place where we do a copy is during emplacement into
// the vector where it's unavoidable
[[nodiscard]] std::vector<std::string> split_by_delimiter(std::string_view str, std::string_view delimiter, bool keep_empty_tokens = false) {
    if (delimiter.empty()) return {std::string(str)};
    // handle empty delimiter explicitly so we can't fall into an infinite loop

    std::vector<std::string> tokens;
    std::size_t              cursor        = 0;
    std::size_t              segment_start = cursor;

    while ((cursor = str.find(delimiter, cursor)) != std::string_view::npos) {
        if (keep_empty_tokens || segment_start != cursor) tokens.emplace_back(str.substr(segment_start, cursor - segment_start));
        // don't emplace empty tokens in case of leading/trailing/repeated delimiter
        cursor += delimiter.size();
        segment_start = cursor;
    }

    if (keep_empty_tokens || segment_start != str.size()) tokens.emplace_back(str.substr(segment_start));
    // 'cursor' is now at 'npos', so we compare to the size instead
    
    return tokens;
}

// ===================
// --- Other utils ---
// ===================

[[nodiscard]] inline std::string repeat_char(char ch, size_t repeats) { return std::string(repeats, ch); }

[[nodiscard]] inline std::string repeat_string(std::string_view str, size_t repeats) {
    std::string res;
    res.reserve(str.size() * repeats);
    while (repeats--) res += str;
    return res;
}

// Mostly useful to print strings with special chars in console and look at their contents.
[[nodiscard]] std::string escape_control_chars(std::string_view str) {
    std::string res;
    res.reserve(str.size()); // no necesseraly correct, but it's a godd first guess

    for (const char c : str) {
        // Control characters with dedicated escape sequences get escaped with those sequences
        if (c == '\a') res += "\\a";
        else if (c == '\b') res += "\\b";
        else if (c == '\f') res += "\\f";
        else if (c == '\n') res += "\\n";
        else if (c == '\r') res += "\\r";
        else if (c == '\t') res += "\\t";
        else if (c == '\v') res += "\\v";
        // Other non-printable chars get replaced with their codes
        else if (!std::isprint(static_cast<unsigned char>(c))) {
            res += '\\';
            res += std::to_string(static_cast<int>(c));
        }
        // Printable chars are appended as is.
        else
            res += c;
    }
    // Note: This could be implemented faster using the 'utl::json' method of handling escapes with buffering and
    // a lookup table, however I don't see much practical reason to complicate this implementation like that.

    return res;
}

[[nodiscard]] std::size_t index_of_difference(std::string_view str_1, std::string_view str_2) {
    using namespace std::string_literals;
    if (str_1.size() != str_2.size())
        throw std::logic_error("String {"s + std::string(str_1) + "} of size "s + std::to_string(str_1.size()) +
                               " and {"s + std::string(str_2) + "} of size "s + std::to_string(str_2.size()) +
                               " do not have a meaningful index of difference due to incompatible sizes."s);
    for (std::size_t i = 0; i < str_1.size(); ++i)
        if (str_1[i] != str_2[i]) return i;
    return str_1.size();
}

} // namespace utl::stre

#endif
#endif // module utl::stre
