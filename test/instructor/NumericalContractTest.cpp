#include "TestSupport.hpp"
#include <complex>

#ifdef MATRIXCLASS_EXTERNAL_NUMBER_TESTS
#include <ArbitraryInteger.hpp>
#include <RationalNumber.hpp>
using NumericalTypes = testing::Types<RationalNumber<ArbitraryInteger>>;
#else
using NumericalTypes = testing::Types<double, std::complex<double>>;
#endif

template<typename T>
class NumericalContract : public testing::Test {};
TYPED_TEST_SUITE(NumericalContract, NumericalTypes,);

template<typename T>
void expectNumerical(const Matrix<T>& actual, const Matrix<T>& expected) {
    EXPECT_EQ(actual, expected);
}

void expectNumerical(const Matrix<double>& actual, const Matrix<double>& expected) {
    expectNear(actual, expected);
}

void expectNumerical(const Matrix<std::complex<double>>& actual,
                     const Matrix<std::complex<double>>& expected) {
    ASSERT_EQ(actual.getNumRows(), expected.getNumRows());
    ASSERT_EQ(actual.getNumCols(), expected.getNumCols());
    for (std::size_t row = 0; row < actual.getNumRows(); ++row) {
        for (std::size_t col = 0; col < actual.getNumCols(); ++col) {
            const auto reference = expected.getElement(row, col);
            EXPECT_LE(std::abs(actual.getElement(row, col) - reference),
                1e-10 * std::max(1.0, std::abs(reference)));
        }
    }
}

TYPED_TEST(NumericalContract, PivotingMaintainsPermutationAndTriangularFactors) {
    const auto a = matrix<TypeParam>({{0, 2, 1}, {1, -2, -3}, {2, 3, 1}});
    a.PLU();
    const auto p = a.getPLU_P();
    const auto l = a.getPLU_L();
    const auto u = a.getPLU_U();
    expectNumerical(p * a, l * u);
    EXPECT_EQ(p.getElement(0, 2), TypeParam{1});
    for (std::size_t row = 0; row < 3; ++row) {
        TypeParam rowSum{};
        TypeParam colSum{};
        for (std::size_t col = 0; col < 3; ++col) {
            EXPECT_TRUE(p.getElement(row, col) == TypeParam{} || p.getElement(row, col) == TypeParam{1});
            rowSum = rowSum + p.getElement(row, col);
            colSum = colSum + p.getElement(col, row);
            if (row < col) EXPECT_EQ(l.getElement(row, col), TypeParam{});
            if (row > col) EXPECT_EQ(u.getElement(row, col), TypeParam{});
        }
        EXPECT_EQ(l.getElement(row, row), TypeParam{1});
        EXPECT_EQ(rowSum, TypeParam{1});
        EXPECT_EQ(colSum, TypeParam{1});
    }
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 0);
}

TYPED_TEST(NumericalContract, EachFactorGetterRequestsPLULazilyAndReturnsIndependentStorage) {
    for (int first = 0; first < 3; ++first) {
        const auto a = matrix<TypeParam>({{2, 1}, {1, 3}});
        EXPECT_TRUE(a.getCacheInfo().pluDirty);
        auto factor = first == 0 ? a.getPLU_P() : first == 1 ? a.getPLU_L() : a.getPLU_U();
        factor.setElement(0, 0, TypeParam{99});
        expectNumerical(a.getPLU_P() * a, a.getPLU_L() * a.getPLU_U());
        EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
        EXPECT_TRUE(a.getCacheInfo().determinantDirty);
        EXPECT_EQ(a.getCacheInfo().determinantComputations, 0);
    }
}

TYPED_TEST(NumericalContract, SolveIncludesFractionalAnswersAndMultipleRightHandSides) {
    const auto a = matrix<TypeParam>({{0, 2, 1}, {1, -2, -3}, {2, 3, 1}});
    const auto expected = matrix<TypeParam>({{TypeParam{1} / TypeParam{2}, 2},
        {TypeParam{-2} / TypeParam{3}, -1}, {TypeParam{3} / TypeParam{7}, 4}});
    const auto b = a * expected;
    const auto original = a;
    const auto originalB = b;
    expectNumerical(a.solve(b), expected);
    expectNumerical(a * a.solve(b), b);
    EXPECT_EQ(a, original);
    EXPECT_EQ(b, originalB);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 0);
    expectNumerical(a.solve(a), identity<TypeParam>(3));
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
}

TYPED_TEST(NumericalContract, OneElementMatricesSolveWithFractionalDivision) {
    const auto a = matrix<TypeParam>({{3}});
    expectNumerical(a.solve(matrix<TypeParam>({{1, -2}})),
        matrix<TypeParam>({{TypeParam{1} / TypeParam{3}, TypeParam{-2} / TypeParam{3}}}));
    expectNumerical(a.getPLU_P() * a, a.getPLU_L() * a.getPLU_U());
}

TYPED_TEST(NumericalContract, SingularClassificationsAreCachedAndInvalidated) {
    for (const auto& a : {Matrix<TypeParam>(2, 2), matrix<TypeParam>({{1, 2}, {2, 4}}),
            matrix<TypeParam>({{0, 1}, {0, 2}})}) {
        EXPECT_THROW(a.PLU(), std::domain_error);
        EXPECT_THROW(a.getPLU_P(), std::domain_error);
        EXPECT_THROW(a.getPLU_L(), std::domain_error);
        EXPECT_THROW(a.getPLU_U(), std::domain_error);
        EXPECT_THROW(a.solve(identity<TypeParam>(2)), std::domain_error);
        EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
        EXPECT_FALSE(a.getCacheInfo().pluDirty);
        auto repaired = a;
        repaired = identity<TypeParam>(2);
        EXPECT_NO_THROW(repaired.PLU());
    }
    auto a = matrix<TypeParam>({{1, 2}, {2, 4}});
    EXPECT_THROW(a.PLU(), std::domain_error);
    a.setElement(1, 1, TypeParam{5});
    EXPECT_NO_THROW(a.PLU());
    EXPECT_EQ(a.getCacheInfo().pluComputations, 2);
}

TYPED_TEST(NumericalContract, EveryMutationInvalidatesPreviouslyComputedFactors) {
    for (int operation = 0; operation < 6; ++operation) {
        auto a = matrix<TypeParam>({{3, 1}, {1, 4}});
        a.PLU();
        if (operation == 0) a.setElement(0, 0, TypeParam{4});
        if (operation == 1) a = identity<TypeParam>(2);
        if (operation == 2) a += identity<TypeParam>(2);
        if (operation == 3) a -= identity<TypeParam>(2);
        if (operation == 4) a *= identity<TypeParam>(2);
        if (operation == 5) a.setEchelon(0.001);
        EXPECT_TRUE(a.getCacheInfo().pluDirty);
        EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
        expectNumerical(a.solve(a), identity<TypeParam>(2));
        EXPECT_EQ(a.getCacheInfo().pluComputations, 2);
        EXPECT_EQ(a.getCacheInfo().determinantComputations, 0);
    }
}

TYPED_TEST(NumericalContract, CopiesDiscardFactorsAndNoOpChangesKeepThem) {
    auto a = matrix<TypeParam>({{2, 1}, {0, 3}});
    a.PLU();
    auto copy = a;
    EXPECT_TRUE(copy.getCacheInfo().pluDirty);
    EXPECT_EQ(copy.getCacheInfo().pluComputations, 0);
    copy.PLU();
    copy.setElement(0, 0, TypeParam{9});
    EXPECT_FALSE(a.getCacheInfo().pluDirty);
    a.setEchelon(a.getEchelon());
    const auto& alias = a;
    a = alias;
    EXPECT_FALSE(a.getCacheInfo().pluDirty);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
}

TYPED_TEST(NumericalContract, RelativeToleranceHasAnInclusiveBoundaryAndZeroDisablesIt) {
    auto a = matrix<TypeParam>({{1, 0}, {0, TypeParam{1} / TypeParam{8}}});
    a.setEchelon(0.125);
    EXPECT_THROW(a.PLU(), std::domain_error);
    EXPECT_THROW(a.solve(a), std::domain_error);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
    a.setEchelon(0.0625);
    expectNumerical(a.solve(a), identity<TypeParam>(2));
    a.setEchelon(0);
    expectNumerical(a.solve(a), identity<TypeParam>(2));
    EXPECT_EQ(a.getCacheInfo().pluComputations, 3);
}

TYPED_TEST(NumericalContract, InvalidShapesAndPartialDimensionsFailBeforeCalculating) {
    const Matrix<TypeParam> empty;
    const Matrix<TypeParam> rectangular(2, 3);
    const auto a = identity<TypeParam>(2);
    EXPECT_THROW(empty.PLU(), std::invalid_argument);
    EXPECT_THROW(empty.getPLU_P(), std::invalid_argument);
    EXPECT_THROW(empty.getPLU_L(), std::invalid_argument);
    EXPECT_THROW(empty.getPLU_U(), std::invalid_argument);
    EXPECT_THROW(empty.solve(empty), std::invalid_argument);
    EXPECT_THROW(rectangular.PLU(), std::invalid_argument);
    EXPECT_THROW(rectangular.getPLU_P(), std::invalid_argument);
    EXPECT_THROW(rectangular.getPLU_L(), std::invalid_argument);
    EXPECT_THROW(rectangular.getPLU_U(), std::invalid_argument);
    EXPECT_THROW(rectangular.solve(a), std::invalid_argument);
    EXPECT_THROW(a.solve(empty), std::invalid_argument);
    EXPECT_THROW(a.solve(Matrix<TypeParam>(3, 1)), std::invalid_argument);
    Matrix<TypeParam> partial;
    partial.setNumCols(2);
    EXPECT_THROW(partial.PLU(), std::logic_error);
    EXPECT_THROW(partial.getPLU_P(), std::logic_error);
    EXPECT_THROW(partial.getPLU_L(), std::logic_error);
    EXPECT_THROW(partial.getPLU_U(), std::logic_error);
    EXPECT_THROW(partial.solve(a), std::logic_error);
    EXPECT_THROW(a.solve(partial), std::logic_error);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 0);
}

TYPED_TEST(NumericalContract, ParallelEliminationAndRightHandSidesMatchKnownSolutions) {
    Matrix<TypeParam> a(17, 17);
    Matrix<TypeParam> expected(17, 4);
    for (std::size_t row = 0; row < 17; ++row) {
        a.setElement(row, row, TypeParam{2});
        a.setElement(row, (row + 1) % 17, TypeParam{1});
        for (std::size_t col = 0; col < 4; ++col) {
            expected.setElement(row, col, TypeParam(int((row + col) % 5) - 2) / TypeParam{3});
        }
    }
    const auto b = a * expected;
    expectNumerical(a.solve(b), expected);
    expectNumerical(a.getPLU_P() * a, a.getPLU_L() * a.getPLU_U());
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
}
