#include <filesystem>
#include <string>
#include <limits>

namespace fs = std::filesystem;

using namespace std::string_literals;
using namespace std::string_view_literals;

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