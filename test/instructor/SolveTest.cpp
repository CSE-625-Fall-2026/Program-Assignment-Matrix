#include "TestSupport.hpp"

TEST(Solve, PivotedSystemHasKnownFractionalAndNegativeSolution) {
    const auto a = matrix<double>({{0, 2, 1}, {1, -2, -3}, {2, 3, 1}});
    const auto b = matrix<double>({{3}, {0}, {7}});
    const auto originalA = a;
    const auto originalB = b;
    const auto x = a.solve(b);
    expectNear(x, matrix<double>({{1}, {2}, {-1}}));
    expectNear(a * x, b);
    EXPECT_EQ(a, originalA);
    EXPECT_EQ(b, originalB);
    expectNear(matrix<double>({{2}}).solve(matrix<double>({{1}})), matrix<double>({{0.5}}));
}

TEST(Solve, MultipleRightHandSidesAndSelfRightHandSide) {
    const auto a = matrix<double>({{0, 2, 1}, {1, -2, -3}, {2, 3, 1}});
    const auto expected = matrix<double>({{1, -1, 0, 0.25}, {2, 0, 0, -0.5}, {-1, 2, 0, 0.75}});
    const auto b = a * expected;
    expectNear(a.solve(b), expected);
    expectNear(a.solve(a), identity<double>(3));
    expectNear(a.solve(Matrix<double>(3, 4)), Matrix<double>(3, 4));
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 0);
}

TEST(Solve, SeededSystemsMatchKnownSolutionsAcrossShapesAndScales) {
    std::mt19937 random(170625);
    std::uniform_real_distribution<double> entry(-2, 2);
    for (std::size_t n : {1, 2, 3, 8, 20}) {
        for (std::size_t columns : {1, 2, 5}) {
            for (double scale : {1e-60, 1.0, 1e60}) {
                Matrix<double> a(n, n);
                Matrix<double> expected(n, columns);
                for (std::size_t row = 0; row < n; ++row) {
                    for (std::size_t col = 0; col < n; ++col) {
                        a.setElement(row, col, scale * (entry(random) + (row == col ? 3*double(n) : 0)));
                    }
                    for (std::size_t col = 0; col < columns; ++col) {
                        expected.setElement(row, col, entry(random));
                    }
                }
                const auto b = a * expected;
                expectNear(a.solve(b), expected);
            }
        }
    }
}

TEST(Solve, RejectsEmptyRectangularAndMismatchedRightHandSides) {
    const auto a = identity<double>(2);
    EXPECT_THROW(a.solve(Matrix<double>()), std::invalid_argument);
    EXPECT_THROW(a.solve(Matrix<double>(1, 2)), std::invalid_argument);
    EXPECT_THROW(a.solve(Matrix<double>(3, 1)), std::invalid_argument);
    EXPECT_THROW(Matrix<double>().solve(Matrix<double>()), std::invalid_argument);
    EXPECT_THROW(Matrix<double>(2, 3).solve(Matrix<double>(2, 1)), std::invalid_argument);
    EXPECT_THROW(Matrix<double>(3, 2).solve(Matrix<double>(3, 1)), std::invalid_argument);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 0);
}

TEST(Solve, NonzeroButSingularIsStillRejected) {
    const auto singular = matrix<double>({{1, 2}, {2, 4}});
    EXPECT_THROW(singular.solve(matrix<double>({{3}, {6}})), std::domain_error);
    EXPECT_THROW(singular.solve(matrix<double>({{3}, {7}})), std::domain_error);
    EXPECT_THROW(Matrix<double>(2, 2).solve(Matrix<double>(2, 1)), std::domain_error);
    EXPECT_EQ(singular.getCacheInfo().pluComputations, 1);
}

TEST(Solve, ToleranceControlsNumericalSingularity) {
    auto a = matrix<double>({{1, 0}, {0, 1e-13}});
    const auto b = matrix<double>({{2}, {3e-13}});
    EXPECT_THROW(a.solve(b), std::domain_error);
    a.setEchelon(1e-14);
    expectNear(a.solve(b), matrix<double>({{2}, {3}}));
}

TEST(Solve, RejectsNonfiniteInputAndResultOverflow) {
    for (double invalid : {std::numeric_limits<double>::quiet_NaN(),
            std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()}) {
        EXPECT_THROW(identity<double>(2).solve(matrix<double>({{1}, {invalid}})), std::invalid_argument);
        EXPECT_THROW(matrix<double>({{invalid}}).solve(matrix<double>({{1}})), std::invalid_argument);
    }
    const auto a = matrix<double>({{1e-200}});
    EXPECT_THROW(a.solve(matrix<double>({{1e200}})), std::overflow_error);
    expectNear(a.solve(matrix<double>({{1e-200}})), matrix<double>({{1}}));
}

TEST(Solve, EveryCallUsesTheCurrentRightHandSide) {
    const auto a = matrix<double>({{2, 0}, {0, 4}});
    auto b = matrix<double>({{4}, {8}});
    expectNear(a.solve(b), matrix<double>({{2}, {2}}));
    b.setElement(0, 0, 6);
    expectNear(a.solve(b), matrix<double>({{3}, {2}}));
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
}
