#include <filesystem>
#include <string>
#include <string_view>
#include <limits>

// Note:
// This is a common include for all tests,
// it exists purely to reduce boilerplate and shouldn't be included anywhere else.

namespace fs = std::filesystem;

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace utl {}
using namespace utl;

template <class Func>
bool check_if_throws(Func f) {
    bool throws = false;
    try {
        f();
    } catch (...) { throws = true; }
    return throws;
}

template<class T>
using nlim = std::numeric_limits<T>;