#include <cmath>
#include <vector>

#include "module_mvl.hpp"
#include "MACRO_PROFILER.hpp"
#include "MACRO_DEFINE.hpp"
#include "thirdparty/nanobench.h"


constexpr std::size_t repeats = 2;
constexpr std::size_t N = 10000;

using namespace utl;

#define BENCHMARK(label) UTL_PROFILER(label) UTL_DEFINE_REPEAT(repeats)



int main() {
    
    double sum = 0;
    
    BENCHMARK("std::vector::operator[] loop") {
        std::vector<double> vec(N * N, 1.);
        for (std::size_t i =  0; i < vec.size(); ++i) sum += vec[i];
    }
    
    BENCHMARK("std::vector range-based loop") {
        std::vector<double> vec(N * N, 1.);
        for (const auto &e : vec) sum += e;
    }
    
    BENCHMARK("std::vector iterator loop") {
        std::vector<double> vec(N * N, 1.);
        for (auto it = vec.cbegin(); it != vec.cend(); ++it) sum += *it;
    }
    
    BENCHMARK("mvl::Matrix::operator[] loop") {
        mvl::Matrix<double> mat(N, N, 1.);
        for (std::size_t i =  0; i < mat.size(); ++i) sum += mat[i];
    }
    
    BENCHMARK("mvl::Matrix range-based loop") {
        mvl::Matrix<double> mat(N, N, 1.);
        for (const auto &e : mat) sum += e;
    }
    
    BENCHMARK("mvl::Matrix iterator loop") {
        mvl::Matrix<double> mat(N, N, 1.);
        for (auto it = mat.cbegin(); it != mat.cend(); ++it) sum += *it;
    }
    
    BENCHMARK("mvl::Matrix::operator() loop") {
        mvl::Matrix<double> mat(N, N, 1.);
        for (std::size_t i =  0; i < N; ++i) for (std::size_t j =  0; j < N; ++j) sum += mat(i, j);
    }
    
    BENCHMARK("mvl::Matrix::for_each() loop") {
        const mvl::Matrix<double> mat(N, N, 1.);
        mat.for_each([&](const double &e) { sum += e; });
    }
    
    BENCHMARK("mvl::Matrix::for_each() (with idx) loop") {
        const mvl::Matrix<double> mat(N, N, 1.);
        mat.for_each([&](const double &e, std::size_t) { sum += e; });
    }
    
    BENCHMARK("mvl::Matrix::for_each() (with i, j) loop") {
        mvl::Matrix<double> mat(N, N, 1.);
        mat.for_each([&](const double &e, std::size_t, std::size_t) { sum += e; });
    }
    
    BENCHMARK("mvl::Matrix::for_each() (with idx) (bound-checked) loop") {
        mvl::Matrix<double, mvl::Checking::BOUNDS> mat(N, N, 1.);
        mat.for_each([&](const double &e, std::size_t) { sum += e; });
    }
    
    BENCHMARK("mvl::Matrix::for_each() (with i, j) (bound-checked) loop") {
        mvl::Matrix<double, mvl::Checking::BOUNDS> mat(N, N, 1.);
        mat.for_each([&](const double &e, std::size_t, std::size_t) { sum += e; });
    }
    
    BENCHMARK("mvl::MatrixView::for_each() (with i, j) loop") {
        std::vector<double> vec(N * N, 1.);
        mvl::MatrixView<double> view(N, N, vec.data());
        view.for_each([&](const double &e, std::size_t, std::size_t) { sum += e; });
    }
    
    // Prevent compiler from optimizing away the benchmarks
    std::cout << "sum = " << sum << "\n";
    
    return 0;
}