// __________ TEST FRAMEWORK & LIBRARY  __________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#define UTL_PICK_MODULES
#define UTLMODULE_STRE
#include "proto_utils.hpp"

// ________________ TEST INCLUDES ________________

#include <array>
#include <vector>
#include <cuchar>

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