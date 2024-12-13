// __________ BENCHMARK FRAMEWORK & LIBRARY  __________

#include "benchmark.hpp"
#include "thirdparty/Eigen/Sparse"
#include "thirdparty/Eigen/src/Core/Map.h"
#include "thirdparty/Eigen/src/Core/Matrix.h"
#include "thirdparty/Eigen/src/SparseCore/SparseMatrix.h"
#include "thirdparty/Eigen/src/SparseCore/SparseUtil.h"

#include <array>
#include <cstddef>
#include <set>
#include <stdexcept>
#include <vector>

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

double sum_raw_ptr(const double* vec, std::size_t N) {
    double s = 0;
    for (std::size_t i = 0; i < N; ++i) s += vec[i];
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
        copy_<begin + 1, end>::execute(a, b);
    }
};

template <int begin, int how_many>
struct copy {
    template <typename T, typename U>
    static void execute(T& a, const U& b) {
        copy_<begin, begin + how_many - 1>::execute(a, b);
    }
};

// Arbitrary unroll
template <class T, T... inds, class F>
constexpr void _loop(std::integer_sequence<T, inds...>, F&& f) {
    (f(std::integral_constant<T, inds>{}), ...); // C++17 fold expression
}
template <class T, T count, class F>
constexpr void loop(F&& f) {
    _loop(std::make_integer_sequence<T, count>{}, std::forward<F>(f));
}

template <std::size_t N, std::size_t I = 0, typename F>
inline void unroll_template_lambda(F const& f) {
    if constexpr (I < N) {
        f.template operator()<I>();
        unroll_template_lambda<N, I + 1>(f);
    }
}
// this seems to be the way for loop unrolling, but it requires C++20 for explicit template lambdas
// the difference from C++17 version is that unroll index is now constexpr template argument rather that
// a regular function arg

template <std::size_t unroll_size>
inline double sum_unroll_template(const std::vector<double>& vec) {
    std::array<double, unroll_size> s{};

    // Unroll the loop for SIMD
    std::size_t i;
    for (i = 0; i < vec.size(); i += unroll_size) {
        loop<std::size_t, unroll_size>([&s, &vec, i](std::size_t j) { s[j] += vec[i + j]; });
        // note that 'i' has to be copied here, capturing it by reference leads to a slowdown
    }
    // Add the remaining elements
    for (; i < vec.size(); ++i) s[0] += vec[i];
    // Collect partial sums into a final one
    for (i = 1; i < s.size(); ++i) s[0] += s[i];

    return s[0];
}

void benchmark_simd_unrolling() {
    double              sum = 0;
    std::vector<double> vec(5'000'000);
    for (auto& e : vec) e = random::rand_double(-0.01, 0.01);

    bench.minEpochIterations(200).timeUnit(millisecond, "ms").title("Unrolling the sum").relative(true).warmup(10);

    benchmark("sum_regular", [&] {
        sum = 0;
        sum += sum_regular(vec);
    });

    log::println(sum);

    benchmark("sum_raw_ptr", [&] {
        sum = 0;
        sum += sum_raw_ptr(vec.data(), vec.size());
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

// ========================================
// --- Matrix multiplication benchmarks ---
// ========================================

using DenseMat = mvl::Matrix<double>;

DenseMat dd_matnul_ijk(const DenseMat& left, const DenseMat& right) {
    const std::size_t N_i = left.rows(), N_k = left.cols(), N_j = right.cols();

    DenseMat res(N_i, N_j, 0.);

    for (std::size_t i = 0; i < N_i; ++i)
        for (std::size_t j = 0; j < N_j; ++j)
            for (std::size_t k = 0; k < N_k; ++k) res(i, j) += left(i, k) * right(k, j);

    return res;
}

DenseMat dd_matnul_ijk_transposed(const DenseMat& left, DenseMat right) {
    const std::size_t N_i = left.rows(), N_k = left.cols(), N_j = right.cols();
    
    right = right.transposed();
    
    DenseMat res(N_i, N_j, 0.);

    for (std::size_t i = 0; i < N_i; ++i)
        for (std::size_t j = 0; j < N_j; ++j)
            for (std::size_t k = 0; k < N_k; ++k) res(i, j) += left(i, k) * right(j, k);

    return res;
}

DenseMat dd_matnul_ikj(const DenseMat& left, const DenseMat& right) {
    const std::size_t N_i = left.rows(), N_k = left.cols(), N_j = right.cols();

    DenseMat res(N_i, N_j, 0.);

    for (std::size_t i = 0; i < N_i; ++i)
        for (std::size_t k = 0; k < N_k; ++k) {
            const auto left_ik = left(i, k);
            for (std::size_t j = 0; j < N_j; ++j) res(i, j) += left_ik * right(k, j);
        }

    return res;
}


DenseMat dd_matnul_ikj_noinit(const DenseMat& left, const DenseMat& right) {
    const std::size_t N_i = left.rows(), N_k = left.cols(), N_j = right.cols();
    
    DenseMat res(N_i, N_j);
    
    for (std::size_t i = 0; i < N_i; ++i) {
        const auto left_i0 = left(i, 0);
        for (std::size_t j = 0; j < N_j; ++j) res(i, j) = left_i0 * right(0, j);
        
        for (std::size_t k = 1; k < N_k; ++k) {
            const auto left_ik = left(i, k);
            for (std::size_t j = 0; j < N_j; ++j) res(i, j) += left_ik * right(k, j);
        }
    }

    return res;
}

template <std::size_t block_size_kk>
DenseMat dd_matnul_ikj_kk_blocked(const DenseMat& left, const DenseMat& right) {
    const std::size_t N_i = left.rows(), N_k = left.cols(), N_j = right.cols();

    DenseMat res(N_i, N_j, 0.);

    for (std::size_t kk = 0; kk < N_k; kk += block_size_kk) {
        const auto k_extent = std::min(N_k, kk + block_size_kk);
        // needed for matrices that aren't a multiple of block size
        for (std::size_t i = 0; i < N_i; ++i) {
            for (std::size_t k = kk; k < k_extent; ++k) {
                const auto r = left(i, k);
                for (std::size_t j = 0; j < N_j; ++j) res(i, j) += r * right(k, j);
            }
        }
    }

    return res;
}

template <std::size_t block_size_ii, std::size_t block_size_kk>
DenseMat dd_matnul_ikj_ii_kk_blocked(const DenseMat& left, const DenseMat& right) {
    const std::size_t N_i = left.rows(), N_k = left.cols(), N_j = right.cols();
    
    DenseMat res(N_i, N_j, 0.);

    for (std::size_t kk = 0; kk < N_k; kk += block_size_kk) {
        const auto k_extent = std::min(N_k, kk + block_size_kk);
        for (std::size_t ii = 0; ii < N_i; ii += block_size_ii) {
            const auto i_extent = std::min(N_i, ii + block_size_ii);
            for (std::size_t i = ii; i < i_extent; ++i) {  
                for (std::size_t k = kk; k < k_extent; ++k) {
                    const auto r = left(i, k);
                    for (std::size_t j = 0; j < N_j; ++j) res(i, j) += r * right(k, j);
                }
            }
        }
    }

    return res;
}

void benchmark_matmul() {
    constexpr std::size_t N_i = 3110, N_k = 3120, N_j = 3130;

    DenseMat A(N_i, N_k, [] { return random::rand_double(-0.1, 0.1); });
    DenseMat B(N_k, N_j, [] { return random::rand_double(-0.1, 0.1); });
    DenseMat C(N_i, N_j, 0.);

    log::println("\n\n====== BENCHMARKING ON: dense/dense matmul ======\n");
    log::println("N_i               -> ", N_i);
    log::println("N_k               -> ", N_k);
    log::println("N_j               -> ", N_j);
    log::println("Data memory usage -> ", math::memory_size<double>(N_i * N_k + N_k * N_j + N_i * N_j), " MiB");

    bench.minEpochIterations(1)
        .timeUnit(millisecond, "ms")
        .title("dense/dense matrix multiplication")
        .relative(true)
        .warmup(2);

    std::vector<std::pair<std::string, double>> control_sums;

    benchmark("dd_matnul_ikj", [&] { C = dd_matnul_ikj(A, B); });
    control_sums.emplace_back("dd_matnul_ikj", C.sum());
    
    benchmark("dd_matnul_ikj_noinit", [&] { C = dd_matnul_ikj_noinit(A, B); });
    control_sums.emplace_back("dd_matnul_ikj_noinit", C.sum());

    benchmark("dd_matnul_ijk", [&] { C = dd_matnul_ijk(A, B); });
    control_sums.emplace_back("dd_matnul_ijk", C.sum());
    
    benchmark("dd_matnul_ijk_transposed", [&] { C = dd_matnul_ijk_transposed(A, B); });
    control_sums.emplace_back("dd_matnul_ijk_transposed", C.sum());

    benchmark("dd_matnul_ikj_kk_blocked<4>", [&] { C = dd_matnul_ikj_kk_blocked<4>(A, B); });
    control_sums.emplace_back("dd_matnul_ikj_kk_blocked<4>", C.sum());

    benchmark("dd_matnul_ikj_kk_blocked<8>", [&] { C = dd_matnul_ikj_kk_blocked<8>(A, B); });
    control_sums.emplace_back("dd_matnul_ikj_kk_blocked<8>", C.sum());

    benchmark("dd_matnul_ikj_kk_blocked<16>", [&] { C = dd_matnul_ikj_kk_blocked<16>(A, B); });
    control_sums.emplace_back("dd_matnul_ikj_kk_blocked<16>", C.sum());
    
    benchmark("dd_matnul_ikj_kk_blocked<32>", [&] { C = dd_matnul_ikj_kk_blocked<32>(A, B); });
    control_sums.emplace_back("dd_matnul_ikj_kk_blocked<32>", C.sum());
    
    benchmark("dd_matnul_ikj_ii_kk_blocked<4, 4>", [&] { C = dd_matnul_ikj_ii_kk_blocked<4, 4>(A, B); });
    control_sums.emplace_back("dd_matnul_ikj_ii_kk_blocked<4, 4>", C.sum());
    
    benchmark("dd_matnul_ikj_ii_kk_blocked<8, 8>", [&] { C = dd_matnul_ikj_ii_kk_blocked<8, 8>(A, B); });
    control_sums.emplace_back("dd_matnul_ikj_ii_kk_blocked<8, 8>", C.sum());
    
    benchmark("dd_matnul_ikj_ii_kk_blocked<16, 16>", [&] { C = dd_matnul_ikj_ii_kk_blocked<16, 16>(A, B); });
    control_sums.emplace_back("dd_matnul_ikj_ii_kk_blocked<16, 16>", C.sum());
    
    benchmark("dd_matnul_ikj_ii_kk_blocked<32, 32>", [&] { C = dd_matnul_ikj_ii_kk_blocked<32, 32>(A, B); });
    control_sums.emplace_back("dd_matnul_ikj_ii_kk_blocked<32, 32>", C.sum());

    // Copy data into Eigen matrices
    Eigen::MatrixXd A_eigen(N_i, N_k), B_eigen(N_k, N_j), C_eigen;
    A.for_each([&](double elem, std::size_t i, std::size_t j) { A_eigen(i, j) = elem; });
    B.for_each([&](double elem, std::size_t i, std::size_t j) { B_eigen(i, j) = elem; });

    benchmark("Eigen::Map<>::operator*", [&] { C_eigen = A_eigen * B_eigen; });
    control_sums.emplace_back("Eigen::MatrixXd::operator*", C_eigen.sum());

    // Print control sums to verify matmul correctness
    table::create({50, 20});
    table::set_formats({table::DEFAULT(), table::FIXED(8)});

    log::println();
    table::hline();
    table::cell("Benchmark", "Control sum");
    table::hline();
    for (const auto& [name, sum] : control_sums) table::cell(name, sum);
    
    // Notes:
    // Test indicate that there is no benefit whatsoever to manual unrolling as compiler is already
    // pretty good at seing SIMD opportunities.
}

// ===========================
// --- Indexing benchmarks ---
// ===========================

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
    benchmark_matmul();
    //benchmark_indexation();
    // benchmark_simd_unrolling();
}