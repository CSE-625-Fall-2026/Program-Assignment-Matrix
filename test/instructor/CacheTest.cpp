#include "TestSupport.hpp"

TEST(Cache, ConstructionCopyAccessArithmeticAndEqualityAreLazy) {
    auto a = matrix<double>({{2, 1}, {4, 3}});
    const auto original = a;
    a.getElement(0, 0);
    EXPECT_EQ(a, original);
    const auto sum = a + original;
    const auto difference = a - original;
    const auto product = a * original;
    for (const auto* value : std::array<const Matrix<double>*, 5>{&a, &original, &sum, &difference, &product}) {
        EXPECT_EQ(value->getCacheInfo().pluComputations, 0);
        EXPECT_EQ(value->getCacheInfo().determinantComputations, 0);
        EXPECT_TRUE(value->getCacheInfo().pluDirty);
        EXPECT_TRUE(value->getCacheInfo().determinantDirty);
    }
}

TEST(Cache, PLUAndDeterminantShareFactorsWithoutEagerDeterminant) {
    const auto a = matrix<double>({{2, 1}, {4, 3}});
    a.PLU();
    EXPECT_TRUE(a.getCacheInfo().determinantDirty);
    EXPECT_FALSE(a.getCacheInfo().pluDirty);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
    a.getPLU_P();
    a.getPLU_L();
    a.getPLU_U();
    a.solve(identity<double>(2));
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 0);
    EXPECT_NEAR(a.determinant(), 2, 1e-12);
    EXPECT_NEAR(a.determinant(), 2, 1e-12);
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 1);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
}

TEST(Cache, DeterminantFirstThenSolveAndFactorsReuseOneDecomposition) {
    const auto a = matrix<double>({{0, 2}, {3, 1}});
    EXPECT_NEAR(a.determinant(), -6, 1e-12);
    a.solve(identity<double>(2));
    expectPLU(a);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 1);
}

TEST(Cache, EveryEntryMutationInvalidatesBothCalculations) {
    for (std::size_t row = 0; row < 2; ++row) {
        for (std::size_t col = 0; col < 2; ++col) {
            auto a = matrix<double>({{2, 1}, {4, 3}});
            a.determinant();
            a.setElement(row, col, a.getElement(row, col) + 0.25);
            EXPECT_TRUE(a.getCacheInfo().determinantDirty);
            EXPECT_TRUE(a.getCacheInfo().pluDirty);
            EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
            const double expected = a.getElement(0, 0)*a.getElement(1, 1)
                - a.getElement(0, 1)*a.getElement(1, 0);
            EXPECT_NEAR(a.determinant(), expected, 1e-12);
            EXPECT_EQ(a.getCacheInfo().pluComputations, 2);
            EXPECT_EQ(a.getCacheInfo().determinantComputations, 2);
        }
    }
}

class MutatingOperation : public testing::TestWithParam<int> {};

TEST_P(MutatingOperation, AssignmentAndCompoundsInvalidateCachesLazily) {
    auto a = matrix<double>({{2, 1}, {4, 3}});
    const auto b = identity<double>(2);
    a.determinant();
    if (GetParam() == 0) a = b;
    if (GetParam() == 1) a += b;
    if (GetParam() == 2) a -= b;
    if (GetParam() == 3) a *= b;
    EXPECT_TRUE(a.getCacheInfo().pluDirty);
    EXPECT_TRUE(a.getCacheInfo().determinantDirty);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
    a.determinant();
    EXPECT_EQ(a.getCacheInfo().pluComputations, 2);
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 2);
}

INSTANTIATE_TEST_SUITE_P(AllMutations, MutatingOperation, testing::Range(0, 4));

TEST(Cache, EchelonChangeInvalidatesButRejectedChangeAndSameValueDoNot) {
    auto a = identity<double>(2);
    a.determinant();
    a.setEchelon(a.getEchelon());
    EXPECT_FALSE(a.getCacheInfo().pluDirty);
    for (double invalid : {-0.1, 1.0, 2.0, std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
        EXPECT_THROW(a.setEchelon(invalid), std::invalid_argument);
        EXPECT_FALSE(a.getCacheInfo().pluDirty);
        EXPECT_DOUBLE_EQ(a.getEchelon(), 1e-12);
    }
    a.setEchelon(0);
    EXPECT_TRUE(a.getCacheInfo().pluDirty);
    EXPECT_TRUE(a.getCacheInfo().determinantDirty);
    a.determinant();
    EXPECT_EQ(a.getCacheInfo().pluComputations, 2);
    EXPECT_DOUBLE_EQ(Matrix<int>().getEchelon(), 0);
}

TEST(Cache, FailedOperationsAndSelfAssignmentPreserveValidCaches) {
    auto a = identity<double>(2);
    a.determinant();
    EXPECT_THROW(a.setElement(2, 0, 1), std::out_of_range);
    EXPECT_THROW(a = Matrix<double>(1, 4), std::invalid_argument);
    EXPECT_THROW(a += Matrix<double>(1, 4), std::invalid_argument);
    EXPECT_THROW(a *= Matrix<double>(2, 1), std::invalid_argument);
    const auto& alias = a;
    a = alias;
    EXPECT_FALSE(a.getCacheInfo().pluDirty);
    EXPECT_FALSE(a.getCacheInfo().determinantDirty);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
}

TEST(Cache, CopiesStartUncomputedAndMutationsDoNotInvalidateOriginal) {
    auto a = matrix<double>({{2, 1}, {4, 3}});
    a.determinant();
    auto copy = a;
    EXPECT_TRUE(copy.getCacheInfo().pluDirty);
    EXPECT_EQ(copy.getCacheInfo().pluComputations, 0);
    copy.setElement(0, 0, 8);
    EXPECT_DOUBLE_EQ(copy.determinant(), 20);
    EXPECT_NEAR(a.determinant(), 2, 1e-12);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
}

TEST(Cache, SingularResultsRecoverAfterMutationAndCacheIndependently) {
    auto a = matrix<double>({{1, 2}, {2, 4}});
    EXPECT_DOUBLE_EQ(a.determinant(), 0);
    EXPECT_THROW(a.PLU(), std::domain_error);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
    a.setElement(1, 1, 5);
    EXPECT_NEAR(a.determinant(), 1, 1e-12);
    expectPLU(a);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 2);
}

TEST(Cache, IntegerDeterminantsNeverCalculatePLU) {
    auto a = matrix<int>({{2, 3}, {1, 4}});
    EXPECT_EQ(a.determinant(), 5);
    EXPECT_EQ(a.determinant(), 5);
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 1);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 0);
    a.setElement(0, 0, 3);
    EXPECT_EQ(a.determinant(), 9);
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 2);
}
