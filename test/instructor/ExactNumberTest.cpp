#include "TestSupport.hpp"
#include <ArbitraryInteger.hpp>
#include <RationalNumber.hpp>
#include <string>

using Integer = ArbitraryInteger;
using Fraction = RationalNumber<Integer>;

TEST(ArbitraryIntegerMatrix, LargeEntriesCancelToAnExactUnitDeterminant) {
    const Integer large("1000000000000000000000000000000");
    const auto a = matrix<Integer>({{large, large + 1}, {large - 1, large}});
    EXPECT_EQ(a.determinant(), Integer{1});
    EXPECT_EQ(a.determinant(), Integer{1});
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 1);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 0);
    const auto square = a * a;
    EXPECT_EQ(square.getElement(0, 0).toString(), std::string("1") + std::string(60, '9'));
    EXPECT_EQ(square.determinant(), Integer{1});
    auto difference = a - a;
    EXPECT_EQ(difference, Matrix<Integer>(2, 2));
    EXPECT_EQ((a + a) - a, a);
}

TEST(ArbitraryIntegerMatrix, LargeTriangularDeterminantsAndPermutationSignsAreExact) {
    const Integer large("1000000000000000000000000000000");
    const auto upper = matrix<Integer>({{large, 1, -2}, {0, large, 3}, {0, 0, large}});
    EXPECT_EQ(upper.determinant().toString(), std::string("1") + std::string(90, '0'));
    const auto swapped = matrix<Integer>({{0, large, 3}, {large, 1, -2}, {0, 0, large}});
    EXPECT_EQ(swapped.determinant(), -upper.determinant());
}

TEST(ArbitraryIntegerMatrix, ExhaustiveTwoByTwoSmallEntriesMatchIntegerOracle) {
    for (int a = -2; a <= 2; ++a) {
        for (int b = -2; b <= 2; ++b) {
            for (int c = -2; c <= 2; ++c) {
                for (int d = -2; d <= 2; ++d) {
                    EXPECT_EQ(matrix<Integer>({{a, b}, {c, d}}).determinant(), Integer(a*d - b*c));
                }
            }
        }
    }
}

TEST(ArbitraryIntegerMatrix, ThreeAndFourDimensionalCasesMatchIndependentPermutationOracle) {
    std::mt19937 random(270625);
    std::uniform_int_distribution<int> entry(-3, 3);
    for (std::size_t n : {3, 4}) {
        for (int sample = 0; sample < 12; ++sample) {
            Matrix<Integer> a(n, n);
            Matrix<long long> reference(n, n);
            for (std::size_t row = 0; row < n; ++row) {
                for (std::size_t col = 0; col < n; ++col) {
                    const auto value = entry(random);
                    a.setElement(row, col, Integer{value});
                    reference.setElement(row, col, value);
                }
            }
            EXPECT_EQ(a.determinant(), Integer(permutationDeterminant(reference)));
        }
    }
}

TEST(RationalMatrix, FractionalArithmeticHasExplicitIndependentReferenceValues) {
    const auto a = matrix<Fraction>({{Fraction(1, 2), Fraction(1, 3)}, {Fraction(2, 5), Fraction(3, 7)}});
    const auto b = matrix<Fraction>({{Fraction(1, 4), Fraction(-2, 3)}, {Fraction(3, 5), Fraction(5, 6)}});
    EXPECT_EQ(a + b, matrix<Fraction>({{Fraction(3, 4), Fraction(-1, 3)}, {Fraction(1), Fraction(53, 42)}}));
    EXPECT_EQ(a - b, matrix<Fraction>({{Fraction(1, 4), Fraction(1)}, {Fraction(-1, 5), Fraction(-17, 42)}}));
    EXPECT_EQ(a * b, matrix<Fraction>({{Fraction(13, 40), Fraction(-1, 18)}, {Fraction(5, 14), Fraction(19, 210)}}));
    EXPECT_EQ(a.determinant(), Fraction(17, 210));
    auto updated = a;
    updated += b;
    EXPECT_EQ(updated, a + b);
    updated -= b;
    EXPECT_EQ(updated, a);
    updated *= b;
    EXPECT_EQ(updated, a * b);
}

TEST(RationalMatrix, EquivalentFractionsCompareEquallyAndNormalizeStoredValues) {
    const auto a = matrix<Fraction>({{Fraction(2, 4), Fraction(-3, -9)}});
    const auto b = matrix<Fraction>({{Fraction(1, 2), Fraction(1, 3)}});
    EXPECT_EQ(a, b);
    EXPECT_EQ(a.getElement(0, 0).getNumerator(), Integer{1});
    EXPECT_EQ(a.getElement(0, 0).getDenominator(), Integer{2});
    EXPECT_NE(a, matrix<Fraction>({{Fraction(1, 2), Fraction(1, 4)}}));
}

TEST(RationalMatrix, ExhaustiveTwoByTwoFractionsMatchAnIntegerNumeratorOracle) {
    for (int a = -2; a <= 2; ++a) {
        for (int b = -2; b <= 2; ++b) {
            for (int c = -2; c <= 2; ++c) {
                for (int d = -2; d <= 2; ++d) {
                    const auto value = matrix<Fraction>({{Fraction(a, 2), Fraction(b, 3)},
                                                        {Fraction(c, 5), Fraction(d, 7)}});
                    EXPECT_EQ(value.determinant(), Fraction(15*a*d - 14*b*c, 210));
                }
            }
        }
    }
}

TEST(RationalMatrix, LargeDenominatorsAndCancellationRemainExact) {
    const Integer large("1000000000000000000000000000000");
    const auto a = matrix<Fraction>({{Fraction(Integer{1}, large), Fraction{1}},
                                    {Fraction{1}, Fraction(large + 1)}});
    EXPECT_EQ(a.determinant(), Fraction(Integer{1}, large));
    EXPECT_EQ(a.determinant().getDenominator(), large);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 0);
}

TEST(RationalMatrix, CofactorExpansionWorksForNonintegralThreeAndFourDimensionalMatrices) {
    for (std::size_t n : {3, 4}) {
        Matrix<Fraction> a(n, n);
        Matrix<long long> numerator(n, n);
        long long denominator = 1;
        for (std::size_t row = 0; row < n; ++row) {
            denominator *= static_cast<long long>(row + 2);
            for (std::size_t col = 0; col < n; ++col) {
                const int entry = row == col ? 5 : int(row + col) - 1;
                numerator.setElement(row, col, entry);
                a.setElement(row, col, Fraction(Integer{entry}, Integer(static_cast<long long>(row + 2))));
            }
        }
        EXPECT_EQ(a.determinant(), Fraction(Integer(permutationDeterminant(numerator)), Integer{denominator}));
    }
}

TEST(RationalMatrix, FractionalEntryMutationInvalidatesTheCache) {
    auto a = matrix<Fraction>({{Fraction(1, 2), Fraction(1, 3)}, {Fraction(2, 5), Fraction(3, 7)}});
    EXPECT_EQ(a.determinant(), Fraction(17, 210));
    a.setElement(0, 0, Fraction(2, 3));
    EXPECT_TRUE(a.getCacheInfo().determinantDirty);
    EXPECT_EQ(a.determinant(), Fraction(16, 105));
    a.setEchelon(0.9);
    EXPECT_EQ(a.determinant(), Fraction(16, 105));
    EXPECT_EQ(a.getCacheInfo().pluComputations, 0);
}

TEST(RationalMatrix, ParallelFractionalProductsHaveExactExpectedValues) {
    Matrix<Fraction> a(16, 16);
    Matrix<Fraction> b(16, 16);
    for (std::size_t row = 0; row < 16; ++row) {
        for (std::size_t col = 0; col < 16; ++col) {
            a.setElement(row, col, Fraction(int(row % 3) - 1, 2));
            b.setElement(row, col, Fraction(int(col % 3) - 1, 3));
        }
    }
    const auto product = a * b;
    for (std::size_t row = 0; row < 16; ++row) {
        for (std::size_t col = 0; col < 16; ++col) {
            EXPECT_EQ(product.getElement(row, col), Fraction(8 * (int(row % 3) - 1) * (int(col % 3) - 1), 3));
        }
    }
}

TEST(RationalMatrix, ExhaustiveSmallPLUAndSolvePreserveExactFractions) {
    const auto expected = matrix<Fraction>({{Fraction(1, 2)}, {Fraction(-2, 3)}});
    for (int a = -2; a <= 2; ++a) {
        for (int b = -2; b <= 2; ++b) {
            for (int c = -2; c <= 2; ++c) {
                for (int d = -2; d <= 2; ++d) {
                    const auto value = matrix<Fraction>({{a, b}, {c, d}});
                    if (a*d == b*c) {
                        EXPECT_THROW(value.PLU(), std::domain_error);
                        EXPECT_THROW(value.solve(value * expected), std::domain_error);
                    } else {
                        value.PLU();
                        EXPECT_EQ(value.getPLU_P() * value, value.getPLU_L() * value.getPLU_U());
                        EXPECT_EQ(value.solve(value * expected), expected);
                    }
                    EXPECT_EQ(value.getCacheInfo().pluComputations, 1);
                    EXPECT_EQ(value.getCacheInfo().determinantComputations, 0);
                }
            }
        }
    }
}

TEST(RationalMatrix, PivotsAndSolutionsWorkBeyondDoubleRangeWithoutConversion) {
    const Integer huge(std::string("1") + std::string(400, '0'));
    const Fraction tiny(Integer{1}, huge);
    const auto a = matrix<Fraction>({{tiny, Fraction(huge)}, {Fraction(huge), tiny}});
    const auto expected = matrix<Fraction>({{tiny}, {Fraction(huge)}});
    EXPECT_DOUBLE_EQ(a.getEchelon(), 0);
    EXPECT_EQ(a.solve(a * expected), expected);
    EXPECT_EQ(a.getPLU_P(), matrix<Fraction>({{0, 1}, {1, 0}}));
    EXPECT_EQ(a.getPLU_P() * a, a.getPLU_L() * a.getPLU_U());
    const auto diagonal = matrix<Fraction>({{tiny, 0}, {0, Fraction(huge)}});
    EXPECT_EQ(diagonal.solve(diagonal), identity<Fraction>(2));
}

TEST(RationalMatrix, ManualToleranceUsesExactBinaryFractionWithoutChangingCofactors) {
    auto atBoundary = matrix<Fraction>({{1, 0}, {0, Fraction(1, 10)}});
    atBoundary.setEchelon(0.1);
    EXPECT_THROW(atBoundary.PLU(), std::domain_error);
    EXPECT_EQ(atBoundary.determinant(), Fraction(1, 10));
    auto aboveBoundary = matrix<Fraction>({{1, 0}, {0, Fraction(1000001, 10000000)}});
    aboveBoundary.setEchelon(0.1);
    EXPECT_NO_THROW(aboveBoundary.PLU());
    atBoundary.setEchelon(0);
    EXPECT_EQ(atBoundary.solve(atBoundary), identity<Fraction>(2));
    EXPECT_EQ(atBoundary.getCacheInfo().pluComputations, 2);
}

TEST(RationalMatrix, TinyRepresentableToleranceIsConvertedExactly) {
    const Integer huge(std::string("1") + std::string(400, '0'));
    auto a = matrix<Fraction>({{1, 0}, {0, Fraction(Integer{1}, huge)}});
    a.setEchelon(std::numeric_limits<double>::denorm_min());
    EXPECT_THROW(a.PLU(), std::domain_error);
    a.setEchelon(0);
    EXPECT_EQ(a.solve(a), identity<Fraction>(2));
}

TEST(RationalMatrix, LaterRowSwapsPreservePreviouslyComputedLowerFactors) {
    const auto a = matrix<Fraction>({{9, 1, 1}, {3, 1, 8}, {1, 7, 2}});
    EXPECT_EQ(a.getPLU_P(), matrix<Fraction>({{1, 0, 0}, {0, 0, 1}, {0, 1, 0}}));
    EXPECT_EQ(a.getPLU_L().getElement(1, 0), Fraction(1, 9));
    EXPECT_EQ(a.getPLU_L().getElement(2, 0), Fraction(1, 3));
    EXPECT_EQ(a.getPLU_P() * a, a.getPLU_L() * a.getPLU_U());
}
