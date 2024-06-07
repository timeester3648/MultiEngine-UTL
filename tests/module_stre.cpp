// __________ TEST FRAMEWORK & LIBRARY  __________

#include <random>
#include <string>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#define UTL_PICK_MODULES
#define UTLMODULE_STRE
#include "proto_utils.hpp"

// ________________ TEST INCLUDES ________________

#include <array>
#include <cuchar>
#include <map>
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

TEST_CASE("stre::to_str() performs as expected") {
    
    std::vector<int>                     test1 = { 1, 2, 3 };
    std::vector<std::vector<int>>        test2 = { { 1, 2 }, { 3 }, { 4, 5, 6 } };
    std::tuple<int, double, std::string> test3 = { 17, 0.5, "text" };
    std::map<std::string, int>           test4 = { { "key1", 1 }, { "key2", 2 } };
    
    CHECK(utl::stre::to_str(test1) == std::string("[ 1, 2, 3 ]"                     ));
    CHECK(utl::stre::to_str(test2) == std::string("[ [ 1, 2 ], [ 3 ], [ 4, 5, 6 ] ]"));
    CHECK(utl::stre::to_str(test3) == std::string("< 17, 0.5, text >"               ));
    CHECK(utl::stre::to_str(test4) == std::string("[ < key1, 1 >, < key2, 2 > ]"    ));
}

TEST_CASE("Formatting utils perform as expected") {
    
    std::string result = (utl::stre::InlineStream() << "Value is " << 3 << '\n');
    CHECK(result == std::string("Value is 3\n"));
    
    CHECK(utl::stre::pad_with_zeroes(15,   4) == std::string("0015"));
    CHECK(utl::stre::pad_with_zeroes(137,  3) == std::string("137" ));
    CHECK(utl::stre::pad_with_zeroes(4321, 2) == std::string("4321"));
    
    CHECK(utl::stre::repeat_symbol('k', 6) == std::string("kkkkkk"));
    CHECK(utl::stre::repeat_symbol('k', 0) == std::string(""));
    
    CHECK(utl::stre::repeat_string("xo", 3) == std::string("xoxoxo"));
    CHECK(utl::stre::repeat_string("xo", 0) == std::string(""));
}