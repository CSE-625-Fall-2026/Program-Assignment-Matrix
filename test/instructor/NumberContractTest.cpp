#include "TestSupport.hpp"
#include <complex>
#include <type_traits>

#ifdef MATRIXCLASS_EXTERNAL_NUMBER_TESTS
#include <ArbitraryInteger.hpp>
#include <RationalNumber.hpp>
using ContractTypes = testing::Types<ArbitraryInteger, RationalNumber<ArbitraryInteger>>;
using IntegerTypes = testing::Types<ArbitraryInteger>;
#else
using ContractTypes = testing::Types<double, int, std::complex<double>>;
using IntegerTypes = testing::Types<int>;
#endif

template<typename T>
class NumberContract : public testing::Test {};
TYPED_TEST_SUITE(NumberContract, ContractTypes,);

TYPED_TEST(NumberContract, DivisionMethodsUseElementArithmetic) {
    const auto a = matrix<TypeParam>({{2, 1}, {0, 3}});
    const auto b = matrix<TypeParam>({{3}, {3}});
    a.PLU();
    EXPECT_EQ(a.getPLU_P() * a, a.getPLU_L() * a.getPLU_U());
    EXPECT_EQ(a.solve(b), matrix<TypeParam>({{1}, {1}}));
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
    EXPECT_EQ(a.determinant(), TypeParam{6});
}

TYPED_TEST(NumberContract, DivisionKeepsTheElementTypeForPositiveAndNegativeAnswers) {
    const auto a = matrix<TypeParam>({{2}});
    const auto b = matrix<TypeParam>({{-3, -1, 1, 3}});
    EXPECT_EQ(a.solve(b), matrix<TypeParam>({{TypeParam{-3} / TypeParam{2},
        TypeParam{-1} / TypeParam{2}, TypeParam{1} / TypeParam{2}, TypeParam{3} / TypeParam{2}}}));
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
}

TYPED_TEST(NumberContract, PivotedSystemWithIntegralIntermediateResultsWorksForEveryType) {
    const auto a = matrix<TypeParam>({{0, 2}, {2, 1}});
    EXPECT_EQ(a.getPLU_P(), matrix<TypeParam>({{0, 1}, {1, 0}}));
    EXPECT_EQ(a.getPLU_L(), identity<TypeParam>(2));
    EXPECT_EQ(a.getPLU_U(), matrix<TypeParam>({{2, 1}, {0, 2}}));
    EXPECT_EQ(a.solve(matrix<TypeParam>({{4}, {4}})), matrix<TypeParam>({{1}, {2}}));
}

TYPED_TEST(NumberContract, DivisionStillRejectsInvalidShapesAndZeroPivots) {
    const auto singular = matrix<TypeParam>({{1, 0}, {0, 0}});
    EXPECT_THROW(singular.PLU(), std::domain_error);
    EXPECT_THROW(singular.getPLU_P(), std::domain_error);
    EXPECT_THROW(singular.getPLU_L(), std::domain_error);
    EXPECT_THROW(singular.getPLU_U(), std::domain_error);
    EXPECT_THROW(singular.solve(identity<TypeParam>(2)), std::domain_error);
    EXPECT_EQ(singular.getCacheInfo().pluComputations, 1);
    EXPECT_THROW(Matrix<TypeParam>().PLU(), std::invalid_argument);
    EXPECT_THROW(Matrix<TypeParam>(2, 3).PLU(), std::invalid_argument);
    EXPECT_THROW(identity<TypeParam>(2).solve(Matrix<TypeParam>(3, 1)), std::invalid_argument);
}

template<typename T>
class IntegerContract : public testing::Test {};
TYPED_TEST_SUITE(IntegerContract, IntegerTypes,);

TYPED_TEST(IntegerContract, TruncatedEliminationCanProduceAnIncorrectSolutionWithoutRejection) {
    const auto a = matrix<TypeParam>({{2, 0}, {1, 1}});
    const auto b = matrix<TypeParam>({{2}, {2}});
    EXPECT_EQ(a.getPLU_L(), identity<TypeParam>(2));
    EXPECT_EQ(a.getPLU_U(), matrix<TypeParam>({{2, 0}, {0, 1}}));
    EXPECT_NE(a.getPLU_P() * a, a.getPLU_L() * a.getPLU_U());
    const auto x = a.solve(b);
    EXPECT_EQ(x, matrix<TypeParam>({{1}, {2}}));
    EXPECT_NE(a * x, b);
}

TYPED_TEST(IntegerContract, FractionalToleranceNaturallyTruncatesToZero) {
    auto a = matrix<TypeParam>({{1, 0}, {0, 2048}});
    for (double tolerance : {0.0, 0.125, 0.001, 1e-300}) {
        a.setEchelon(tolerance);
        EXPECT_EQ(a.getEchelon(), tolerance);
        EXPECT_EQ(a.solve(a), identity<TypeParam>(2));
    }
}

TYPED_TEST(IntegerContract, ParallelDivisionReusesAndInvalidatesFactors) {
    Matrix<TypeParam> a(20, 20);
    Matrix<TypeParam> expected(20, 4);
    for (std::size_t row = 0; row < 20; ++row) {
        a.setElement(row, row, TypeParam{2});
        for (std::size_t col = row + 1; col < 20; ++col) {
            a.setElement(row, col, TypeParam{1});
        }
        for (std::size_t col = 0; col < 4; ++col) {
            expected.setElement(row, col, TypeParam(int(col + 1)));
        }
    }
    EXPECT_EQ(a.solve(a * expected), expected);
    EXPECT_EQ(a.solve(a * expected), expected);
    EXPECT_EQ(a.getPLU_P() * a, a.getPLU_L() * a.getPLU_U());
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 0);
    a.setElement(0, 0, TypeParam{4});
    EXPECT_TRUE(a.getCacheInfo().pluDirty);
    EXPECT_EQ(a.solve(a * expected), expected);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 2);
}

TYPED_TEST(NumberContract, AllConstructionFormsOwnIndependentValues) {
    Matrix<TypeParam> empty;
    EXPECT_EQ(empty.getNumRows(), 0);
    EXPECT_EQ(empty.getNumCols(), 0);
    empty.setNumCols(3);
    EXPECT_EQ(empty.getNumRows(), 0);
    EXPECT_THROW(empty.getElement(0, 0), std::logic_error);
    empty.setNumRows(2);
    EXPECT_EQ(empty, Matrix<TypeParam>(2, 3));
    std::vector<std::vector<TypeParam>> values{{TypeParam{1}, TypeParam{2}, TypeParam{3}},
                                              {TypeParam{4}, TypeParam{5}, TypeParam{6}}};
    const Matrix<TypeParam> fromVector(values);
    values[1][2] = TypeParam{99};
    Matrix<TypeParam> copy(fromVector);
    copy.setElement(0, 0, TypeParam{8});
    EXPECT_EQ(fromVector.getElement(0, 0), TypeParam{1});
    EXPECT_EQ(fromVector.getElement(1, 2), TypeParam{6});
    empty = fromVector;
    EXPECT_EQ(empty, fromVector);
    EXPECT_NE(copy, fromVector);
    EXPECT_THROW(empty.setNumRows(2), std::logic_error);
    EXPECT_THROW(empty.setNumCols(3), std::logic_error);
}

TYPED_TEST(NumberContract, EveryEntryCanBeReadAndWrittenWithoutAliasing) {
    for (std::size_t rows = 1; rows <= 4; ++rows) {
        for (std::size_t cols = 1; cols <= 4; ++cols) {
            Matrix<TypeParam> a(rows, cols);
            for (std::size_t row = 0; row < rows; ++row) {
                for (std::size_t col = 0; col < cols; ++col) {
                    EXPECT_EQ(a.getElement(row, col), TypeParam{});
                    a.setElement(row, col, TypeParam(int(row * cols + col + 1)));
                }
            }
            for (std::size_t row = 0; row < rows; ++row) {
                for (std::size_t col = 0; col < cols; ++col) {
                    EXPECT_EQ(a.getElement(row, col), TypeParam(int(row * cols + col + 1)));
                }
            }
        }
    }
}

TYPED_TEST(NumberContract, ArithmeticHasIndependentReferenceEntries) {
    const auto a = matrix<TypeParam>({{1, -2}, {3, 4}});
    const auto b = matrix<TypeParam>({{5, 6}, {-7, 8}});
    EXPECT_EQ(a + b, matrix<TypeParam>({{6, 4}, {-4, 12}}));
    EXPECT_EQ(a - b, matrix<TypeParam>({{-4, -8}, {10, -4}}));
    EXPECT_EQ(a * b, matrix<TypeParam>({{19, -10}, {-13, 50}}));
    EXPECT_EQ(b * a, matrix<TypeParam>({{23, 14}, {17, 46}}));
    EXPECT_EQ(a.getElement(0, 0), TypeParam{1});
    EXPECT_EQ(b.getElement(0, 0), TypeParam{5});
}

TYPED_TEST(NumberContract, AssignmentAndAllCompoundsPreserveShapeAndReturnTheReceiver) {
    const auto a = matrix<TypeParam>({{1, 2}, {3, 4}});
    const auto b = matrix<TypeParam>({{5, 6}, {7, 8}});
    Matrix<TypeParam> result(2, 2);
    EXPECT_EQ(&(result = a), &result);
    EXPECT_EQ(&(result += b), &result);
    EXPECT_EQ(result, a + b);
    EXPECT_EQ(&(result -= b), &result);
    EXPECT_EQ(result, a);
    EXPECT_EQ(&(result *= b), &result);
    EXPECT_EQ(result, matrix<TypeParam>({{19, 22}, {43, 50}}));
    EXPECT_EQ(result.getNumRows(), 2);
    EXPECT_EQ(result.getNumCols(), 2);
    EXPECT_THROW(result.setNumRows(2), std::logic_error);
    EXPECT_THROW(result.setNumCols(2), std::logic_error);
}

TYPED_TEST(NumberContract, SelfOperationsAreSafe) {
    auto a = matrix<TypeParam>({{1, 2}, {3, 4}});
    const auto original = a;
    const auto& alias = a;
    a = alias;
    EXPECT_EQ(a, original);
    a += a;
    EXPECT_EQ(a, matrix<TypeParam>({{2, 4}, {6, 8}}));
    a -= a;
    EXPECT_EQ(a, Matrix<TypeParam>(2, 2));
    a = original;
    a *= a;
    EXPECT_EQ(a, matrix<TypeParam>({{7, 10}, {15, 22}}));
}

TYPED_TEST(NumberContract, RectangularProductsAndDotProductsAreCorrect) {
    const auto a = matrix<TypeParam>({{1, 2, 3}, {4, 5, 6}});
    const auto b = matrix<TypeParam>({{7, 8}, {9, 10}, {11, 12}});
    EXPECT_EQ(a * b, matrix<TypeParam>({{58, 64}, {139, 154}}));
    EXPECT_EQ(matrix<TypeParam>({{1, 2, 3}}) * matrix<TypeParam>({{4}, {5}, {6}}), matrix<TypeParam>({{32}}));
    auto unchanged = a;
    unchanged *= identity<TypeParam>(3);
    EXPECT_EQ(unchanged, a);
    EXPECT_THROW(unchanged *= b, std::invalid_argument);
    EXPECT_EQ(unchanged, a);
}

TYPED_TEST(NumberContract, EqualityUsesEveryEntryAndTheShape) {
    const auto a = matrix<TypeParam>({{1, 2}, {3, 4}});
    EXPECT_TRUE(a == a);
    EXPECT_FALSE(a != a);
    EXPECT_NE(a, matrix<TypeParam>({{1, 2, 3, 4}}));
    for (std::size_t row = 0; row < 2; ++row) {
        for (std::size_t col = 0; col < 2; ++col) {
            auto changed = a;
            changed.setElement(row, col, TypeParam{99});
            EXPECT_NE(changed, a);
            EXPECT_NE(a, changed);
        }
    }
}

TYPED_TEST(NumberContract, CofactorCasesIncludeEmptySingularAndRowPermutations) {
    EXPECT_EQ(Matrix<TypeParam>().determinant(), TypeParam{1});
    EXPECT_EQ(matrix<TypeParam>({{-7}}).determinant(), TypeParam{-7});
    EXPECT_EQ(matrix<TypeParam>({{2, 3}, {1, 4}}).determinant(), TypeParam{5});
    EXPECT_EQ(matrix<TypeParam>({{0, 1}, {1, 0}}).determinant(), TypeParam{-1});
    EXPECT_EQ(matrix<TypeParam>({{1, 2}, {2, 4}}).determinant(), TypeParam{});
    EXPECT_EQ(matrix<TypeParam>({{2, 1, 0}, {0, 3, 1}, {0, 0, 4}}).determinant(), TypeParam{24});
}

TYPED_TEST(NumberContract, CachesRemainLazyAndAreInvalidatedByEveryMutation) {
    const auto original = matrix<TypeParam>({{2, 1}, {0, 3}});
    EXPECT_EQ(original.getCacheInfo().determinantComputations, 0);
    EXPECT_EQ(original.getCacheInfo().pluComputations, 0);
    for (int operation = 0; operation < 6; ++operation) {
        auto a = original;
        EXPECT_EQ(a.determinant(), TypeParam{6});
        EXPECT_EQ(a.determinant(), TypeParam{6});
        EXPECT_EQ(a.getCacheInfo().determinantComputations, 1);
        const auto identityMatrix = identity<TypeParam>(2);
        if (operation == 0) a.setElement(0, 0, TypeParam{4});
        if (operation == 1) a = identityMatrix;
        if (operation == 2) a += identityMatrix;
        if (operation == 3) a -= identityMatrix;
        if (operation == 4) a *= identityMatrix;
        if (operation == 5) a.setEchelon(0.001);
        EXPECT_TRUE(a.getCacheInfo().determinantDirty);
        EXPECT_TRUE(a.getCacheInfo().pluDirty);
        EXPECT_EQ(a.getCacheInfo().determinantComputations, 1);
        EXPECT_EQ(a.determinant(), permutationDeterminant(a));
        EXPECT_EQ(a.getCacheInfo().determinantComputations, 2);
        if (!std::is_same<TypeParam, double>::value) {
            EXPECT_EQ(a.getCacheInfo().pluComputations, 0);
        }
    }
}

TYPED_TEST(NumberContract, CopiesAndGetterResultsCannotMutateOriginalOrItsCache) {
    const auto a = matrix<TypeParam>({{2, 1}, {0, 3}});
    a.determinant();
    auto copy = a;
    EXPECT_EQ(copy.getCacheInfo().determinantComputations, 0);
    EXPECT_TRUE(copy.getCacheInfo().determinantDirty);
    auto element = a.getElement(0, 0);
    element = TypeParam{99};
    copy.setElement(0, 0, TypeParam{4});
    EXPECT_EQ(copy.determinant(), TypeParam{12});
    EXPECT_EQ(a.determinant(), TypeParam{6});
    EXPECT_EQ(a.getElement(0, 0), TypeParam{2});
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 1);
}

TYPED_TEST(NumberContract, AllShapeAndIndexErrorsPreserveTheOriginal) {
    auto a = matrix<TypeParam>({{2, 1}, {0, 3}});
    const auto original = a;
    const Matrix<TypeParam> row(1, 2);
    const Matrix<TypeParam> column(2, 1);
    EXPECT_THROW(a.getElement(2, 0), std::out_of_range);
    EXPECT_THROW(a.getElement(0, 2), std::out_of_range);
    EXPECT_THROW(a.setElement(2, 0, TypeParam{1}), std::out_of_range);
    EXPECT_THROW(a.setElement(0, 2, TypeParam{1}), std::out_of_range);
    EXPECT_THROW(a = row, std::invalid_argument);
    EXPECT_THROW(a + row, std::invalid_argument);
    EXPECT_THROW(a - row, std::invalid_argument);
    EXPECT_THROW(a * row, std::invalid_argument);
    EXPECT_THROW(a += row, std::invalid_argument);
    EXPECT_THROW(a -= row, std::invalid_argument);
    EXPECT_THROW(a *= column, std::invalid_argument);
    EXPECT_THROW(row.determinant(), std::invalid_argument);
    EXPECT_EQ(a, original);
}

TYPED_TEST(NumberContract, DimensionAndConstructionErrorsAreConsistent) {
    EXPECT_THROW((Matrix<TypeParam>(0, 2)), std::invalid_argument);
    EXPECT_THROW((Matrix<TypeParam>(2, 0)), std::invalid_argument);
    EXPECT_THROW((Matrix<TypeParam>(std::vector<std::vector<TypeParam>>{{}})), std::invalid_argument);
    EXPECT_THROW((Matrix<TypeParam>(std::vector<std::vector<TypeParam>>{{TypeParam{1}}, {TypeParam{1}, TypeParam{2}}})), std::invalid_argument);
    EXPECT_THROW((Matrix<TypeParam>(std::numeric_limits<std::size_t>::max(), 2)), std::length_error);
    Matrix<TypeParam> a;
    EXPECT_THROW(a.setNumRows(0), std::invalid_argument);
    EXPECT_THROW(a.setNumCols(0), std::invalid_argument);
    a.setNumRows(2);
    EXPECT_THROW(a.setNumRows(2), std::logic_error);
    EXPECT_THROW(a.determinant(), std::logic_error);
    EXPECT_THROW(a.setNumCols(std::numeric_limits<std::size_t>::max()), std::length_error);
    EXPECT_EQ(a.getNumCols(), 0);
    a.setNumCols(2);
    EXPECT_EQ(a, Matrix<TypeParam>(2, 2));
}

TYPED_TEST(NumberContract, ToleranceValidationAndNoOpCacheRulesApplyToEveryType) {
    auto a = matrix<TypeParam>({{2, 1}, {0, 3}});
    a.determinant();
    a.setEchelon(a.getEchelon());
    EXPECT_FALSE(a.getCacheInfo().determinantDirty);
    for (double invalid : {-1.0, 1.0, std::numeric_limits<double>::quiet_NaN(),
            std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()}) {
        EXPECT_THROW(a.setEchelon(invalid), std::invalid_argument);
        EXPECT_FALSE(a.getCacheInfo().determinantDirty);
    }
    a.setEchelon(0.25);
    EXPECT_DOUBLE_EQ(a.getEchelon(), 0.25);
    EXPECT_TRUE(a.getCacheInfo().determinantDirty);
}

TYPED_TEST(NumberContract, ParallelSizedArithmeticMatchesASerialOracle) {
    Matrix<TypeParam> a(16, 16);
    Matrix<TypeParam> b(16, 16);
    for (std::size_t row = 0; row < 16; ++row) {
        for (std::size_t col = 0; col < 16; ++col) {
            a.setElement(row, col, TypeParam(int((row + col) % 5) - 2));
            b.setElement(row, col, TypeParam(int((2 * row + col) % 3) - 1));
        }
    }
    const auto sum = a + b;
    const auto difference = a - b;
    const auto product = a * b;
    for (std::size_t row = 0; row < 16; ++row) {
        for (std::size_t col = 0; col < 16; ++col) {
            EXPECT_EQ(sum.getElement(row, col), a.getElement(row, col) + b.getElement(row, col));
            EXPECT_EQ(difference.getElement(row, col), a.getElement(row, col) - b.getElement(row, col));
            int expected = 0;
            for (std::size_t k = 0; k < 16; ++k) {
                expected += (int((row + k) % 5) - 2) * (int((2 * k + col) % 3) - 1);
            }
            EXPECT_EQ(product.getElement(row, col), TypeParam(expected));
        }
    }
}
