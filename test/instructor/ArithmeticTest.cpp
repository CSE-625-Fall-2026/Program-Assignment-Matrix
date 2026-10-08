#include "TestSupport.hpp"

template<class T>
class Arithmetic : public testing::Test {};
using ArithmeticTypes = testing::Types<int, long long, double>;
TYPED_TEST_SUITE(Arithmetic, ArithmeticTypes,);

TYPED_TEST(Arithmetic, AdditionSubtractionAndCompoundsHaveExpectedEntries) {
    const auto a = matrix<TypeParam>({{1, -2, 3}, {4, 5, -6}});
    const auto b = matrix<TypeParam>({{7, 8, -9}, {-1, 2, 3}});
    const auto sum = matrix<TypeParam>({{8, 6, -6}, {3, 7, -3}});
    const auto difference = matrix<TypeParam>({{-6, -10, 12}, {5, 3, -9}});
    EXPECT_EQ(a + b, sum);
    EXPECT_EQ(a - b, difference);
    auto c = a;
    EXPECT_EQ(&(c += b), &c);
    EXPECT_EQ(c, sum);
    EXPECT_EQ(&(c -= b), &c);
    EXPECT_EQ(c, a);
    EXPECT_EQ(c -= b, difference);
    EXPECT_EQ(a.getElement(0, 0), TypeParam{1});
    EXPECT_EQ(b.getElement(0, 0), TypeParam{7});
}

TYPED_TEST(Arithmetic, RectangularProductsUseInnerDimensionAndCorrectOrder) {
    const auto a = matrix<TypeParam>({{1, 2, 3}, {4, 5, 6}});
    const auto b = matrix<TypeParam>({{7, 8}, {9, 10}, {11, 12}});
    EXPECT_EQ(a * b, matrix<TypeParam>({{58, 64}, {139, 154}}));
    EXPECT_EQ(b * a, matrix<TypeParam>({{39, 54, 69}, {49, 68, 87}, {59, 82, 105}}));
    EXPECT_EQ(matrix<TypeParam>({{1, 2, 3}}) * matrix<TypeParam>({{4}, {5}, {6}}),
        matrix<TypeParam>({{32}}));
    EXPECT_EQ(matrix<TypeParam>({{1}, {2}, {3}}) * matrix<TypeParam>({{4, 5}}),
        matrix<TypeParam>({{4, 5}, {8, 10}, {12, 15}}));
}

TYPED_TEST(Arithmetic, CompoundProductsAreAliasSafeAndKeepDimensions) {
    auto a = matrix<TypeParam>({{1, 2}, {3, 4}});
    EXPECT_EQ(&(a *= a), &a);
    EXPECT_EQ(a, matrix<TypeParam>({{7, 10}, {15, 22}}));
    auto rectangular = matrix<TypeParam>({{1, 2, 3}, {4, 5, 6}});
    const auto old = rectangular;
    rectangular *= identity<TypeParam>(3);
    EXPECT_EQ(rectangular, old);
    Matrix<TypeParam> shapeChanging(3, 2);
    EXPECT_THROW(rectangular *= shapeChanging, std::invalid_argument);
    EXPECT_EQ(rectangular, old);
    auto self = old;
    self += self;
    EXPECT_EQ(self, old + old);
    self -= self;
    EXPECT_EQ(self, Matrix<TypeParam>(2, 3));
}

TYPED_TEST(Arithmetic, RejectsAllShapeMismatchesWithoutChangingOperands) {
    auto a = matrix<TypeParam>({{1, 2, 3}, {4, 5, 6}});
    const auto old = a;
    Matrix<TypeParam> transposed(3, 2);
    Matrix<TypeParam> wrongInner(2, 4);
    EXPECT_THROW(a + transposed, std::invalid_argument);
    EXPECT_THROW(a - transposed, std::invalid_argument);
    EXPECT_THROW(a += transposed, std::invalid_argument);
    EXPECT_THROW(a -= transposed, std::invalid_argument);
    EXPECT_THROW(a * wrongInner, std::invalid_argument);
    EXPECT_THROW(a *= wrongInner, std::invalid_argument);
    EXPECT_EQ(a, old);
}

TYPED_TEST(Arithmetic, EmptyOperationsAndEqualityAreConsistent) {
    Matrix<TypeParam> a;
    Matrix<TypeParam> b;
    EXPECT_EQ(a, b);
    EXPECT_EQ(a + b, a);
    EXPECT_EQ(a - b, a);
    EXPECT_EQ(a * b, a);
    EXPECT_EQ(a += b, b);
    EXPECT_EQ(a -= b, b);
    EXPECT_EQ(a *= b, b);
    EXPECT_NE(a, Matrix<TypeParam>(1, 1));
    EXPECT_THROW(a * Matrix<TypeParam>(1, 1), std::invalid_argument);
}

TYPED_TEST(Arithmetic, EqualityChecksShapeAndEveryEntry) {
    auto a = matrix<TypeParam>({{1, 2}, {3, 4}});
    const auto copy = a;
    EXPECT_TRUE(a == copy);
    EXPECT_FALSE(a != copy);
    EXPECT_FALSE(a == matrix<TypeParam>({{1, 2, 3, 4}}));
    for (std::size_t row = 0; row < 2; ++row) {
        for (std::size_t col = 0; col < 2; ++col) {
            auto changed = copy;
            changed.setElement(row, col, TypeParam{99});
            EXPECT_NE(changed, copy);
            EXPECT_NE(copy, changed);
        }
    }
}

TEST(ArithmeticContract, DoubleEqualityIsExactAndIndependentOfEchelon) {
    auto a = matrix<double>({{1}});
    auto b = matrix<double>({{std::nextafter(1.0, 2.0)}});
    a.setEchelon(0.1);
    EXPECT_NE(a, b);
    EXPECT_EQ(matrix<double>({{-0.0}}), matrix<double>({{0.0}}));
    const auto nan = matrix<double>({{std::numeric_limits<double>::quiet_NaN()}});
    EXPECT_FALSE(nan == nan);
    EXPECT_TRUE(nan != nan);
}

TEST(ArithmeticContract, DeterministicRectangularProductsMatchIndependentSerialOracle) {
    std::mt19937 random(625);
    std::uniform_int_distribution<int> value(-4, 4);
    for (std::size_t rows : {1, 2, 7, 17}) {
        for (std::size_t inner : {1, 3, 8}) {
            for (std::size_t cols : {1, 4, 19}) {
                Matrix<int> a(rows, inner);
                Matrix<int> b(inner, cols);
                for (std::size_t row = 0; row < rows; ++row) {
                    for (std::size_t col = 0; col < inner; ++col) {
                        a.setElement(row, col, value(random));
                    }
                }
                for (std::size_t row = 0; row < inner; ++row) {
                    for (std::size_t col = 0; col < cols; ++col) {
                        b.setElement(row, col, value(random));
                    }
                }
                const auto actual = a * b;
                for (std::size_t row = 0; row < rows; ++row) {
                    for (std::size_t col = 0; col < cols; ++col) {
                        int expected = 0;
                        for (std::size_t k = 0; k < inner; ++k) {
                            expected += a.getElement(row, k) * b.getElement(k, col);
                        }
                        EXPECT_EQ(actual.getElement(row, col), expected);
                    }
                }
            }
        }
    }
}
