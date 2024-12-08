// __________ BENCHMARK FRAMEWORK & LIBRARY  __________

#include "benchmark.hpp"
#include "thirdparty/Eigen/Sparse"
#include "thirdparty/Eigen/src/SparseCore/SparseMatrix.h"
#include "thirdparty/Eigen/src/SparseCore/SparseUtil.h"

#include <array>
#include <cstddef>
#include <set>

// _____________ BENCHMARK IMPLEMENTATION _____________

using SparseMat = mvl::SparseMatrix<double>;

// SparseMat matmul(const SparseMat &A, const SparseMat &B) {
//     assert(A.cols() == B.rows());

//     // // Convert triplets to Eigen format (can be avoided by implementing necessary interface in mvl::SparseEnty2D)
//     // std::vector<Eigen::Triplet<SparseMat::value_type>> triplets_A;
//     // triplets_A.reserve(mvl_A.size());
//     // for (const auto &e : mvl_A._data) triplets_A.emplace_back(e.i, e.j, e.value);
//     //
//     // // Create Eigen matrix
//     // Eigen::SparseMatrix<SparseMat::value_type> A(mvl_A.rows(), mvl_A.cols());
//     // A.setFromTriplets(triplets_A.begin(), triplets_A.end());
//     // // docs state that 'setFromSortedTriplets()' also exists, but I can't see it
//     //
//     // std::vector<Eigen::Triplet<SparseMat::value_type>> triplets_B;
//     // triplets_B.reserve(mvl_B.size());
//     // for (const auto &e : mvl_B._data) triplets_A.emplace_back(e.i, e.j, e.value);
//     //
//     // Eigen::SparseMatrix<SparseMat::value_type> B(mvl_A.rows(), mvl_A.cols());
//     // B.setFromTriplets(triplets_B.begin(), triplets_B.end());
//     //
//     // Eigen::SparseMatrix<SparseMat::value_type> C = A * B;
//     //
//     // // But how do I get triplets from Eigen matrix to convert it back?

//     // Dense matrix multiplication
//     //SparseMat C;
//     //for (size_t i = 0; i < mvl_A.rows(); ++i)
//     //    for (size_t j = 0; j < mvl_B.cols(); ++j)
//     //	    for (size_t k = 0; k < mvl_A.cols(); ++k) // swapping j <-> k rows for dense matrices gives a performace
//     boost
//     //			C(i, j) += mvl_A(i, k) * mvl_B(k, j);

//     // mvl::Matrix<double> C = mvl::Matrix<double>(A) * mvl::Matrix<double>(B);
//     // return mvl::SparseMatrix<double>{C};
// }

double sum_regular(const std::vector<double>& vec) {
    double s = 0;
    for (std::size_t i = 0; i < vec.size(); ++i) s += vec[i];
    return s;
}

#define UNROLL(j_) s1 += vec[i + j_]

double sum_unroll_2(const std::vector<double>& vec) {
    double s1 = 0, s2 = 0;

    std::size_t i;
    for (i = 0; i < vec.size(); i += 2) {
        s1 += vec[i + 0];
        s2 += vec[i + 1];
    }
    for (; i < vec.size(); ++i) s1 += vec[i];

    return s1 + s2;
}

double sum_unroll_4(const std::vector<double>& vec) {
    double s1 = 0, s2 = 0, s3 = 0, s4 = 0;

    std::size_t i;
    for (i = 0; i < vec.size(); i += 4) {
        s1 += vec[i + 0];
        s2 += vec[i + 1];
        s3 += vec[i + 2];
        s4 += vec[i + 3];
    }
    for (; i < vec.size(); ++i) s1 += vec[i];

    return s1 + s2 + s3 + s4;
}

double sum_unroll_8(const std::vector<double>& vec) {
    double s1 = 0, s2 = 0, s3 = 0, s4 = 0, s5 = 0, s6 = 0, s7 = 0, s8 = 0;

    std::size_t i;
    for (i = 0; i < vec.size(); i += 8) {
        s1 += vec[i + 0];
        s2 += vec[i + 1];
        s3 += vec[i + 2];
        s4 += vec[i + 3];
        s5 += vec[i + 4];
        s6 += vec[i + 5];
        s7 += vec[i + 6];
        s8 += vec[i + 7];
    }
    for (; i < vec.size(); ++i) s1 += vec[i];

    return s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8;
}

double sum_unroll_8_array(const std::vector<double>& vec) {
    std::array<double, 8> s{};

    std::size_t i;
    for (i = 0; i < vec.size(); i += 8) {
        s[0] += vec[i + 0];
        s[1] += vec[i + 1];
        s[2] += vec[i + 2];
        s[3] += vec[i + 3];
        s[4] += vec[i + 4];
        s[5] += vec[i + 5];
        s[6] += vec[i + 6];
        s[7] += vec[i + 7];
    }
    // Add the remaining elements
    for (; i < vec.size(); ++i) s[0] += vec[i];
    // Collect partial sums into a final one
    for (i = 1; i < s.size(); ++i) s[0] += s[i];

    return s[0];
}

double sum_unroll_8_pragma(const std::vector<double>& vec) {
    double s = 0;

    #ifdef UTL_PREDEF_COMPILER_IS_GCC
    #pragma GCC unroll 8
    #endif
    for (std::size_t i = 0; i < vec.size(); ++i) s += vec[i];

    return s;
}

template <int begin, int end>
struct copy_;

template <int end>
struct copy_<end, end> {
  template <typename T, typename U>
  static void execute(T& a, const U& b) {
    a[end] += b[end];
  }
};

template <int begin, int end>
struct copy_ {
  template <typename T, typename U>
  static void execute(T& a, const U& b) {
    a[begin] += b[begin];
    copy_<begin+1, end>::execute(a, b);
  }
};

template <int begin, int how_many>
struct copy {
  template <typename T, typename U>
  static void execute(T& a, const U& b) {
    copy_<begin, begin+how_many-1>::execute(a, b);
  }
};

// Arbitrary unroll
template<class T, T... inds, class F>
constexpr void _loop(std::integer_sequence<T, inds...>, F&& f) {
  (f(std::integral_constant<T, inds>{}), ...);// C++17 fold expression
}
template<class T, T count, class F>
constexpr void loop(F&& f) {
  _loop(std::make_integer_sequence<T, count>{}, std::forward<F>(f));
}

template <std::size_t N, std::size_t I = 0, typename F>
inline void unroll_template_lambda(F const & f) {
    if constexpr(I < N) {
        f.template operator() <I> ();
        unroll_template_lambda<N, I + 1>(f);
    }
}
// this seems to be the way for loop unrolling, but it requires C++20 for explicit template lambdas
// the difference from C++17 version is that unroll index is now constexpr template argument rather that
// a regular function arg

template<std::size_t unroll_size>
inline double sum_unroll_template(const std::vector<double>& vec) {
    std::array<double, unroll_size> s{};
    
    // Unroll the loop for SIMD
    std::size_t i;
    for (i = 0; i < vec.size(); i += unroll_size) {
        loop<std::size_t, unroll_size>([&s, &vec, i](std::size_t j){ s[j] += vec[i + j];});
        // note that 'i' has to be copied here, capturing it by reference leads to a slowdown
    }
    // Add the remaining elements
    for (; i < vec.size(); ++i) s[0] += vec[i];
    // Collect partial sums into a final one
    for (i = 1; i < s.size(); ++i) s[0] += s[i];
    
    return s[0];
}

void benchmark_simd_unrolling() {
    double sum = 0;
    std::vector<double>     vec(5'000'000);
    for (auto &e : vec) e = random::rand_double(-0.01, 0.01);
    
    bench.minEpochIterations(200)
        .timeUnit(millisecond, "ms")
        .title("Unrolling the sum")
        .relative(true)
        .warmup(10);
    
    benchmark("sum_regular", [&] {
        sum = 0;
        sum += sum_regular(vec);
    });
    
    log::println(sum);
    
    benchmark("sum_unroll_2", [&] {
        sum = 0;
        sum += sum_unroll_4(vec);
    });
    
    log::println(sum);
    
    benchmark("sum_unroll_4", [&] {
        sum = 0;
        sum += sum_unroll_4(vec);
    });
    
    log::println(sum);
    
    benchmark("sum_unroll_8", [&] {
        sum = 0;
        sum += sum_unroll_8(vec);
    });
    
    log::println(sum);
    
    benchmark("sum_unroll_8_array", [&] {
        sum = 0;
        sum += sum_unroll_8_array(vec);
    });
    
    log::println(sum);
    
    benchmark("sum_unroll_8_pragma", [&] {
        sum = 0;
        sum += sum_unroll_8_pragma(vec);
    });
    
    log::println(sum);
    
    benchmark("sum_unroll_template<8>", [&] {
        sum = 0;
        sum += sum_unroll_template<8>(vec);
    });
    
    log::println(sum);
}

void benchmark_indexation() {

    constexpr std::size_t N = 2000;

    double sum = 0;


    bench.minEpochIterations(4)
        .timeUnit(millisecond, "ms")
        .title("Dense vector/matrix indexation")
        .relative(true)
        .warmup(2);

    benchmark("std::vector::operator[] loop", [&] {
        std::vector<double> vec(N * N, 1.);
        for (std::size_t i = 0; i < vec.size(); ++i) sum += vec[i];
    });

    benchmark("std::vector range-based loop", [&] {
        std::vector<double> vec(N * N, 1.);
        for (const auto& e : vec) sum += e;
    });

    benchmark("std::vector iterator loop", [&] {
        std::vector<double> vec(N * N, 1.);
        for (auto it = vec.cbegin(); it != vec.cend(); ++it) sum += *it;
    });

    benchmark("mvl::Matrix::operator[] loop", [&] {
        mvl::Matrix<double> mat(N, N, 1.);
        for (std::size_t i = 0; i < mat.size(); ++i) sum += mat[i];
    });

    benchmark("mvl::Matrix range-based loop", [&] {
        mvl::Matrix<double> mat(N, N, 1.);
        for (const auto& e : mat) sum += e;
    });

    benchmark("mvl::Matrix iterator loop", [&] {
        mvl::Matrix<double> mat(N, N, 1.);
        for (auto it = mat.cbegin(); it != mat.cend(); ++it) sum += *it;
    });

    benchmark("mvl::Matrix::operator() loop", [&] {
        mvl::Matrix<double> mat(N, N, 1.);
        for (std::size_t i = 0; i < N; ++i)
            for (std::size_t j = 0; j < N; ++j) sum += mat(i, j);
    });

    benchmark("mvl::Matrix::for_each() loop", [&] {
        const mvl::Matrix<double> mat(N, N, 1.);
        mat.for_each([&](const double& e) { sum += e; });
    });

    benchmark("mvl::Matrix::for_each() (with idx) loop", [&] {
        const mvl::Matrix<double> mat(N, N, 1.);
        mat.for_each([&](const double& e, std::size_t) { sum += e; });
    });

    benchmark("mvl::Matrix::for_each() (with i, j) loop", [&] {
        mvl::Matrix<double> mat(N, N, 1.);
        mat.for_each([&](const double& e, std::size_t, std::size_t) { sum += e; });
    });

    benchmark("mvl::Matrix::for_each() (with idx) (bound-checked) loop", [&] {
        mvl::Matrix<double, mvl::Checking::BOUNDS> mat(N, N, 1.);
        mat.for_each([&](const double& e, std::size_t) { sum += e; });
    });

    benchmark("mvl::Matrix::for_each() (with i, j) (bound-checked) loop", [&] {
        mvl::Matrix<double, mvl::Checking::BOUNDS> mat(N, N, 1.);
        mat.for_each([&](const double& e, std::size_t, std::size_t) { sum += e; });
    });

    benchmark("mvl::MatrixView::for_each() (with i, j) loop", [&] {
        std::vector<double>     vec(N * N, 1.);
        mvl::MatrixView<double> view(N, N, vec.data());
        view.for_each([&](const double& e, std::size_t, std::size_t) { sum += e; });
    });
    
    DO_NOT_OPTIMIZE_AWAY(sum);
}

int main() {
    //benchmark_indexation();
    
    benchmark_simd_unrolling();
}