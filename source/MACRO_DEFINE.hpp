// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Macro-Module:  UTL_DEFINE
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/MACRO_DEFINE.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMACRO_DEFINE)
#ifndef UTLHEADERGUARD_DEFINE
#define UTLHEADERGUARD_DEFINE

// _______________________ INCLUDES _______________________

#include <algorithm>   // fill_n()
#include <cctype>      // isspace()
#include <cstdlib>     // exit()
#include <iostream>    // cerr
#include <iterator>    // ostreambuf_iterator<>
#include <ostream>     // endl
#include <sstream>     // istringstream
#include <string>      // string, getline()
#include <string_view> // string_view
#include <type_traits> // remove_const<>
#include <utility>     // declval<>()

// ____________________ DEVELOPER DOCS ____________________

// Macros for automatic codegen, such as codegen for type traits, loops,
// operators, enum bitflags, enum <=> string conversions and etc.
//
// Implementation can be rather sketchy at times due to trying to do things that
// aren't really supposed to be done, but nothing straight-up illegal.
//
// # UTL_DEFINE_CURRENT_OS_STRING #
// Evaluates to current platform name detected through compiler macros. Can evaluate to:
// "Windows64", "Windows32", "Windows (CYGWIN)", "Android", "Linux", "Unix-like OS", "MacOS", ""
//
// # UTL_DEFINE_IS_DEBUG #
// Resolves to 'true' in debug mode and to 'false' in release.
// Useful for some debug-only 'contexpr if ()' expressions.
//
// # UTL_DEFINE_REPEAT(N) #
// Equivalent to 'for(int i=0; i<N; ++i)'
//
// # UTL_DEFINE_VA_ARGS_COUNT(args...) #
// Counts how many comma-separated arguments are passed. Works in a true 'preprocessor' evaluating arguments
// as arbitrary text that doesn't have to be a valid identifier. Useful for a lot of variadic macros.
// Note: Since macro evaluates arbitrary preprocessor text, language construct with additional comma should
// be surrounded by parentheses aka '(std::pair<int, int>{4, 5})', since 'std::pair<int, int>{4, 5}' gets
// interpreted as 3 separate args 'std::pair<int', ' int>{4', ' 5}' by any variadic macro.
//
// # UTL_DEFINE_ENUM_WITH_STRING_CONVERSION(enum_name, enum_values...) #
// Create enum 'enum_name' with given enum_values and methods 'to_string()', 'from_string()' inside 'enum_name'
// namespace. Note: Enums can't be declared inside functions.
//
// # UTL_DEFINE_IS_FUNCTION_PRESENT(function_name, return_type, argumet_types...) #
// Creates integral constant named 'is_function_present_{function_name}' that
// returns in "::value" whether function with given name and mask is a valid identifier.
// Useful for detecting existance of platform-specific methods.
// Note: This is kinda pushing the limits of compiler, but possible through SFINAE, which is the only
// mechanism that can perform that kind check. This is the reason implementing an inline FUNC_EXISTS(...)
// kind of macro is impossible and integral constant has to be created.
//
// # UTL_DEFINE_EXIT(message, exit_code) #
// Performs 'std::exit()' with some additional decorators. Displays formatted exit message, exit code,
// file, function and line of callsite to std::cerr. If ommited replaces message & exit_code with default values.

// ____________________ IMPLEMENTATION ____________________

// ===============================
// --- Compilation Info Macros ---
// ===============================

// - Detect OS -
#if defined(_WIN64) // _WIN64 implies _WIN32 so it should be first
#define UTL_DEFINE_CURRENT_OS_STRING "Windows64"
#elif defined(_WIN32)
#define UTL_DEFINE_CURRENT_OS_STRING "Windows32"
#elif defined(__CYGWIN__) && !defined(_WIN32)
#define UTL_DEFINE_CURRENT_OS_STRING "Windows (CYGWIN)" // Cygwin POSIX under Microsoft Window
#elif defined(__ANDROID__)                              // __ANDROID__ implies __linux__ so it should be first
#define UTL_DEFINE_CURRENT_OS_STRING "Android"
#elif defined(__linux__)
#define UTL_DEFINE_CURRENT_OS_STRING "Linux"
#elif defined(unix) || defined(__unix__) || defined(__unix)
#define UTL_DEFINE_CURRENT_OS_STRING "Unix-like OS"
#elif defined(__APPLE__) && defined(__MACH__)
#define UTL_DEFINE_CURRENT_OS_STRING "MacOS" // Apple OSX and iOS (Darwin)
#else
#define UTL_DEFINE_CURRENT_OS_STRING ""
#endif

// - Debug as bool constant -
#ifdef _DEBUG
#define UTL_DEFINE_IS_DEBUG true
#else
#define UTL_DEFINE_IS_DEBUG false
#endif

// =========================
// --- Automatic Codegen ---
// =========================

//  Repeat loop -
#define UTL_DEFINE_REPEAT(repeats_)                                                                                    \
    for (std::remove_const<decltype(repeats_)>::type count_ = 0; count_ < repeats_; ++count_)

// - Size of __VA_ARGS__ in variadic macros -
#define _utl_expand_va_args(x_) x_ // a fix for MSVC bug not expanding __VA_ARGS__ properly

#define _utl_va_args_count_impl(x01_, x02_, x03_, x04_, x05_, x06_, x07_, x08_, x09_, x10_, x11_, x12_, x13_, x14_,    \
                                x15_, x16_, x17_, x18_, x19_, x20_, x21_, x22_, x23_, x24_, x25_, x26_, x27_, x28_,    \
                                x29_, x30_, x31_, x32_, x33_, x34_, x35_, x36_, x37_, x38_, x39_, x40_, x41_, x42_,    \
                                x43_, x44_, x45_, x46_, x47_, x48_, x49_, N_, ...)                                     \
    N_

#define UTL_DEFINE_VA_ARGS_COUNT(...)                                                                                  \
    _utl_expand_va_args(_utl_va_args_count_impl(__VA_ARGS__, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36,   \
                                                35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19,    \
                                                18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

// - Enum with string conversion -
[[nodiscard]] inline std::string _utl_trim_enum_string(const std::string& str) {
    std::string::const_iterator left_it = str.begin();
    while (left_it != str.end() && std::isspace(*left_it)) ++left_it;

    std::string::const_reverse_iterator right_it = str.rbegin();
    while (right_it.base() != left_it && std::isspace(*right_it)) ++right_it;

    return std::string(left_it, right_it.base()); // return string with whitespaces trimmed at both sides
}

inline void _utl_split_enum_args(const char* va_args, std::string* strings, int count) {
    std::istringstream ss(va_args);
    std::string        buffer;

    for (int i = 0; ss.good() && (i < count); ++i) {
        std::getline(ss, buffer, ',');
        strings[i] = _utl_trim_enum_string(buffer);
    }
};

#define UTL_DEFINE_ENUM_WITH_STRING_CONVERSION(enum_name_, ...)                                                        \
    namespace enum_name_ {                                                                                             \
    enum enum_name_ { __VA_ARGS__, _count };                                                                           \
                                                                                                                       \
    inline std::string _strings[_count];                                                                               \
                                                                                                                       \
    inline std::string to_string(enum_name_ enum_val) {                                                                \
        if (_strings[0].empty()) { _utl_split_enum_args(#__VA_ARGS__, _strings, _count); }                             \
        return _strings[enum_val];                                                                                     \
    }                                                                                                                  \
                                                                                                                       \
    inline enum_name_ from_string(const std::string& enum_str) {                                                       \
        if (_strings[0].empty()) { _utl_split_enum_args(#__VA_ARGS__, _strings, _count); }                             \
        for (int i = 0; i < _count; ++i) {                                                                             \
            if (_strings[i] == enum_str) { return static_cast<enum_name_>(i); }                                        \
        }                                                                                                              \
        return _count;                                                                                                 \
    }                                                                                                                  \
    }
// IMPLEMENTATION COMMENTS:
// We declare namespace with enum inside to simulate enum-class while having '_strings' array
// and 'to_string()', 'from_string()' methods bundled with it.
//
// To count number of enum elements we add fake '_count' value at the end, which ends up being enum size
//
// '_strings' is declared compile-time, but gets filled through lazy evaluation upon first
// 'to_string()' or 'from_string()' call. To fill it we interpret #__VA_ARGS__ as a single string
// with some comma-separated identifiers. Those identifiers get split by commas, trimmed from
// whitespaces and added to '_strings'
//
// Upon further calls (enum -> string) conversion is done though taking '_strings[enum_val]',
// while (string -> enum) conversion requires searching through '_strings' to find enum index


// - is_function_present type trait -
#define UTL_DEFINE_IS_FUNCTION_PRESENT(function_name_, return_type_, ...)                                              \
    template <typename ReturnType, typename... ArgTypes>                                                               \
    class _utl_is_function_present_impl_##function_name_ {                                                             \
    private:                                                                                                           \
        typedef char no[sizeof(ReturnType) + 1];                                                                       \
                                                                                                                       \
        template <typename... C>                                                                                       \
        static auto test(C... arg) -> decltype(function_name_(arg...));                                                \
                                                                                                                       \
        template <typename... C>                                                                                       \
        static no& test(...);                                                                                          \
                                                                                                                       \
    public:                                                                                                            \
        enum { value = (sizeof(test<ArgTypes...>(std::declval<ArgTypes>()...)) == sizeof(ReturnType)) };               \
    };                                                                                                                 \
                                                                                                                       \
    using is_function_present_##function_name_ =                                                                       \
        _utl_is_function_present_impl_##function_name_<return_type_, __VA_ARGS__>;
// TASK:
// We need to detect at compile time if function FUNC(ARGS...) exists.
// FUNC identifier isn't guaranteed to be declared.
//
// Ideal method would look like UTL_FUNC_EXISTS(FUNC, RETURN_TYPE, ARG_TYPES...) -> true/false
// This does not seem to be possible, we have to declare integral constant instead, see explanation below.
//
// WHY IS IT SO HARD:
// (1) Can this be done through preprocessor macros?
// No, preprocessor has no way to tell whether C++ identifier is defined or not.
//
// (2) Is there a compiler-specific way to do it?
// Doesn't seem to be the case.
//
// (3) Why not use some sort of template with FUNC as a parameter?
// Essentially we have to evaluate undeclared identifier, while compiler exits with error upon
// encountering anything undeclared. The only way to detect whether undeclared identifier exists
// or not seems to be through SFINAE.
//
// IMPLEMENTATION COMMENTS:
// We declate integral constant class with 2 functions 'test()', first one takes priority during overload
// resolution and compiles if FUNC(ARGS...) is defined, otherwise it's {Substitution Failure} which is
// {Is Not An Error} and second function compiles.
//
// To resolve which overload of 'test()' was selected we check the sizeof() return type, 2nd overload
// has a return type 'char[sizeof(ReturnType) + 1]' so it's always different from 1st overload.
// Resolution result (true/false) gets stored to '::value'.
//
// Note that we can't pass 'ReturnType' and 'ArgTypes' directly through '__VA_ARGS__' because
// to call function 'test(ARGS...)' in general case we have to 'std::declval<>()' all 'ARGS...'.
// To do so we can use variadic template syntax and then just forward '__VA_ARGS__' to the template
// through 'using is_function_present = is_function_present_impl<ReturnType, __VA_ARGS__>'.
//
// ALTERNATIVES: Perhaps some sort of tricky inline SFINAE can be done through C++14 generic lambdas.
//
// Note: Some versions of 'clangd' give a 'bugprone-sizeof-expression' warning for sizeof(*A),
// this is a false alarm.

// - Exit with decorators -
inline void _utl_exit_with_message(std::string_view file, int line, std::string_view func,
                                   std::string_view message = "<NO MESSAGE>", int code = 1) {
    constexpr int  HLINE_WIDTH  = 50;
    constexpr char HLINE_SYMBOL = '-';

    const std::string_view filename = file.substr(file.find_last_of("/\\") + 1);

    // Create some more space
    std::cerr << std::endl;

    // Horizontal line
    std::fill_n(std::ostreambuf_iterator<char>(std::cerr), HLINE_WIDTH, HLINE_SYMBOL);
    std::cerr << std::endl;

    // Text
    std::cerr << "Exit triggered on [" << filename << ":" << line << ", " << func << "()] with:" << std::endl
              << "Message => " << message << std::endl
              << "Code    => " << code << std::endl;

    // Horizontal line
    std::fill_n(std::ostreambuf_iterator<char>(std::cerr), HLINE_WIDTH, HLINE_SYMBOL);
    std::cerr << std::endl;

    std::exit(code);
}

#define UTL_DEFINE_EXIT(...) _utl_exit_with_message(__FILE__, __LINE__, __func__, __VA_ARGS__);

#endif
#endif // macro-module UTL_DEFINE
