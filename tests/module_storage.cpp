// __________ TEST FRAMEWORK & LIBRARY  __________

#include <numeric>
#include <vector>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#define UTL_PICK_MODULES
#define UTLMODULE_STORAGE
#include "proto_utils.hpp"

// ________________ TEST INCLUDES ________________

#include <stdexcept>
#include <algorithm>

// _____________ TEST IMPLEMENTATION _____________

TEST_CASE("Matrix constructors & methods derived from storage::AbstractIndexableObject behave as expected") {
        
    SUBCASE("Matrix default-initialization is correct") {
        utl::storage::Matrix<int> matrix(12, 5);
        bool all_elements_are_zero = true;
        matrix.for_each([&](int &element) { if (element != 0) all_elements_are_zero = false; });
        CHECK(all_elements_are_zero);
    }
    
    
    SUBCASE("Matrix value-initialization is correct") {
        utl::storage::Matrix<std::string> matrix(12, 5, "xo");
        bool all_elements_are_correct = true;
        matrix.for_each([&](std::string &element) { if (element != "xo") all_elements_are_correct = false; });
        CHECK(all_elements_are_correct);
    }
    
    
    SUBCASE("Matrix 1D indexation") {
        constexpr std::size_t rows = 3, cols = 2, size = rows * cols;
        utl::storage::Matrix<int> matrix(rows, cols, 1);
        const auto &cref = matrix;
        // Reading
        int sum = 0;
        for (std::size_t idx = 0; idx < size; ++idx) sum += cref[idx];
        CHECK(sum == size);
        // Writing
        sum = 0;
        for (std::size_t idx = 0; idx < size; ++idx) sum += (matrix[idx] = -1); // also tests operator=() returns
        CHECK(-sum == size);
    }
    
    
    SUBCASE("Matrix 2D indexation") {
        constexpr std::size_t rows = 3, cols = 4, size = rows * cols;
        utl::storage::Matrix<float> matrix(rows, cols, 0.5f);
        const auto &cref = matrix;
        // Reading
        decltype(matrix)::value_type sum = 0.0f; // also tests the types
        for (std::size_t i = 0; i < rows; ++i)
            for (std::size_t j = 0; j < cols; ++j)
                sum += cref(i, j);
        CHECK(sum == doctest::Approx(size * 0.5f));
        // Writing
        sum = 0.0f;
        for (std::size_t i = 0; i < rows; ++i)
            for (std::size_t j = 0; j < cols; ++j)
                sum += (matrix(i, j) = 1.4f);
        CHECK(sum == doctest::Approx(size * 1.4f));
    }
    
    
    SUBCASE("Matrix .fill() + .for_each() + .size() test") {
        // .fill() matrix with 1's and check that their .for_each()-computed sum equals .size()
        auto matrix = std::move(utl::storage::Matrix<int>(15, 7).fill(1));
            // triggers copy without std::move()
            // By doing this with 'auto' we test .fill() CRTP return type and Matrix move operator=
        int sum = 0;
        matrix.for_each([&](int &element) { sum += element; });
        CHECK(sum == matrix.size());
    }
    
    
    SUBCASE("Matrix .for_each() (with idx) test") {
        // Fill matrix elements with their corresponding 1D indexes and check that it is correct
        auto matrix = utl::storage::Matrix<int>(3, 5);
        matrix.for_each([](int &element, std::size_t idx) { element = idx; });
        for (std::size_t i = 0; i < matrix.size(); ++i) CHECK(matrix[i] == i);
    }
    
    SUBCASE("Matrix .for_each() (with i, j) test") {
        // Fill matrix elements with their corresponding 1D indexes and check that it is correct
        auto matrix = utl::storage::Matrix<int>(3, 5);
        matrix.for_each([](int &element, std::size_t i, std::size_t j) { element = 10 * i + j; });
        for (std::size_t i = 0; i < matrix.rows(); ++i)
            for (std::size_t j = 0; j < matrix.cols(); ++j)
                CHECK(matrix(i, j) == (10 * i + j));
    }
    
    SUBCASE("Matrix .front() and .back() test") {
        // Fill matrix elements with their corresponding 1D indexes and check that it is correct
        utl::storage::Matrix<char> matrix(4, 13);
        matrix.front() = 'F';
        matrix.back() = 'B';
        CHECK(matrix[0]                 == matrix.front());
        CHECK(matrix[0]                 == 'F'           );
        CHECK(matrix[matrix.size() - 1] == matrix.back() );
        CHECK(matrix[matrix.size() - 1] == 'B'           );
    }
        
    SUBCASE("Matrix .to_std_vector() + initializer_list constructor test") {
        utl::storage::Matrix<char> matrix = { { 'a', 'b' }, { 'c', 'd' } };
        matrix.to_std_vector();
        CHECK(matrix.to_std_vector() == std::vector{ 'a', 'b', 'c', 'd' });
    }
    
    
    SUBCASE("Matrix initializer_list constructor (throw case) test") {
        bool caught_appropriate_exception = false;
        try {
            utl::storage::Matrix<int> matrix({
                {  1,  2,  3,  4 },
                {  5,  6,  7     }, // missing an element
                {  8,  9, 10, 11 }
            });
        } catch (std::invalid_argument const& ex) {
            caught_appropriate_exception = true;
            INFO("Caught exception: ", ex.what(), "\n");
        }
        CHECK(caught_appropriate_exception);
    }
    
    SUBCASE("Matrix const & non-const overloads test") {
        utl::storage::Matrix<int> matrix = { { 1, 2 }, { 3, 4 }, { 5, 6 } };
        const auto &cref = matrix;
        CHECK(matrix[0] == cref[0]);
        CHECK(matrix(0, 1) == cref(0, 1));
        CHECK(matrix.front() == cref.front());
        CHECK(matrix.back() == cref.back());
    }
}


TEST_CASE("Basic matrix methods behave as expected") {  
       
    SUBCASE("Matrix 2D bound-checking test") {
        utl::storage::Matrix<int, utl::storage::BoundChecking::ENABLED> matrix(4, 5);
        // 1D access
        auto access_idx_throws = [&](std::size_t idx) -> bool {
            try {
                matrix[idx] = 0;
            } catch (std::out_of_range const& ex) {
                INFO("Caught exception: ", ex.what(), "\n");
                return true;
            }
            return false;
        };
        CHECK(access_idx_throws(0)  == false);
        CHECK(access_idx_throws(7)  == false);
        CHECK(access_idx_throws(19) == false);
        CHECK(access_idx_throws(20) == true );
        CHECK(access_idx_throws(34) == true );
        // 2D access
        auto access_ij_throws = [&](std::size_t i, std::size_t j) -> bool {
            try {
                matrix(i, j) = 0;
            } catch (std::out_of_range const& ex) {
                INFO("Caught exception: ", ex.what(), "\n");
                return true;
            }
            return false;
        };
        CHECK(access_ij_throws( 0,  0) == false);
        CHECK(access_ij_throws( 3,  4) == false);
        CHECK(access_ij_throws( 2,  1) == false);
        CHECK(access_ij_throws( 4,  0) == true );
        CHECK(access_ij_throws( 0,  5) == true );
        CHECK(access_ij_throws(10, 15) == true );
    }
    
    SUBCASE("Matrix CHECKED -> UNCHECKED conversion test") {
        utl::storage::Matrix<int, utl::storage::BoundChecking::ENABLED>    checked_matrix = { { 1, 2 }, { 3, 4 } };
        utl::storage::Matrix<int, utl::storage::BoundChecking::DISABLED> unchecked_matrix;
        
        // CHECKED -> UNCHECKED move
        CHECK(  checked_matrix.empty() == false);
        CHECK(unchecked_matrix.empty() == true );
        
        // unchecked_matrix = checked_matrix; <- will not work, conversion over boundaries has to be explicit!
        unchecked_matrix = utl::storage::Matrix<int, utl::storage::BoundChecking::DISABLED>(std::move(checked_matrix));
        
        // NOTE: No guarantees about moved-from matrix being empty!
        CHECK(unchecked_matrix.empty() == false);
        
        CHECK(unchecked_matrix(0, 0) == 1);
        CHECK(unchecked_matrix(0, 1) == 2);
        CHECK(unchecked_matrix(1, 0) == 3);
        CHECK(unchecked_matrix(1, 1) == 4);
    }
}


TEST_CASE("Matrix views behave as expected") {
    
    
    SUBCASE("Can create const matrix view from matrix and read the contents") {
        utl::storage::Matrix<int> matrix = { { 1, 2 }, { 3, 4 } };
        utl::storage::ConstMatrixView<int> view(matrix);
        
        // Indexing a view gives the same values
        CHECK(view(0, 0) == 1);
        CHECK(view(0, 1) == 2);
        CHECK(view(1, 0) == 3);
        CHECK(view(1, 1) == 4);
        // Sensible assumptions work
        CHECK(view.data() == matrix.data());
        CHECK(view.to_std_vector() == matrix.to_std_vector());
    }
    
    SUBCASE("Can create const matrix view from matrix and read the contents") {
        constexpr std::size_t rows = 3, cols = 4, size = rows * cols;
        std::vector<int> original_vector(size, 17); 
        const int* const_ptr_to_data = original_vector.data();
        
        // Create view to raw data and use it like a matrix
        utl::storage::ConstMatrixView<int> view(rows, cols, const_ptr_to_data);
        // All elements are correct
        for (std::size_t i = rows; i < rows; ++i)
            for (std::size_t j = cols; j < cols; ++j)
                CHECK(view(i, j) == 17);
        // Sensible assumptions work
        CHECK(view.data() == original_vector.data());
        CHECK(view.to_std_vector() == original_vector);
        // Const version of .for_each
        decltype(view)::value_type sum = 0;
        view.for_each([&](const int& element) { sum += element; });
        CHECK(sum == size * 17);
    }
    
    
    SUBCASE("Can create mutable matrix view and change the contents of matrix") {
        utl::storage::Matrix<int> matrix = { { 1, 2 }, { 3, 4 } };
        
        auto view = matrix.get_view();
        
        // Indexing a view gives the same values
        CHECK(view(0, 0) == 1);
        CHECK(view(0, 1) == 2);
        CHECK(view(1, 0) == 3);
        CHECK(view(1, 1) == 4);
        // Sensible assumptions work
        CHECK(view.data() == matrix.data());
        CHECK(view.to_std_vector() == matrix.to_std_vector());
        // Can modify matrix through a view
        view.fill(7);
        matrix.for_each([](int &element) { CHECK(element == 7); });
        view[0]     = 5;
        view(1, 0)  = 3;
        view.back() = 7;
        CHECK(matrix(0, 0) == 5);
        CHECK(matrix(1, 0) == 3);
        CHECK(matrix(1, 1) == 7);
    }
    
    
    SUBCASE("Can create bound-checked view from unchecked matrix") {
        utl::storage::Matrix<int, utl::storage::BoundChecking::DISABLED> unchecked_matrix = { { 1, 2 }, { 3, 4 } };
        
        // Create bound-checked view to matrix
        auto checked_view = unchecked_matrix.get_checked_view();
        
        auto access_ij_throws = [&](std::size_t i, std::size_t j) -> bool {
            try {
                checked_view(i, j) = 0;
            } catch (std::out_of_range const& ex) {
                INFO("Caught exception: ", ex.what(), "\n");
                return true;
            }
            return false;
        };
        CHECK(access_ij_throws( 0,  0) == false);
        CHECK(access_ij_throws( 1,  0) == false);
        CHECK(access_ij_throws( 1,  1) == false);
        CHECK(access_ij_throws( 2,  0) == true );
        CHECK(access_ij_throws( 0,  2) == true );
        CHECK(access_ij_throws(13, 14) == true );
    }
    
    
    SUBCASE("Can create view from const matrix reference and chain functions") {
        utl::storage::Matrix<int> matrix = { { 1, 2 }, { 3, 4 } };
        const auto &cref = matrix;
        
        // Create view from const reference
        auto view = cref.get_const_view();
        // Basic assumptions
        CHECK(view.rows() == matrix.rows());
        CHECK(view.cols() == matrix.cols());
        CHECK(view.size() == matrix.size());
        CHECK(view.data() == matrix.data());
        // Try abusing function chains
        int sum = 1;
        CHECK(
            matrix
            .for_each([&](const int &element, std::size_t idx) { sum *= element * (idx + 1); })
            .for_each([&](const int &element, std::size_t idx) { sum -= element; })
            .back() == 4
        );
        CHECK(sum == 566); // computed by hand
    }
}


TEST_CASE("Iterators behave as expected") {
    
    
    SUBCASE("Const iterator works") {
        utl::storage::Matrix<int> matrix(4, 5);
        matrix.for_each([](int &element, std::size_t idx) { element = idx; });
        const auto &cref = matrix;
        // Check that all filled values are read the same when const-iterating though
        std::size_t idx = 0;
        for(auto it = cref.cbegin(); it != cref.cend(); ++it, ++idx) CHECK(*it == idx);
        // Check that we covered the whole array
        CHECK(idx == cref.size());
        // Check basic operations of random access iterator
        CHECK(*cref.cbegin() == cref.front());                     // *it
        CHECK(cref.cend() - cref.cbegin() == cref.size());         // it - it
        CHECK(*(cref.cbegin() + 1) == 1);                          // it + n
        CHECK(*(2 + cref.cbegin()) == 2);                          // n + it
        CHECK(*(cref.cend() - 1) == cref.back());                  // it - n
        CHECK(cref.cbegin()[2] == 2);                              // it[n]
        CHECK(cref.cbegin() < cref.cend());                        // it1 < it2
        CHECK(cref.cend() > cref.cbegin());                        // it1 > it2
        CHECK(cref.cbegin() <= cref.cend());                       // it1 <= it2
        CHECK(cref.cend() >= cref.cbegin());                       // it1 >= it2
        CHECK(*(cref.cbegin() += 2) == 2);                         // it1 += n
        CHECK(*(cref.cend() -= 1) == matrix.back());               // it1 -= n
        // Check that operations don't mess up when we use derived reverse iterator
        CHECK(cref.crbegin() < cref.crend());                        // it1 < it2
        CHECK(cref.crend() > cref.crbegin());                        // it1 > it2
        // Iterate in reverse
        matrix.for_each([&](int &element, std::size_t idx) { element = idx; }); // [ 0 .. N-1 ]
        for (auto it = matrix.rbegin(); it != matrix.rend(); ++it) CHECK(matrix.rend() - 1 - it == *it); // true for how we filled matrix
            // note that reverse_iterator also reverses the logic of 'it1 - it2', makes sense because '.crbegin() < .crend()'
    }
    
    
    SUBCASE("Mutable iterator works") {
        utl::storage::Matrix<int> matrix(4, 5);
        // Check that range-based for now works (it's a syntactic sugar defined for all containers with 
        // defined forward iterators and .begin(), .end() methods) (doesn't call .cbegin() & .cend() even if they exist)
        for (auto &element : matrix) element = 7;
        for (const auto &element : matrix) CHECK(element == 7);
        // Check basic operations of random access iterator
        matrix.for_each([](int &element, std::size_t idx) { element = idx; });
        CHECK(*matrix.begin() == matrix.front());                     // *it
        CHECK(matrix.end() - matrix.begin() == matrix.size());         // it - it
        CHECK(*(matrix.begin() + 1) == 1);                          // it + n
        CHECK(*(2 + matrix.begin()) == 2);                          // n + it
        CHECK(*(matrix.end() - 1) == matrix.back());                  // it - n
        CHECK(matrix.begin()[2] == 2);                              // it[n]
        CHECK(matrix.begin() < matrix.end());                        // it1 < it2
        CHECK(matrix.end() > matrix.begin());                        // it1 > it2
        CHECK(matrix.begin() <= matrix.end());                       // it1 <= it2
        CHECK(matrix.end() >= matrix.begin());                       // it1 >= it2
        CHECK(*(matrix.begin() += 2) == 2);                         // it1 += n
        CHECK(*(matrix.end() -= 1) == matrix.back());               // it1 -= n
        // Check that operations don't mess up when we use derived reverse iterator
        CHECK(matrix.rbegin() < matrix.rend());                        // it1 < it2
        CHECK(matrix.rend() > matrix.rbegin());                        // it1 > it2
        // Check that algorithms works
        // std::sort
        matrix.for_each([&](int &element, std::size_t idx) { element = matrix.size() - 1 - idx; }); // [ N-1 .. 0 ]
        std::sort(matrix.begin(), matrix.end());                                                    // becomes [ 0 ... N-1 ]
        matrix.for_each([&](int &element, std::size_t idx) { CHECK(element == idx); });
        // std::fill + std::accumulate
        std::fill(matrix.begin(), matrix.end(), 10);
        const int sum_1 = std::accumulate(matrix.begin(), matrix.end(), 0);
        int sum_2 = 0;
        matrix.for_each([&](const int &element) { sum_2 += element; });
        CHECK(sum_1 == sum_2);
        // Iterate in reverse
        matrix.for_each([&](int &element, std::size_t idx) { element = matrix.size() - 1 - idx; }); // [ N-1 .. 0 ]
        for (auto it = matrix.rbegin(); it != matrix.rend(); ++it) CHECK(it - matrix.rbegin() == *it); // true for how we filled matrix
    }
}


TEST_CASE("Col-major ordering and transposition behave as expected") {
    
    SUBCASE("matrix.transposed() behaves as expected") {
        utl::storage::Matrix<int> matrix(3, 5, 0);
        matrix.for_each([](int& element, std::size_t i, std::size_t j) { element = 1000 * i + j; });
        // Transpose
        auto matrixT = matrix.transposed();
        // Check sizes
        CHECK(matrix.rows() == matrixT.cols());
        CHECK(matrix.cols() == matrixT.rows());
        // Check contents
        for (std::size_t i = 0; i < matrix.rows(); ++i)
            for (std::size_t j = 0; j < matrix.cols(); ++j)
                CHECK(matrix(i, j) == matrixT(j, i));
    }
}