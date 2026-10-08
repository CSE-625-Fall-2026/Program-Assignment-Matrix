#include "TestSupport.hpp"
#include <complex>

using Complex = std::complex<double>;

TEST(ComplexMatrix, NonrealArithmeticHasExplicitReferenceValues) {
    const auto a = matrix<Complex>({{Complex(1, 1), Complex(2, 0)}, {Complex(0, 1), Complex(3, 2)}});
    const auto b = matrix<Complex>({{Complex(2, -1), Complex(0, 1)}, {Complex(1, 0), Complex(-1, 2)}});
    EXPECT_EQ(a + b, matrix<Complex>({{Complex(3, 0), Complex(2, 1)}, {Complex(1, 1), Complex(2, 4)}}));
    EXPECT_EQ(a - b, matrix<Complex>({{Complex(-1, 2), Complex(2, -1)}, {Complex(-1, 1), Complex(4, 0)}}));
    EXPECT_EQ(a * b, matrix<Complex>({{Complex(5, 1), Complex(-3, 5)}, {Complex(4, 4), Complex(-8, 4)}}));
    EXPECT_EQ(a.determinant(), Complex(1, 3));
    auto updated = a;
    updated += b;
    EXPECT_EQ(updated, a + b);
    updated -= b;
    EXPECT_EQ(updated, a);
    updated *= b;
    EXPECT_EQ(updated, a * b);
}

TEST(ComplexMatrix, ExhaustiveTwoByTwoCasesPreserveRealAndImaginaryParts) {
    const std::array<Complex, 4> values{Complex(0, 0), Complex(1, 0), Complex(0, 1), Complex(1, 1)};
    for (const auto& a : values) {
        for (const auto& b : values) {
            for (const auto& c : values) {
                for (const auto& d : values) {
                    const auto m = matrix<Complex>({{a, b}, {c, d}});
                    const double real = a.real()*d.real() - a.imag()*d.imag()
                        - b.real()*c.real() + b.imag()*c.imag();
                    const double imaginary = a.real()*d.imag() + a.imag()*d.real()
                        - b.real()*c.imag() - b.imag()*c.real();
                    EXPECT_EQ(m.determinant(), Complex(real, imaginary));
                    EXPECT_EQ(m * identity<Complex>(2), m);
                }
            }
        }
    }
}

TEST(ComplexMatrix, SeededThreeAndFourDimensionalDeterminantsMatchPermutationExpansion) {
    std::mt19937 random(625220);
    std::uniform_int_distribution<int> part(-2, 2);
    for (std::size_t n : {3, 4}) {
        for (int sample = 0; sample < 24; ++sample) {
            Matrix<Complex> a(n, n);
            for (std::size_t row = 0; row < n; ++row) {
                for (std::size_t col = 0; col < n; ++col) {
                    a.setElement(row, col, Complex(part(random), part(random)));
                }
            }
            EXPECT_EQ(a.determinant(), permutationDeterminant(a));
            EXPECT_EQ(a.getCacheInfo().pluComputations, 0);
        }
    }
}

TEST(ComplexMatrix, FractionalComplexValuesUseBothPartsWithoutConjugating) {
    const auto a = matrix<Complex>({{Complex(0.5, 0.25), Complex(-0.25, 0.5)}});
    const auto b = matrix<Complex>({{Complex(0.5, -0.25)}, {Complex(0.25, 0.5)}});
    EXPECT_EQ((a * b).getElement(0, 0), Complex(0, 0));
    const auto row = matrix<Complex>({{Complex(0, 1), Complex(0, 1)}});
    const auto column = matrix<Complex>({{Complex(0, 1)}, {Complex(0, 1)}});
    EXPECT_EQ((row * column).getElement(0, 0), Complex(-2, 0));
}

TEST(ComplexMatrix, ImaginaryChangesInvalidateDeterminantAndEquality) {
    auto a = matrix<Complex>({{Complex(1, 1), Complex(2, 0)}, {Complex(0, 1), Complex(3, 2)}});
    const auto original = a;
    EXPECT_EQ(a.determinant(), Complex(1, 3));
    a.setElement(0, 0, Complex(1, 2));
    EXPECT_NE(a, original);
    EXPECT_TRUE(a.getCacheInfo().determinantDirty);
    EXPECT_EQ(a.determinant(), Complex(-1, 6));
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 2);
    a.setEchelon(0.9);
    EXPECT_EQ(a.determinant(), Complex(-1, 6));
    EXPECT_EQ(a.getCacheInfo().pluComputations, 0);
}

TEST(ComplexMatrix, ParallelNonrealProductsMatchSerialResults) {
    Matrix<Complex> a(16, 16);
    Matrix<Complex> b(16, 16);
    for (std::size_t row = 0; row < 16; ++row) {
        for (std::size_t col = 0; col < 16; ++col) {
            a.setElement(row, col, Complex(int(row % 3) - 1, int(col % 3) - 1));
            b.setElement(row, col, Complex(int(col % 3) - 1, int(row % 3) - 1));
        }
    }
    const auto product = a * b;
    for (std::size_t row = 0; row < 16; ++row) {
        for (std::size_t col = 0; col < 16; ++col) {
            Complex expected{};
            for (std::size_t inner = 0; inner < 16; ++inner) {
                expected += a.getElement(row, inner) * b.getElement(inner, col);
            }
            EXPECT_EQ(product.getElement(row, col), expected);
        }
    }
}

void expectComplexNear(const Matrix<Complex>& actual, const Matrix<Complex>& expected) {
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

TEST(ComplexMatrix, PLUPivotsByMagnitudeAndSolvesNonrealSystems) {
    const auto a = matrix<Complex>({{Complex(0, 4), 2}, {Complex(3, 4), Complex(1, -1)}});
    const auto expected = matrix<Complex>({{Complex(0.5, -0.25), Complex(2, 1)},
                                          {Complex(1, 3), Complex(-1, 0.5)}});
    EXPECT_DOUBLE_EQ(a.getEchelon(), 1e-12);
    EXPECT_EQ(a.getPLU_P(), matrix<Complex>({{0, 1}, {1, 0}}));
    expectComplexNear(a.getPLU_P() * a, a.getPLU_L() * a.getPLU_U());
    expectComplexNear(a.solve(a * expected), expected);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 1);
}

TEST(ComplexMatrix, EqualMagnitudePivotsChooseTheFirstRow) {
    const auto a = matrix<Complex>({{Complex(3, 4), 1}, {Complex(0, -5), 2}});
    EXPECT_EQ(a.getPLU_P(), identity<Complex>(2));
    expectComplexNear(a.getPLU_P() * a, a.getPLU_L() * a.getPLU_U());
}

TEST(ComplexMatrix, ExhaustiveNonrealTwoByTwoPLUAndSolveMatchKnownAnswers) {
    const std::array<Complex, 4> values{Complex{}, Complex{1}, Complex(0, 1), Complex(1, 1)};
    const auto expected = matrix<Complex>({{Complex(0.5, 1)}, {Complex(-1, 0.25)}});
    for (auto a : values) {
        for (auto b : values) {
            for (auto c : values) {
                for (auto d : values) {
                    const auto value = matrix<Complex>({{a, b}, {c, d}});
                    if (a*d == b*c) {
                        EXPECT_THROW(value.PLU(), std::domain_error);
                        EXPECT_THROW(value.solve(value * expected), std::domain_error);
                    } else {
                        expectComplexNear(value.getPLU_P() * value, value.getPLU_L() * value.getPLU_U());
                        expectComplexNear(value.solve(value * expected), expected);
                    }
                    EXPECT_EQ(value.getCacheInfo().pluComputations, 1);
                }
            }
        }
    }
}

TEST(ComplexMatrix, NonfiniteRealOrImaginaryInputsDoNotPublishFactors) {
    for (double invalid : {std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
        for (Complex entry : {Complex(invalid, 1), Complex(1, invalid)}) {
            const auto a = matrix<Complex>({{entry}});
            EXPECT_THROW(a.PLU(), std::invalid_argument);
            EXPECT_THROW(a.getPLU_P(), std::invalid_argument);
            EXPECT_THROW(a.getPLU_L(), std::invalid_argument);
            EXPECT_THROW(a.getPLU_U(), std::invalid_argument);
            EXPECT_THROW(a.solve(identity<Complex>(1)), std::invalid_argument);
            EXPECT_EQ(a.getCacheInfo().pluComputations, 0);
            EXPECT_TRUE(a.getCacheInfo().pluDirty);
            const auto valid = identity<Complex>(1);
            EXPECT_THROW(valid.solve(a), std::invalid_argument);
            EXPECT_EQ(valid.getCacheInfo().pluComputations, 0);
        }
    }
}

TEST(ComplexMatrix, NumericalOverflowFailsWithoutPublishingIncompleteFactors) {
    const double huge = std::numeric_limits<double>::max();
    const auto elimination = matrix<Complex>({{huge, huge}, {-huge, huge}});
    EXPECT_THROW(elimination.PLU(), std::overflow_error);
    EXPECT_EQ(elimination.getCacheInfo().pluComputations, 0);
    EXPECT_TRUE(elimination.getCacheInfo().pluDirty);
    const auto norm = matrix<Complex>({{Complex(huge, huge)}});
    EXPECT_THROW(norm.PLU(), std::overflow_error);
    EXPECT_EQ(norm.getCacheInfo().pluComputations, 0);
    auto laterNorm = matrix<Complex>({{1, Complex(8e307, 8e307)}, {-1, Complex(8e307, 8e307)}});
    laterNorm.setEchelon(0);
    EXPECT_THROW(laterNorm.PLU(), std::overflow_error);
    EXPECT_TRUE(laterNorm.getCacheInfo().pluDirty);
    EXPECT_EQ(laterNorm.getCacheInfo().pluComputations, 0);
    const auto tiny = matrix<Complex>({{Complex(1e-300, 0)}});
    EXPECT_THROW(tiny.solve(matrix<Complex>({{Complex(1e300, 0)}})), std::overflow_error);
    EXPECT_EQ(tiny.getCacheInfo().pluComputations, 1);
}

TEST(ComplexMatrix, ParallelNonrealEliminationAndSolveMatchKnownValues) {
    Matrix<Complex> a(17, 17);
    Matrix<Complex> expected(17, 4);
    for (std::size_t row = 0; row < 17; ++row) {
        a.setElement(row, row, Complex(3, 1));
        a.setElement(row, (row + 1) % 17, Complex(0.5, -0.5));
        for (std::size_t col = 0; col < 4; ++col) {
            expected.setElement(row, col, Complex(int(row % 3) - 1, int(col) - 2));
        }
    }
    expectComplexNear(a.solve(a * expected), expected);
    expectComplexNear(a.getPLU_P() * a, a.getPLU_L() * a.getPLU_U());
}
