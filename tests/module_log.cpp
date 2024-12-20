// _______________ TEST FRAMEWORK & MODULE  _______________

#include <cstdint>
#include <limits>
#include <ostream>
#include <utility>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "module_log.hpp"

// _______________________ INCLUDES _______________________

#include <array>         // testing stringification
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

std::ostream& operator<<(std::ostream& os, Printable value) { return os << "printable_value"; }

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
    CHECK(log::stringify(17u) == "17");
    CHECK(log::stringify(8ul) == "8");
    CHECK(log::stringify(-450) == "-450");
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::int8_t>::max()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::int8_t>::min()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::int16_t>::max()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::int16_t>::min()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::int32_t>::max()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::int32_t>::min()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::int64_t>::max()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::int64_t>::min()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::uint8_t>::max()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::uint8_t>::min()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::uint16_t>::max()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::uint16_t>::min()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::uint32_t>::max()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::uint32_t>::min()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::uint64_t>::max()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<std::uint64_t>::min()); }));
    // Float
    CHECK(log::stringify(0.5) == "0.5");
    CHECK(log::stringify(-1.5) == "-1.5");
    CHECK(log::stringify(0.) == "0");
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<float>::max()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<float>::min()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<double>::max()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<double>::min()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<long double>::max()); }));
    CHECK(!check_if_throws([]{ log::stringify(std::numeric_limits<long double>::min()); }));
    // String view-convertible
    CHECK(log::stringify("lorem ipsum") == "lorem ipsum");
    CHECK(log::stringify("lorem ipsum"s) == "lorem ipsum");
    CHECK(log::stringify("lorem ipsum"sv) == "lorem ipsum");
    // String-convertible
    CHECK(log::stringify(fs::path("lorem/ipsum")) == "lorem/ipsum");
    // Array-like
    //CHECK(log::stringify(std::array{1, 2, 3}) == "{ 1, 2, 3 }");
    CHECK(log::stringify(std::vector{1, 2, 3}) == "{ 1, 2, 3 }");
    CHECK(log::stringify(std::set{1, 2, 3}) == "{ 1, 2, 3 }");
    // Tuple-like
    CHECK(log::stringify(std::pair{1, 2}) == "< 1, 2 >");
    CHECK(log::stringify(std::tuple{"lorem", 2, "ipsum"}) == "< lorem, 2, ipsum >");
    // Printable
    CHECK(log::stringify(Printable{}) == "printable_value");
    
    // Compound types
    CHECK(log::stringify(std::map{ std::pair{"k1", 1}, std::pair{"k2", 2} }) == "{ < k1, 1 >, < k2, 2 > }");
    CHECK(log::stringify(std::vector{ std::vector{ 1, 2 }, std::vector{ 3 } }) == "{ { 1, 2 }, { 3 } }");
    CHECK(log::stringify(std::vector<std::vector<std::vector<const char *>>>{{{ "lorem" }}}) == "{ { { lorem } } }");
}