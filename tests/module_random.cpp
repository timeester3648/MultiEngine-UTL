// __________ TEST FRAMEWORK & LIBRARY  __________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#define UTL_PICK_MODULES
#define UTLMODULE_RANDOM
#include "proto_utils.hpp"

// ________________ TEST INCLUDES ________________

#include <vector>
#include <algorithm>
#include <numeric>

// _____________ TEST IMPLEMENTATION _____________

double vec_mean(const std::vector<double> &vec) {
    return std::reduce(vec.cbegin(), vec.cend()) / static_cast<double>(vec.size());
}

double vec_min(const std::vector<double> &vec) {
    return static_cast<double>(*std::min_element(vec.cbegin(), vec.cend()));
}

double vec_max(const std::vector<double> &vec) {
    return static_cast<double>(*std::max_element(vec.cbegin(), vec.cend()));
}

TEST_CASE("Uniform distribution mean/min/max are sensible") {
    
    constexpr std::size_t N = 500'000; // number of random vals
    constexpr double eps = 2e-2; // epsilon used for double comparison, 1e-2 ~ 1% allowed error
    std::vector<double> vec(N);
    
    utl::random::seed(5);
    
    // Check for all utl::random:: uniform distribution functions
    // that mean/min/max are ~= their expected values
    for (auto &e : vec) e = utl::random::rand_double();
    CHECK(vec_mean(vec) == doctest::Approx(0.5).epsilon(eps));
    CHECK(vec_min(vec)  == doctest::Approx(0.0).epsilon(eps));
    CHECK(vec_max(vec)  == doctest::Approx(1.0).epsilon(eps));
    
    for (auto &e : vec) e = utl::random::rand_double(-8., 8.);
    CHECK(vec_mean(vec) == doctest::Approx( 0.0).epsilon(eps));
    CHECK(vec_min(vec)  == doctest::Approx(-8.0).epsilon(eps));
    CHECK(vec_max(vec)  == doctest::Approx( 8.0).epsilon(eps));
    
    for (auto &e : vec) e = utl::random::rand_float();
    CHECK(vec_mean(vec) == doctest::Approx(0.5).epsilon(eps));
    CHECK(vec_min(vec)  == doctest::Approx(0.0).epsilon(eps));
    CHECK(vec_max(vec)  == doctest::Approx(1.0).epsilon(eps));
    
    for (auto &e : vec) e = utl::random::rand_float(-3., 1.);
    CHECK(vec_mean(vec) == doctest::Approx(-1.0).epsilon(eps));
    CHECK(vec_min(vec)  == doctest::Approx(-3.0).epsilon(eps));
    CHECK(vec_max(vec)  == doctest::Approx( 1.0).epsilon(eps));
    
    for (auto &e : vec) e = utl::random::rand_int(-90, -80);
    CHECK(vec_mean(vec) == doctest::Approx(-85.0).epsilon(eps));
    CHECK(vec_min(vec)  == doctest::Approx(-90.0).epsilon(eps));
    CHECK(vec_max(vec)  == doctest::Approx(-80.0).epsilon(eps));
    
    for (auto &e : vec) e = utl::random::rand_uint(5, 15);
    CHECK(vec_mean(vec) == doctest::Approx(10.0).epsilon(eps));
    CHECK(vec_min(vec)  == doctest::Approx( 5.0).epsilon(eps));
    CHECK(vec_max(vec)  == doctest::Approx(15.0).epsilon(eps));
    
    for (auto &e : vec) e = static_cast<int>(utl::random::rand_bool());
    CHECK(vec_mean(vec) == doctest::Approx(0.5).epsilon(eps));
    CHECK(vec_min(vec)  == doctest::Approx(0.0).epsilon(eps));
    CHECK(vec_max(vec)  == doctest::Approx(1.0).epsilon(eps));
    
    for (auto &e : vec) e = utl::random::rand_choise({-2, -1, 0, 1, 2});
    CHECK(vec_mean(vec) == doctest::Approx( 0.0).epsilon(eps));
    CHECK(vec_min(vec)  == doctest::Approx(-2.0).epsilon(eps));
    CHECK(vec_max(vec)  == doctest::Approx( 2.0).epsilon(eps));
}