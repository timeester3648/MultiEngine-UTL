// _______________ TEST FRAMEWORK & MODULE  _______________

#include <cstdint>
#include <limits>
#include <ostream>
#include <string>
#include <utility>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "module_log.hpp"

// _______________________ INCLUDES _______________________

#include <array>         // testing stringification
#include <complex>       // testing stringification
#include <filesystem>    // testing stringification
#include <map>           // testing stringification
#include <set>           // testing stringification
#include <unordered_map> // testing stringification
#include <unordered_set> // testing stringification
#include <vector>        // testing stringification

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

using namespace utl;
namespace fs = std::filesystem;

template <class Func>
bool check_if_throws(Func f) {
    bool throws = false;
    try {
        f();
    } catch (...) { throws = true; }
    return throws;
}

// ==================================
// --- RFC-8259 conformance tests ---
// ==================================

struct Printable {};

struct NonIncrementableWithIterator {
    int begin() const { return 1; }
};

std::ostream& operator<<(std::ostream& os, Printable value) { return os << "printable_value"; }

template<class T>
using nlim = std::numeric_limits<T>;

TEST_CASE("Stringification") {
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    // Bool
    CHECK(log::stringify(false) == "false");
    CHECK(log::stringify(true) == "true");
    
    // Char
    CHECK(log::stringify('g') == "g");
    
    // Integer
    CHECK(log::stringify(0) == "0");
    CHECK(log::stringify(-450) == "-450");
    
    CHECK(log::stringify(-17) == "-17");
    CHECK(log::stringify(17u) == "17");
    CHECK(log::stringify(-17l) == "-17");
    CHECK(log::stringify(17ul) == "17");
    CHECK(log::stringify(-17ll) == "-17");
    CHECK(log::stringify(17ull) == "17");

    CHECK(log::stringify(nlim<std::int8_t >::min()) == std::to_string(nlim<std::int8_t >::min()));
    CHECK(log::stringify(nlim<std::int16_t>::min()) == std::to_string(nlim<std::int16_t>::min()));
    CHECK(log::stringify(nlim<std::int32_t>::min()) == std::to_string(nlim<std::int32_t>::min()));
    CHECK(log::stringify(nlim<std::int64_t>::min()) == std::to_string(nlim<std::int64_t>::min()));
    
    CHECK(log::stringify(nlim<std::int8_t >::max()) == std::to_string(nlim<std::int8_t >::max()));
    CHECK(log::stringify(nlim<std::int16_t>::max()) == std::to_string(nlim<std::int16_t>::max()));
    CHECK(log::stringify(nlim<std::int32_t>::max()) == std::to_string(nlim<std::int32_t>::max()));
    CHECK(log::stringify(nlim<std::int64_t>::max()) == std::to_string(nlim<std::int64_t>::max()));
    
    CHECK(log::stringify(nlim<std::uint8_t >::min()) == std::to_string(nlim<std::uint8_t >::min()));
    CHECK(log::stringify(nlim<std::uint16_t>::min()) == std::to_string(nlim<std::uint16_t>::min()));
    CHECK(log::stringify(nlim<std::uint32_t>::min()) == std::to_string(nlim<std::uint32_t>::min()));
    CHECK(log::stringify(nlim<std::uint64_t>::min()) == std::to_string(nlim<std::uint64_t>::min()));
    
    CHECK(log::stringify(nlim<std::uint8_t >::max()) == std::to_string(nlim<std::uint8_t >::max()));
    CHECK(log::stringify(nlim<std::uint16_t>::max()) == std::to_string(nlim<std::uint16_t>::max()));
    CHECK(log::stringify(nlim<std::uint32_t>::max()) == std::to_string(nlim<std::uint32_t>::max()));
    CHECK(log::stringify(nlim<std::uint64_t>::max()) == std::to_string(nlim<std::uint64_t>::max()));
    
    // Float
    CHECK(log::stringify(0.5) == "0.5");
    CHECK(log::stringify(-1.5) == "-1.5");
    CHECK(log::stringify(0.) == "0");
    CHECK(!check_if_throws([] { return log::stringify(std::numeric_limits<float>::max()); }));
    CHECK(!check_if_throws([] { return log::stringify(std::numeric_limits<float>::min()); }));
    CHECK(!check_if_throws([] { return log::stringify(std::numeric_limits<double>::max()); }));
    CHECK(!check_if_throws([] { return log::stringify(std::numeric_limits<double>::min()); }));
    CHECK(!check_if_throws([] { return log::stringify(std::numeric_limits<long double>::max()); }));
    CHECK(!check_if_throws([] { return log::stringify(std::numeric_limits<long double>::min()); }));
    
    // Complex
    CHECK(log::stringify(std::complex{1, 2}) == "1 + 2 i");
    
    // String view-convertible
    CHECK(log::stringify("lorem ipsum") == "lorem ipsum");
    CHECK(log::stringify("lorem ipsum"s) == "lorem ipsum");
    CHECK(log::stringify("lorem ipsum"sv) == "lorem ipsum");
    
    // String-convertible
    CHECK(log::stringify(fs::path("lorem/ipsum")) == "lorem/ipsum");
    
    // Array-like
    CHECK(log::stringify(std::array{1, 2, 3}) == "{ 1, 2, 3 }");
    CHECK(log::stringify(std::vector{1, 2, 3}) == "{ 1, 2, 3 }");
    CHECK(log::stringify(std::set{1, 2, 3}) == "{ 1, 2, 3 }");
    
    // Tuple-like
    CHECK(log::stringify(std::pair{1, 2}) == "< 1, 2 >");
    CHECK(log::stringify(std::tuple{"lorem", 2, "ipsum"}) == "< lorem, 2, ipsum >");
    
    // Printable
    CHECK(log::stringify(Printable{}) == "printable_value");

    // Compound types
    CHECK(log::stringify(std::map{
              std::pair{"k1", 1},
              std::pair{"k2", 2}
    }) == "{ < k1, 1 >, < k2, 2 > }");
    CHECK(log::stringify(std::vector{
              std::vector{1, 2},
              std::vector{3}
    }) == "{ { 1, 2 }, { 3 } }");
    CHECK(log::stringify(std::vector<std::vector<std::vector<const char*>>>{{{"lorem"}}}) == "{ { { lorem } } }");

    using array_t           = std::array<int, 3>;
    using iter              = array_t::iterator;
    constexpr bool has_it_1 = log::_has_input_iter_v<int>;
    constexpr bool has_it_2 = log::_has_input_iter_v<NonIncrementableWithIterator>;
    constexpr bool has_it_3 = log::_has_input_iter_v<array_t>;

    std::next(std::array{1, 2, 3}.begin());
}