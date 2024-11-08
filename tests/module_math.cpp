// __________ TEST FRAMEWORK & LIBRARY  __________

#include <functional>
#include <random>
#include <string>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "module_math.hpp"

// ________________ TEST INCLUDES ________________

#include <array>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

// _____________ TEST IMPLEMENTATION _____________

using namespace utl;

TEST_CASE_TEMPLATE("(math::is_sized<T> == true) for T = ", T, //
                   std::vector<double>,                       //
                   std::array<int, 7>,                        //
                   std::unordered_map<std::string, float>,    //
                   std::string_view                           //
) {
    CHECK(math::is_sized<T>::value == true);
}

TEST_CASE_TEMPLATE("(math::is_sized<T> == false) for T = ", T, //
                   int,                                        //
                   char*                                       //
) {
    CHECK(math::is_sized<T>::value == false);
}

TEST_CASE_TEMPLATE("(math::is_addable_with_itself<T> == true) for T = ", T, //
                   int,                                                     //
                   std::size_t,                                             //
                   float,                                                   //
                   double,                                                  //
                   long double,                                             //
                   char,                                                    //
                   std::string                                              //
) {
    CHECK(math::is_addable_with_itself<T>::value == true);
}

TEST_CASE_TEMPLATE("(math::is_addable_with_itself<T> == false) for T = ", T, //
                   std::vector<int>,                                         //
                   std::unordered_map<int, float>,                           //
                   char*                                                     //
) {
    CHECK(math::is_addable_with_itself<T>::value == false);
}

TEST_CASE_TEMPLATE("(math::is_multipliable_by_scalar<T> == true) for T = ", T, //
                   int,                                                        //
                   std::size_t,                                                //
                   float,                                                      //
                   double,                                                     //
                   long double,                                                //
                   char                                                        //
) {
    CHECK(math::is_multipliable_by_scalar<T>::value == true);
}

TEST_CASE_TEMPLATE("(math::is_multipliable_by_scalar<T> == false) for T = ", T, //
                   std::vector<int>,                                            //
                   std::unordered_map<int, float>,                              //
                   char*,                                                       //
                   std::string                                                  //
) {
    CHECK(math::is_multipliable_by_scalar<T>::value == false);
}

TEST_CASE("(math::memory_size<T, size_t> == ...) for T = ") {
    using T = std::tuple<uint8_t, uint8_t, uint8_t>;
    constexpr std::size_t size = 1920 * 1080;
    CHECK(math::memory_size<T, math::MemoryUnit::MiB>(size) == doctest::Approx(size * 3 / std::pow(1024., 2)));
}

// Test 'is_function_with_signature' with 1) std::function 2) lambdas 3) functors 4) function pointers
// while checking that type conversions are respected properly
using target_arg_t       = const int&;
using target_return_t    = double;
using target_signature_t = target_return_t(target_arg_t);

using arg_convertible_t    = std::size_t;
using return_convertible_t = float;
using nonconvertible_t     = std::string_view;

// 1) std::function
using stdfunction_exact              = std::function<target_return_t(target_arg_t)>;
using stdfunction_arg_convertible    = std::function<target_return_t(arg_convertible_t)>;
using stdfunction_return_convertible = std::function<return_convertible_t(target_arg_t)>;

using stdfunction_arg_nonconvertible    = std::function<target_return_t(nonconvertible_t)>;
using stdfunction_return_nonconvertible = std::function<nonconvertible_t(target_arg_t)>;

// 2) lambdas
constexpr auto lambda_exact              = [](target_arg_t) -> target_return_t { return {}; };
constexpr auto lambda_arg_convertible    = [](arg_convertible_t) -> target_return_t { return {}; };
constexpr auto lambda_return_convertible = [](target_arg_t) -> return_convertible_t { return {}; };

constexpr auto lambda_arg_nonconvertible    = [](nonconvertible_t) -> target_return_t { return {}; };
constexpr auto lambda_return_nonconvertible = [](target_arg_t) -> nonconvertible_t { return {}; };

// 3) functors
struct functor_exact {
    target_return_t operator()(target_arg_t) { return {}; }
};
struct functor_arg_convertible {
    target_return_t operator()(arg_convertible_t) { return {}; }
};
struct functor_return_convertible {
    return_convertible_t operator()(target_arg_t) { return {}; }
};

struct functor_arg_nonconvertible {
    target_return_t operator()(nonconvertible_t) { return {}; }
};
struct functor_return_nonconvertible {
    nonconvertible_t operator()(target_arg_t) { return {}; }
};

// 4) function pointers
using functionptr_exact              = target_return_t (*)(target_arg_t);
using functionptr_arg_convertible    = target_return_t (*)(arg_convertible_t);
using functionptr_return_convertible = return_convertible_t (*)(target_arg_t);

using functionptr_arg_nonconvertible    = target_return_t (*)(nonconvertible_t);
using functionptr_return_nonconvertible = nonconvertible_t (*)(target_arg_t);

TEST_CASE_TEMPLATE("(math::is_function_with_signature<T, double(const int&)> == true) for T = ", T, //
                   stdfunction_exact,                                                               //
                   stdfunction_arg_convertible,                                                     //
                   stdfunction_return_convertible,                                                  //
                   decltype(lambda_exact),                                                          //
                   decltype(lambda_arg_convertible),                                                //
                   decltype(lambda_return_convertible),                                             //
                   functor_exact,                                                                   //
                   functor_arg_convertible,                                                         //
                   functor_return_convertible,                                                      //
                   functionptr_exact,                                                               //
                   functionptr_arg_convertible,                                                     //
                   functionptr_return_convertible                                                   //
) {
    CHECK(math::is_function_with_signature<T, double(const int&)>::value == true);
}

TEST_CASE_TEMPLATE("(math::is_function_with_signature<T, double(const int&)> == false) for T = ", T, //
                   stdfunction_arg_nonconvertible,                                                   //
                   stdfunction_return_nonconvertible,                                                //
                   decltype(lambda_arg_nonconvertible),                                              //
                   decltype(lambda_return_nonconvertible),                                           //
                   functor_arg_nonconvertible,                                                       //
                   functor_return_nonconvertible,                                                    //
                   functionptr_arg_nonconvertible,                                                   //
                   functionptr_return_nonconvertible                                                 //
) {
    CHECK(math::is_function_with_signature<T, double(const int&)>::value == false);
}

TEST_CASE("Basic math functions") {
    constexpr double eps = 1e-6; // epsilon used for double comparison, 1e-6 ~ 1e-4% allowed error

    // Standard math functions (integer case should be exact, no floating point conversion involved)
    CHECK(math::abs(4) == 4);
    CHECK(math::abs(-5) == 5);
    CHECK(math::sign(15) == 1);
    CHECK(math::sign(-4) == -1);
    CHECK(math::sqr(1) == 1);
    CHECK(math::sqr(-7) == 49);
    CHECK(math::cube(1) == 1);
    CHECK(math::cube(-3) == -27);
    CHECK(math::midpoint(20, 30) == 25);
    CHECK(math::kronecker_delta(-7, -7) == 1);
    CHECK(math::kronecker_delta(-7, -8) == 0);
    CHECK(math::power_of_minus_one(7) == -1);
    CHECK(math::power_of_minus_one(8) == 1);

    // Standard math functions (floating point case)
    CHECK(math::abs(4.f) == doctest::Approx(4.f).epsilon(eps));
    CHECK(math::abs(-5.f) == doctest::Approx(5.f).epsilon(eps));
    CHECK(math::sign(15.f) == doctest::Approx(1.f).epsilon(eps));
    CHECK(math::sign(-4.f) == doctest::Approx(-1.f).epsilon(eps));
    CHECK(math::sqr(1.f) == doctest::Approx(1.f).epsilon(eps));
    CHECK(math::sqr(-7.f) == doctest::Approx(49.f).epsilon(eps));
    CHECK(math::cube(1.f) == doctest::Approx(1.f).epsilon(eps));
    CHECK(math::cube(-3.f) == doctest::Approx(-27.f).epsilon(eps));
    CHECK(math::midpoint(20.f, 30.f) == doctest::Approx(25.f).epsilon(eps));

    // Degrees and radians
    CHECK(math::deg_to_rad(0.) == doctest::Approx(0.).epsilon(eps));
    CHECK(math::deg_to_rad(360.) == doctest::Approx(math::PI_TWO).epsilon(eps));
    CHECK(math::deg_to_rad(17 * 180.) == doctest::Approx(17. * math::PI).epsilon(eps));
    CHECK(math::deg_to_rad(-180.) == doctest::Approx(-math::PI).epsilon(eps));
    CHECK(math::rad_to_deg(0.) == doctest::Approx(0.).epsilon(eps));
    CHECK(math::rad_to_deg(math::PI_TWO) == doctest::Approx(360.).epsilon(eps));
    CHECK(math::rad_to_deg(17. * math::PI) == doctest::Approx(17 * 180.).epsilon(eps));
    CHECK(math::rad_to_deg(-math::PI) == doctest::Approx(-180.).epsilon(eps));
    
    // Meshing
    const auto grid_1 = math::linspace(0., 1., math::Points(3));
    const auto grid_2 = math::linspace(0., 1., math::Intervals(2));
    CHECK(grid_1 == grid_2);
    CHECK(grid_1.size() == 3);
    CHECK(grid_1[0] == doctest::Approx(0.0).epsilon(eps));
    CHECK(grid_1[1] == doctest::Approx(0.5).epsilon(eps));
    CHECK(grid_1[2] == doctest::Approx(1.0).epsilon(eps));
    
    const auto f = [](double x) -> double { return std::pow(x, 6); };
    const double L1 = -2.;
    const double L2 = 4.;
    const auto integral = math::integrate_trapezoidal(f, L1, L2, math::Intervals(2000));
    const auto integral_exact = std::pow(L2, 7) / 7. - std::pow(L1, 7) / 7.;
    CHECK(integral == doctest::Approx(integral_exact).epsilon(1e-4));
    
    // Misc helpers
    CHECK(math::uint_difference('a', 'b') == char(1));
    CHECK(math::uint_difference('b', 'a') == char(1));
    CHECK(math::uint_difference(15u, 18u) == 3u);
    CHECK(math::uint_difference(18u, 15u) == 3u);
    
    std::vector<int> vec(7);
    CHECK(math::ssize(vec) == 7);
    
    // Branchless ternary
    CHECK(math::ternary_branchless(true, 17u, 6u) == 17u);
    CHECK(math::ternary_branchless(false, 17u, 6u) == 6u);
    CHECK(math::ternary_bitselect(true, 8, -7) == 8);
    CHECK(math::ternary_bitselect(false, 8, -7) == -7);
    CHECK(math::ternary_bitselect(true, 9) == 9);
    CHECK(math::ternary_bitselect(false, 9) == 0);
}