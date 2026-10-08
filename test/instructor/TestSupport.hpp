#ifndef MATRIXCLASS_TEST_SUPPORT_HPP
#define MATRIXCLASS_TEST_SUPPORT_HPP

#include <Matrix.hpp>
#include <gtest/gtest.h>
#include <array>
#include <numeric>
#include <random>

template<class T>
void PrintTo(const Matrix<T>& value, std::ostream* output) {
    *output << value.getNumRows() << " x " << value.getNumCols() << " [";
    for (std::size_t row = 0; row < value.getNumRows(); ++row) {
        if (row != 0) *output << ", ";
        *output << '[';
        for (std::size_t col = 0; col < value.getNumCols(); ++col) {
            if (col != 0) *output << ", ";
            *output << testing::PrintToString(value.getElement(row, col));
        }
        *output << ']';
    }
    *output << ']';
}

template<class T>
Matrix<T> matrix(std::initializer_list<std::initializer_list<T>> values) {
    std::vector<std::vector<T>> rows;
    for (const auto& row : values) {
        rows.emplace_back(row);
    }
    return Matrix<T>(rows);
}

template<class T>
Matrix<T> identity(std::size_t n) {
    Matrix<T> result(n, n);
    for (std::size_t index = 0; index < n; ++index) {
        result.setElement(index, index, T{1});
    }
    return result;
}

inline void expectNear(const Matrix<double>& actual, const Matrix<double>& expected,
                       double tolerance = 1e-10) {
    ASSERT_EQ(actual.getNumRows(), expected.getNumRows());
    ASSERT_EQ(actual.getNumCols(), expected.getNumCols());
    for (std::size_t row = 0; row < actual.getNumRows(); ++row) {
        for (std::size_t col = 0; col < actual.getNumCols(); ++col) {
            const double reference = expected.getElement(row, col);
            EXPECT_TRUE(std::isfinite(actual.getElement(row, col)));
            EXPECT_NEAR(actual.getElement(row, col), reference,
                tolerance * std::max(1.0, std::abs(reference))) << row << ',' << col;
        }
    }
}

template<class T>
T permutationDeterminant(const Matrix<T>& a) {
    std::vector<std::size_t> permutation(a.getNumRows());
    std::iota(permutation.begin(), permutation.end(), 0);
    T result{};
    do {
        T term{1};
        std::size_t inversions = 0;
        for (std::size_t row = 0; row < permutation.size(); ++row) {
            term = term * a.getElement(row, permutation[row]);
            for (std::size_t later = row + 1; later < permutation.size(); ++later) {
                inversions += permutation[row] > permutation[later];
            }
        }
        result = inversions % 2 == 0 ? T(result + term) : T(result - term);
    } while (std::next_permutation(permutation.begin(), permutation.end()));
    return result;
}

inline void expectPLU(const Matrix<double>& a) {
    const auto p = a.getPLU_P();
    const auto l = a.getPLU_L();
    const auto u = a.getPLU_U();
    const auto n = a.getNumRows();
    ASSERT_EQ(p.getNumRows(), n);
    ASSERT_EQ(p.getNumCols(), n);
    ASSERT_EQ(l.getNumRows(), n);
    ASSERT_EQ(l.getNumCols(), n);
    ASSERT_EQ(u.getNumRows(), n);
    ASSERT_EQ(u.getNumCols(), n);
    expectNear(p * a, l * u);
    for (std::size_t row = 0; row < n; ++row) {
        double rowSum = 0;
        double colSum = 0;
        for (std::size_t col = 0; col < n; ++col) {
            EXPECT_TRUE(p.getElement(row, col) == 0 || p.getElement(row, col) == 1);
            rowSum += p.getElement(row, col);
            colSum += p.getElement(col, row);
            if (row < col) {
                EXPECT_DOUBLE_EQ(l.getElement(row, col), 0);
            }
            if (row > col) {
                EXPECT_DOUBLE_EQ(u.getElement(row, col), 0);
            }
        }
        EXPECT_DOUBLE_EQ(rowSum, 1);
        EXPECT_DOUBLE_EQ(colSum, 1);
        EXPECT_DOUBLE_EQ(l.getElement(row, row), 1);
    }
}

#endif
