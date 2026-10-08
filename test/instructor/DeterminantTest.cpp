#include "TestSupport.hpp"

class SmallDeterminant : public testing::TestWithParam<int> {};

TEST_P(SmallDeterminant, ExhaustiveTwoByTwoIntegerAndDoubleCases) {
    const int first = GetParam() / 5 - 2;
    const int second = GetParam() % 5 - 2;
    for (int third = -2; third <= 2; ++third) {
        for (int fourth = -2; fourth <= 2; ++fourth) {
            const int expected = first * fourth - second * third;
            const auto exact = matrix<int>({{first, second}, {third, fourth}});
            const auto approximate = matrix<double>({{double(first), double(second)}, {double(third), double(fourth)}});
            EXPECT_EQ(exact.determinant(), expected);
            EXPECT_NEAR(approximate.determinant(), expected, 1e-12);
        }
    }
}

INSTANTIATE_TEST_SUITE_P(AllFirstRows, SmallDeterminant, testing::Range(0, 25));

TEST(Determinant, AllThreeByThreeMatricesWithEntriesMinusOneZeroAndOne) {
    for (int encoding = 0; encoding < 19683; ++encoding) {
        auto digits = encoding;
        std::array<int, 9> v{};
        Matrix<int> a(3, 3);
        for (std::size_t index = 0; index < v.size(); ++index) {
            v[index] = digits % 3 - 1;
            digits /= 3;
            a.setElement(index / 3, index % 3, v[index]);
        }
        const int expected = v[0]*v[4]*v[8] + v[1]*v[5]*v[6] + v[2]*v[3]*v[7]
            - v[2]*v[4]*v[6] - v[1]*v[3]*v[8] - v[0]*v[5]*v[7];
        ASSERT_EQ(a.determinant(), expected) << encoding;
    }
}

TEST(Determinant, SeededMatricesThroughFiveByFiveMatchPermutationExpansion) {
    std::mt19937 random(220625);
    std::uniform_int_distribution<int> entry(-3, 3);
    for (std::size_t n = 1; n <= 5; ++n) {
        for (int repetition = 0; repetition < 30; ++repetition) {
            Matrix<long long> exact(n, n);
            Matrix<double> approximate(n, n);
            for (std::size_t row = 0; row < n; ++row) {
                for (std::size_t col = 0; col < n; ++col) {
                    const int value = entry(random);
                    exact.setElement(row, col, value);
                    approximate.setElement(row, col, value);
                }
            }
            const auto expected = permutationDeterminant(exact);
            EXPECT_EQ(exact.determinant(), expected);
            EXPECT_NEAR(approximate.determinant(), expected, 1e-9);
        }
    }
}

TEST(Determinant, EmptySingleEntryAndRectangularContracts) {
    EXPECT_EQ(Matrix<int>().determinant(), 1);
    EXPECT_DOUBLE_EQ(Matrix<double>().determinant(), 1);
    for (int value : {-7, 0, 1, 9}) {
        EXPECT_EQ(matrix<int>({{value}}).determinant(), value);
        EXPECT_DOUBLE_EQ(matrix<double>({{double(value)}}).determinant(), value);
    }
    EXPECT_THROW(Matrix<int>(2, 3).determinant(), std::invalid_argument);
    EXPECT_THROW(Matrix<double>(3, 2).determinant(), std::invalid_argument);
}

TEST(Determinant, RowPermutationSignAndTriangularProduct) {
    EXPECT_DOUBLE_EQ(matrix<double>({{0, 1}, {1, 0}}).determinant(), -1);
    EXPECT_DOUBLE_EQ(matrix<double>({{0, 1, 0}, {0, 0, 1}, {1, 0, 0}}).determinant(), 1);
    EXPECT_NEAR(matrix<double>({{-2, 3, 4}, {0, 0.5, 6}, {0, 0, 7}}).determinant(), -7, 1e-12);
    EXPECT_EQ(matrix<int>({{2, 3}, {1, 4}}).determinant(), 5);
}

TEST(Determinant, SingularMatricesReturnZeroAndCacheFailureClassification) {
    for (const auto& a : std::vector<Matrix<double>>{
            matrix<double>({{0}}), matrix<double>({{1, 2}, {2, 4}}),
            matrix<double>({{0, 1, 2}, {0, 3, 4}, {0, 5, 6}}), Matrix<double>(4, 4)}) {
        EXPECT_DOUBLE_EQ(a.determinant(), 0);
        EXPECT_DOUBLE_EQ(a.determinant(), 0);
        EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
        EXPECT_EQ(a.getCacheInfo().determinantComputations, 1);
    }
}

TEST(Determinant, NumericToleranceUsesScaleAndCanBeChanged) {
    auto a = matrix<double>({{1, 0}, {0, 1e-13}});
    EXPECT_DOUBLE_EQ(a.determinant(), 0);
    a.setEchelon(1e-14);
    EXPECT_NEAR(a.determinant(), 1e-13, 1e-28);
    a.setEchelon(0);
    EXPECT_NEAR(a.determinant(), 1e-13, 1e-28);
    for (double scale : {1e-100, 1e-20, 1.0, 1e20, 1e100}) {
        const auto scaled = matrix<double>({{2*scale, scale}, {scale, 3*scale}});
        EXPECT_NEAR(scaled.determinant(), 5*scale*scale, 1e-12 * 5*scale*scale);
    }
}

TEST(Determinant, RejectsNonfiniteInputsAndOverflowWithoutCachingBadResults) {
    for (double invalid : {std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
        auto a = matrix<double>({{invalid}});
        EXPECT_THROW(a.determinant(), std::invalid_argument);
        EXPECT_TRUE(a.getCacheInfo().determinantDirty);
    }
    auto large = matrix<double>({{1e200, 0}, {0, 1e200}});
    EXPECT_THROW(large.determinant(), std::overflow_error);
    EXPECT_TRUE(large.getCacheInfo().determinantDirty);
}
