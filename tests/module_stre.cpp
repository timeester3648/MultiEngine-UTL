// __________ TEST FRAMEWORK & LIBRARY  __________

#include <random>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#define UTL_PICK_MODULES
#define UTLMODULE_STRE
#include "proto_utils.hpp"

// ________________ TEST INCLUDES ________________

#include <array>
#include <cuchar>
#include <random>
#include <tuple>
#include <unordered_map>
#include <vector>

// _____________ TEST IMPLEMENTATION _____________

TEST_CASE_TEMPLATE("(stre::is_string<T> == true) for T = ", T,
    // String types
    char*,
    const char*,
    std::string,
    std::string_view
) {
    CHECK(utl::stre::is_string<T>::value == true);
}

TEST_CASE_TEMPLATE("(stre::is_string<T> == false) for T = ", T,
    // Types that clearly aren't a string
    int,
    double,
    // Types that can be mistaken for a string
    std::array<char, 4>,
    std::vector<char>,
    // Types that denote string with a non-standard encoding
    unsigned char*,
    wchar_t*,
    std::wstring,
    std::wstring_view,
    char16_t*,
    std::u16string,
    std::u16string_view,
    char32_t*,
    std::u32string,
    std::u32string_view
) {
    CHECK(utl::stre::is_string<T>::value == false);
}

TEST_CASE_TEMPLATE("(stre::is_to_str_convertible<T> == true) for T = ", T,
    // "Directly printable" types
    char*,
    const char*,
    std::string,
    std::string_view,
    int,
    unsigned int,
    float,
    double,
    // "Complex" types
    std::vector<int>,                                                                         // vector
    std::tuple<int, double, std::string>,                                                     // tuple
    std::vector<std::vector<int>>,                                                            // vector of vectors
    std::unordered_map<std::string, int>,                                                     // map
    std::tuple<std::vector<bool>, std::vector<std::string>, std::vector<std::pair<int, int>>> // tuple of vectors
) {
    CHECK(utl::stre::is_to_str_convertible<T>::value == true);
}