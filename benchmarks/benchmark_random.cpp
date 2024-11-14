#include "proto_utils.hpp"

#include <algorithm>
#include <numeric>
#include <vector>
#include <random>
#include <iostream>

constexpr std::size_t N = 20'000;
constexpr unsigned int rand_seed = 15;

namespace rng = utl::random;



int main() {
    
    // Grid
    std::vector<double> matrix_data(N * N);
    
    
    UTL_PROFILER("std::rand()") {
        srand(rand_seed);
        for (auto &e : matrix_data) e = std::rand() / (static_cast<double>(RAND_MAX) + 1.);
    }
    
    UTL_PROFILER("std::mt19937 + std::uniform_real_distribution") {
        std::mt19937 gen{rand_seed};
        std::uniform_real_distribution dist{0., 1.};
        for (auto &e : matrix_data) e = dist(gen);
    }
    
    UTL_PROFILER("std::minstd_rand0 + std::uniform_real_distribution") {
        std::minstd_rand0 gen{rand_seed};
        std::uniform_real_distribution dist{0., 1.};
        for (auto &e : matrix_data) e = dist(gen);
    }
    
    UTL_PROFILER("xorshift64star + rand_double()") {
        rng::xorshift64star.seed(rand_seed);
        for (auto &e : matrix_data) e = rng::rand_double();
    }
    
    constexpr double min = -10.;
    constexpr double max = 10.;
    
    UTL_PROFILER("xorshift64star + std::uniform_real_distribution") {
        rng::xorshift64star.seed(rand_seed);
        std::uniform_real_distribution dist{min, max};
        for (auto &e : matrix_data) e = dist(rng::xorshift64star);
    }
    
    UTL_PROFILER("xorshift64star + rand_double() & formula (in loop)") {
        rng::xorshift64star.seed(rand_seed);
        for (auto &e : matrix_data) {
            e = min + (max - min) * rng::rand_double();
        }
    }
    
    UTL_PROFILER("xorshift64star + std::uniform_real_distribution (in loop)") {
        rng::xorshift64star.seed(rand_seed);
        for (auto &e : matrix_data) {
            std::uniform_real_distribution dist{0., 1.};
            e = dist(rng::xorshift64star);
        }
    }
    
    // Use  matrix data so compiler doesn't decide to optimize away previous loops
    const auto mean = std::reduce(matrix_data.begin(), matrix_data.end()) / matrix_data.size();
    const auto sq_sum = std::inner_product(matrix_data.begin(), matrix_data.end(), matrix_data.begin(), 0.0);
    const auto stdev = std::sqrt(sq_sum / matrix_data.size() - mean * mean);
    
    std::cout
        << "mean = " << mean << "\n"
        << "stdev = " << stdev << "\n"
        << "min = " << *std::min_element(matrix_data.begin(), matrix_data.end()) << "\n"
        << "max = " << *std::max_element(matrix_data.begin(), matrix_data.end()) << "\n";
    
    return 0;
}