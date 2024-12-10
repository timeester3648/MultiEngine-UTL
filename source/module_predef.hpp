// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::predef
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_predef.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_PREDEF)
#ifndef UTLHEADERGUARD_PREDEF
#define UTLHEADERGUARD_PREDEF

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
#include <utility>     // declval<>()

// ____________________ DEVELOPER DOCS ____________________

// Macros that provide a nicer way of querying some platform-specific stuff such as:
// compiler, platform, architecture, compilation info and etc.
//
// Boost Predef (https://www.boost.org/doc/libs/1_55_0/libs/predef/doc/html/index.html) provides
// a more complete package when it comes to supporing some esoteric platforms & compilers,
// but has a rather (in my opinion) ugly API.
//
// In addition utl::predef also provides some miscellaneous macros for automatic codegen, such as:
//    UTL_PREDEF_VA_ARGS_COUNT(args...)
//    UTL_PREDEF_ENUM_WITH_STRING_CONVERSION(enum_name, enum_values...)
//    UTL_PREDEF_IS_FUNCTION_DEFINED() - a nightmare of implementation, but it works
// some implementations may be rather sketchy due to trying to achieve things that weren't really
// meant to be achieved, but at the end of the day everything is standard-compliant.

// ____________________ IMPLEMENTATION ____________________

namespace utl::predef {

// ================================
// --- Compiler Detection Macro ---
// ================================

#if defined(_MSC_VER) || defined(_MSC_FULL_VER)
#define UTL_PREDEF_COMPILER_IS_MSVC
#elif defined(__GNUC__) || defined(__GNUC_MINOR__) || defined(__GNUC_PATCHLEVEL__)
#define UTL_PREDEF_COMPILER_IS_GCC
#elif defined(__clang__) || defined(__clang_major__) || defined(__clang_minor__) || defined(__clang_patchlevel__)
#define UTL_PREDEF_COMPILER_IS_CLANG
#elif defined(__llvm__)
#define UTL_PREDEF_COMPILER_IS_LLVM
#elif defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC)
#define UTL_PREDEF_COMPILER_IS_ICC
#elif defined(__PGI) || defined(__PGIC__) || defined(__PGIC_MINOR__) || defined(__PGIC_PATCHLEVEL__)
#define UTL_PREDEF_COMPILER_IS_PGI
#elif defined(__IBMCPP__) || defined(__xlC__) || defined(__xlc__)
#define UTL_PREDEF_COMPILER_IS_IBMCPP
#elif defined(__NVCC__) || defined(__CUDACC__)
#define UTL_PREDEF_COMPILER_IS_NVCC
#else
#define UTL_PREDEF_COMPILER_IS_UNKNOWN
#endif

constexpr std::string_view compiler_name =
#if defined(UTL_PREDEF_COMPILER_IS_MSVC)
    "MSVC"
#elif defined(UTL_PREDEF_COMPILER_IS_GCC)
    "GCC"
#elif defined(UTL_PREDEF_COMPILER_IS_CLANG)
    "clang"
#elif defined(UTL_PREDEF_COMPILER_IS_LLVM)
    "LLVM"
#elif defined(UTL_PREDEF_COMPILER_IS_ICC)
    "ICC"
#elif defined(UTL_PREDEF_COMPILER_IS_PGI)
    "PGI"
#elif defined(UTL_PREDEF_COMPILER_IS_IBMCPP)
    "IBMCPP"
#elif defined(UTL_PREDEF_COMPILER_IS_NVCC)
    "NVCC"
#else
    "<unknown>"
#endif
    ;

constexpr std::string_view compiler_full_name =
#if defined(UTL_PREDEF_COMPILER_IS_MSVC)
    "Microsoft Visual C++ Compiler"
#elif defined(UTL_PREDEF_COMPILER_IS_GCC)
    "GNU C/C++ Compiler"
#elif defined(UTL_PREDEF_COMPILER_IS_CLANG)
    "Clang Compiler"
#elif defined(UTL_PREDEF_COMPILER_IS_LLVM)
    "LLVM Compiler"
#elif defined(UTL_PREDEF_COMPILER_IS_ICC)
    "Inter C/C++ Compiler"
#elif defined(UTL_PREDEF_COMPILER_IS_PGI)
    "Portland Group C/C++ Compiler"
#elif defined(UTL_PREDEF_COMPILER_IS_IBMCPP)
    "IBM XL C/C++ Compiler"
#elif defined(UTL_PREDEF_COMPILER_IS_NVCC)
    "Nvidia Cuda Compiler Driver"
#else
    "<unknown>"
#endif
    ;

// ================================
// --- Platform Detection Macro ---
// ================================

#if defined(_WIN64) // _WIN64 implies _WIN32 so it should be first
#define UTL_PREDEF_PLATFORM_IS_WINDOWS_X64
#elif defined(_WIN32)
#define UTL_PREDEF_PLATFORM_IS_WINDOWS_X32
#elif defined(__CYGWIN__) && !defined(_WIN32) // Cygwin POSIX under Microsoft Window
#define UTL_PREDEF_PLATFORM_IS_CYGWIN
#elif defined(__ANDROID__) // __ANDROID__ implies __linux__ so it should be first
#define UTL_PREDEF_PLATFORM_IS_ANDROID
#elif defined(linux) || defined(__linux__) || defined(__linux)
#define UTL_PREDEF_PLATFORM_IS_LINUX
#elif defined(unix) || defined(__unix__) || defined(__unix)
#define UTL_PREDEF_PLATFORM_IS_UNIX
#elif defined(__APPLE__) && defined(__MACH__)
#define UTL_PREDEF_PLATFORM_IS_MACOS
#else
#define UTL_PREDEF_PLATFORM_IS_UNKNOWN
#endif

constexpr std::string_view platform_name =
#if defined(UTL_PREDEF_PLATFORM_IS_WINDOWS_X64)
    "Windows64"
#elif defined(UTL_PREDEF_PLATFORM_IS_WINDOWS_X32)
    "Windows32"
#elif defined(UTL_PREDEF_PLATFORM_IS_CYGWIN)
    "Windows (CYGWIN)"
#elif defined(UTL_PREDEF_PLATFORM_IS_ANDROID)
    "Android"
#elif defined(UTL_PREDEF_PLATFORM_IS_LINUX)
    "Linux"
#elif defined(UTL_PREDEF_PLATFORM_IS_UNIX)
    "Unix-like OS"
#elif defined(UTL_PREDEF_PLATFORM_IS_MACOS)
    "MacOS" // Apple OSX and iOS (Darwin)
#else
    "<unknown>"
#endif
    ;


// ====================================
// --- Architecture Detection Macro ---
// ====================================

#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || defined(_M_X64)
#define UTL_PREDEF_ARCHITECTURE_IS_X86_64
#elif defined(i386) || defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) ||             \
    defined(__i386) || defined(_M_IX86) || defined(_X86_) || defined(__THW_INTEL__) || defined(__I86__) ||             \
    defined(__INTEL__) || defined(__I86__) || defined(_M_IX86) || defined(__i686__) || defined(__i586__) ||            \
    defined(__i486__) || defined(__i386__)
#define UTL_PREDEF_ARCHITECTURE_IS_X86_32
#elif defined(__arm__) || defined(__thumb__) || defined(__TARGET_ARCH_ARM) || defined(__TARGET_ARCH_THUMB) ||          \
    defined(__TARGET_ARCH_ARM) || defined(__TARGET_ARCH_THUMB)
#define UTL_PREDEF_ARCHITECTURE_IS_ARM
#else
UTL_PREDEF_ARCHITECTURE_IS_UNKNOWN
#endif

constexpr std::string_view architecture_name =
#if defined(UTL_PREDEF_ARCHITECTURE_IS_X86_64)
    "x86-64"
#elif defined(UTL_PREDEF_ARCHITECTURE_IS_X86_32)
    "x86-32"
#elif defined(UTL_PREDEF_ARCHITECTURE_IS_ARM)
    "ARM"
#else
    "<unknown>"
#endif
    ;

// =========================================
// --- Language Standard Detection Macro ---
// =========================================

#if defined(UTL_PREDEF_COMPILER_IS_MSVC)
#define UTL_PREDEF_CPP_VERSION _MSVC_LANG
#else
#define UTL_PREDEF_CPP_VERSION __cplusplus
#endif
// Note 1:
// MSVC '__cplusplus' is defined, but stuck at '199711L'. It uses '_MSVC_LANG' instead.
//
// Note 2:
// '__cplusplus' is defined by the standard, it's only Microsoft who think standards are for other people.
//
// Note 3:
// MSVC has a flag '/Zc:__cplusplus' that enables standard behaviour for '__cplusplus'

#if (UTL_PREDEF_CPP_VERSION >= 202302L)
#define UTL_PREDEF_STANDARD_IS_23_PLUS
#elif (UTL_PREDEF_CPP_VERSION >= 202002L)
#define UTL_PREDEF_STANDARD_IS_20_PLUS
#elif (UTL_PREDEF_CPP_VERSION >= 201703L)
#define UTL_PREDEF_STANDARD_IS_17_PLUS
#elif (UTL_PREDEF_CPP_VERSION >= 201402L)
#define UTL_PREDEF_STANDARD_IS_14_PLUS
#elif (UTL_PREDEF_CPP_VERSION >= 201103L)
#define UTL_PREDEF_STANDARD_IS_11_PLUS
#else // everything below C++11 has the same value of '199711L'
#define UTL_PREDEF_STANDARD_IS_UNKNOWN
#endif
// Note:
// There should be no feasible way to fall below the 'UTL_PREDEF_STANDARD_IS_17_PLUS' since this library itself
// requires C++17 to compile, but might as well have a complete implementation for future reference.

constexpr std::string_view standard_name =
#if defined(UTL_PREDEF_STANDARD_IS_23_PLUS)
    "C++23"
#elif defined(UTL_PREDEF_STANDARD_IS_20_PLUS)
    "C++20"
#elif defined(UTL_PREDEF_STANDARD_IS_17_PLUS)
    "C++17"
#elif defined(UTL_PREDEF_STANDARD_IS_14_PLUS)
    "C++14"
#elif defined(UTL_PREDEF_STANDARD_IS_11_PLUS)
    "C++11"
#else
    "<unknown>"
#endif
    ;

// ========================================
// --- Compilation Mode Detection Macro ---
// ========================================

#if defined(_DEBUG)
#define UTL_PREDEF_MODE_IS_DEBUG
#endif

constexpr bool debug =
#if defined(UTL_PREDEF_MODE_IS_DEBUG)
    true
#else
    false
#endif
    ;

// ===========================
// --- Optimization Macros ---
// ===========================

#if defined(UTL_PREDEF_COMPILER_IS_MSVC)
#define UTL_PREDEF_FORCE_INLINE __forceinline
#elif defined(UTL_PREDEF_COMPILER_IS_GCC) || defined(UTL_PREDEF_COMPILER_IS_CLANG) || defined(UTL_PREDEF_COMPILER_IS_CLANG)
#define UTL_PREDEF_FORCE_INLINE __attribute__((always_inline))
#else
#define UTL_PREDEF_FORCE_INLINE
#endif

#if defined(UTL_PREDEF_STANDARD_IS_23_PLUS)
#define UTL_PREDEF_ASSUME [[assume(__VA_ARGS__))]]
#elif defined(UTL_PREDEF_COMPILER_IS_MSVC)
__assume(__VA_ARGS__)
#elif defined(UTL_PREDEF_COMPILER_IS_CLANG)
__builtin_assume(__VA_ARGS__)
#else // no equivalent GCC built-in
#endif

[[noreturn]] void unreachable() {
#if defined(UTL_PREDEF_STANDARD_IS_23_PLUS)
std::unreachable();
#elif defined(UTL_PREDEF_COMPILER_IS_MSVC)
__assume(false);
#elif defined(UTL_PREDEF_COMPILER_IS_GCC) || defined(UTL_PREDEF_COMPILER_IS_CLANG)
__builtin_unreachable();
#endif
}

// ===================
// --- Other Utils ---
// ===================

[[nodiscard]] std::string compilation_summary() {
    std::string buffer;

    buffer += "Compiler:          ";
    buffer += compiler_full_name;
    buffer += '\n';

    buffer += "Platform:          ";
    buffer += platform_name;
    buffer += '\n';

    buffer += "Architecture:      ";
    buffer += architecture_name;
    buffer += '\n';

    buffer += "Compiled in DEBUG: ";
    buffer += debug ? "true" : "false";
    buffer += '\n';

    buffer += "Compiled under OS: ";
    buffer += __STDC_HOSTED__ ? "true" : "false";
    buffer += '\n';

    buffer += "Compilation date:  ";
    buffer += __DATE__;
    buffer += ' ';
    buffer += __TIME__;
    buffer += '\n';

    return buffer;
}

// - Size of __VA_ARGS__ in variadic macros -
#define _utl_expand_va_args(x_) x_ // a fix for MSVC bug not expanding __VA_ARGS__ properly

#define _utl_va_args_count_impl(x01_, x02_, x03_, x04_, x05_, x06_, x07_, x08_, x09_, x10_, x11_, x12_, x13_, x14_,    \
                                x15_, x16_, x17_, x18_, x19_, x20_, x21_, x22_, x23_, x24_, x25_, x26_, x27_, x28_,    \
                                x29_, x30_, x31_, x32_, x33_, x34_, x35_, x36_, x37_, x38_, x39_, x40_, x41_, x42_,    \
                                x43_, x44_, x45_, x46_, x47_, x48_, x49_, N_, ...)                                     \
    N_

#define UTL_PREDEF_VA_ARGS_COUNT(...)                                                                                  \
    _utl_expand_va_args(_utl_va_args_count_impl(__VA_ARGS__, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36,   \
                                                35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19,    \
                                                18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

// - Enum with string conversion -
[[nodiscard]] inline std::string _trim_enum_string(const std::string& str) {
    std::string::const_iterator left_it = str.begin();
    while (left_it != str.end() && std::isspace(*left_it)) ++left_it;

    std::string::const_reverse_iterator right_it = str.rbegin();
    while (right_it.base() != left_it && std::isspace(*right_it)) ++right_it;

    return std::string(left_it, right_it.base()); // return string with whitespaces trimmed at both sides
}

inline void _split_enum_args(const char* va_args, std::string* strings, int count) {
    std::istringstream ss(va_args);
    std::string        buffer;

    for (int i = 0; ss.good() && (i < count); ++i) {
        std::getline(ss, buffer, ',');
        strings[i] = _trim_enum_string(buffer);
    }
};

#define UTL_PREDEF_ENUM_WITH_STRING_CONVERSION(enum_name_, ...)                                                        \
    namespace enum_name_ {                                                                                             \
    enum enum_name_ { __VA_ARGS__, _count };                                                                           \
                                                                                                                       \
    inline std::string _strings[_count];                                                                               \
                                                                                                                       \
    inline std::string to_string(enum_name_ enum_val) {                                                                \
        if (_strings[0].empty()) { utl::predef::_split_enum_args(#__VA_ARGS__, _strings, _count); }                             \
        return _strings[enum_val];                                                                                     \
    }                                                                                                                  \
                                                                                                                       \
    inline enum_name_ from_string(const std::string& enum_str) {                                                       \
        if (_strings[0].empty()) { utl::predef::_split_enum_args(#__VA_ARGS__, _strings, _count); }                             \
        for (int i = 0; i < _count; ++i) {                                                                             \
            if (_strings[i] == enum_str) { return static_cast<enum_name_>(i); }                                        \
        }                                                                                                              \
        return _count;                                                                                                 \
    }                                                                                                                  \
    }
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

#define UTL_PREDEF_IS_FUNCTION_DEFINED(function_name_, return_type_, ...)                                              \
    template <typename ReturnType, typename... ArgTypes>                                                               \
    class _utl_is_function_defined_impl_##function_name_ {                                                             \
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
    using is_function_defined_##function_name_ =                                                                       \
        _utl_is_function_defined_impl_##function_name_<return_type_, __VA_ARGS__>;
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
// NOTE 1: Some versions of 'clangd' give a 'bugprone-sizeof-expression' warning for sizeof(*A),
// this is a false alarm.
//
// NOTE 2: Frankly, the usefullness of this is rather dubious since constructs like
//     if constexpr (is_function_defined_windown_specific) { <call the windows-specific function> }
//     else { <call the linux-specific function> }
// are still illegal due to 'if constexpr' requiting both branches to have defined identifiers,
// but since this arcane concept is already implemented why not keep it.

} // namespace utl::predef

#endif
#endif // module utl::predef
