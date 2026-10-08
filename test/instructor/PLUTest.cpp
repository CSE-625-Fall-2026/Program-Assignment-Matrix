#include "TestSupport.hpp"

TEST(PLU, KnownTwoByTwoFactorsUsePartialPivoting) {
    const auto a = matrix<double>({{2, 1}, {4, 3}});
    expectNear(a.getPLU_P(), matrix<double>({{0, 1}, {1, 0}}));
    expectNear(a.getPLU_L(), matrix<double>({{1, 0}, {0.5, 1}}));
    expectNear(a.getPLU_U(), matrix<double>({{4, 3}, {0, -0.5}}));
    expectPLU(a);
}

TEST(PLU, LaterPivotSwapsPreviouslyComputedLowerEntries) {
    const auto a = matrix<double>({{4, 2, 1}, {2, 1, 3}, {1, 5, 2}});
    expectPLU(a);
    expectNear(a.getPLU_P(), matrix<double>({{1, 0, 0}, {0, 0, 1}, {0, 1, 0}}));
    EXPECT_DOUBLE_EQ(a.getPLU_L().getElement(1, 0), 0.25);
    EXPECT_DOUBLE_EQ(a.getPLU_L().getElement(2, 0), 0.5);
}

TEST(PLU, LargestAbsolutePivotIncludesNegativeCandidates) {
    const auto a = matrix<double>({{1, 2, 3}, {-9, 2, 1}, {4, -2, 7}});
    expectPLU(a);
    EXPECT_DOUBLE_EQ(a.getPLU_U().getElement(0, 0), -9);
    EXPECT_DOUBLE_EQ(a.getPLU_P().getElement(0, 1), 1);
}

TEST(PLU, SingleIdentityTriangularAndPermutationMatrices) {
    expectPLU(matrix<double>({{-3}}));
    expectPLU(identity<double>(5));
    expectPLU(matrix<double>({{2, 3, 4}, {0, -5, 6}, {0, 0, 7}}));
    expectPLU(matrix<double>({{0, 0, 1}, {1, 0, 0}, {0, 1, 0}}));
}

TEST(PLU, AllSmallNonsingularTwoByTwoInputsReconstructOriginal) {
    for (int a = -2; a <= 2; ++a) {
        for (int b = -2; b <= 2; ++b) {
            for (int c = -2; c <= 2; ++c) {
                for (int d = -2; d <= 2; ++d) {
                    if (a*d != b*c) {
                        expectPLU(matrix<double>({{double(a), double(b)}, {double(c), double(d)}}));
                    }
                }
            }
        }
    }
}

TEST(PLU, SeededDiagonallyDominantMatricesExerciseParallelElimination) {
    std::mt19937 random(706);
    std::uniform_real_distribution<double> entry(-1, 1);
    for (std::size_t n : {3, 7, 17, 24}) {
        Matrix<double> a(n, n);
        for (std::size_t row = 0; row < n; ++row) {
            for (std::size_t col = 0; col < n; ++col) {
                a.setElement(row, col, entry(random) + (row == col ? double(n) : 0));
            }
        }
        const auto original = a;
        expectPLU(a);
        EXPECT_EQ(a, original);
        EXPECT_TRUE(a.getCacheInfo().determinantDirty);
    }
}

TEST(PLU, FactorCopiesCannotAlterCachedMatrices) {
    const auto a = matrix<double>({{2, 1}, {4, 3}});
    auto p = a.getPLU_P();
    auto l = a.getPLU_L();
    auto u = a.getPLU_U();
    p.setElement(0, 0, 99);
    l.setElement(0, 0, 99);
    u.setElement(0, 0, 99);
    expectPLU(a);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
}

TEST(PLU, SingularAndEmptyOrRectangularInputsHaveDistinctErrors) {
    for (const auto& a : std::vector<Matrix<double>>{
            Matrix<double>(2, 2), matrix<double>({{1, 2}, {2, 4}}), matrix<double>({{0}})}) {
        EXPECT_THROW(a.PLU(), std::domain_error);
        EXPECT_THROW(a.getPLU_P(), std::domain_error);
        EXPECT_THROW(a.getPLU_L(), std::domain_error);
        EXPECT_THROW(a.getPLU_U(), std::domain_error);
        EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
    }
    EXPECT_THROW(Matrix<double>().PLU(), std::invalid_argument);
    EXPECT_THROW(Matrix<double>(2, 3).PLU(), std::invalid_argument);
    EXPECT_THROW(Matrix<double>(3, 2).getPLU_P(), std::invalid_argument);
}

TEST(PLU, ToleranceBoundaryIsInclusiveAndZeroMeansExactZeroOnly) {
    auto a = matrix<double>({{1, 0}, {0, 0.125}});
    a.setEchelon(0.125);
    EXPECT_THROW(a.PLU(), std::domain_error);
    a.setEchelon(std::nextafter(0.125, 0.0));
    EXPECT_NO_THROW(a.PLU());
    auto small = matrix<double>({{1, 0}, {0, 1e-20}});
    small.setEchelon(0);
    EXPECT_NO_THROW(small.PLU());
    small.setElement(1, 1, 0);
    EXPECT_THROW(small.PLU(), std::domain_error);
}

TEST(PLU, NonfiniteEntriesAndEliminationOverflowDoNotPublishPartialFactors) {
    for (double invalid : {std::numeric_limits<double>::quiet_NaN(),
            std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()}) {
        auto a = matrix<double>({{1, invalid}, {0, 1}});
        EXPECT_THROW(a.PLU(), std::invalid_argument);
        EXPECT_TRUE(a.getCacheInfo().pluDirty);
        EXPECT_EQ(a.getCacheInfo().pluComputations, 0);
    }
    const double large = std::numeric_limits<double>::max();
    auto a = matrix<double>({{large, large}, {-large, large}});
    EXPECT_THROW(a.PLU(), std::overflow_error);
    EXPECT_TRUE(a.getCacheInfo().pluDirty);
    a.setElement(1, 1, 0);
    EXPECT_NO_THROW(a.PLU());
}

TEST(PLU, ZeroToleranceDoesNotDiscardANonzeroPivotWhenItsRelativeRatioUnderflows) {
    auto a = matrix<double>({{1e200, 0}, {0, 1e-200}});
    a.setEchelon(0);
    EXPECT_NO_THROW(a.PLU());
    EXPECT_NEAR(a.determinant(), 1, 1e-15);
    expectNear(a.solve(matrix<double>({{1e200}, {1e-200}})), matrix<double>({{1}, {1}}));
}
