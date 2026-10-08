#include "TestSupport.hpp"
#include <type_traits>

template<class T>
class Construction : public testing::Test {};
using ElementTypes = testing::Types<int, long long, double, float>;
TYPED_TEST_SUITE(Construction, ElementTypes,);

TYPED_TEST(Construction, DefaultIsEmptyAndInitiallyDirty) {
    Matrix<TypeParam> a;
    EXPECT_EQ(a.getNumRows(), 0);
    EXPECT_EQ(a.getNumCols(), 0);
    EXPECT_TRUE(a.getCacheInfo().determinantDirty);
    EXPECT_TRUE(a.getCacheInfo().pluDirty);
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 0);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 0);
    EXPECT_THROW(a.getElement(0, 0), std::out_of_range);
    EXPECT_THROW(a.setElement(0, 0, TypeParam{}), std::out_of_range);
}

TYPED_TEST(Construction, DimensionsValueInitializeEveryElement) {
    for (std::size_t rows = 1; rows <= 7; ++rows) {
        for (std::size_t cols = 1; cols <= 7; ++cols) {
            Matrix<TypeParam> a(rows, cols);
            ASSERT_EQ(a.getNumRows(), rows);
            ASSERT_EQ(a.getNumCols(), cols);
            for (std::size_t row = 0; row < rows; ++row) {
                for (std::size_t col = 0; col < cols; ++col) {
                    EXPECT_EQ(a.getElement(row, col), TypeParam{});
                    a.setElement(row, col, TypeParam(row * cols + col));
                }
            }
            for (std::size_t row = 0; row < rows; ++row) {
                for (std::size_t col = 0; col < cols; ++col) {
                    EXPECT_EQ(a.getElement(row, col), TypeParam(row * cols + col));
                }
            }
        }
    }
}

TYPED_TEST(Construction, VectorInputIsCopiedAndNotRetained) {
    std::vector<std::vector<TypeParam>> input{{1, 2, 3}, {4, 5, 6}};
    Matrix<TypeParam> a(input);
    input[0][0] = 99;
    input.clear();
    EXPECT_EQ(a.getNumRows(), 2);
    EXPECT_EQ(a.getNumCols(), 3);
    EXPECT_EQ(a.getElement(0, 0), TypeParam{1});
    EXPECT_EQ(a.getElement(1, 2), TypeParam{6});
}

TYPED_TEST(Construction, RejectsRaggedAndPartiallyEmptyDimensions) {
    EXPECT_THROW((Matrix<TypeParam>(0, 1)), std::invalid_argument);
    EXPECT_THROW((Matrix<TypeParam>(1, 0)), std::invalid_argument);
    EXPECT_NO_THROW((Matrix<TypeParam>(0, 0)));
    EXPECT_NO_THROW((Matrix<TypeParam>(std::vector<std::vector<TypeParam>>{})));
    EXPECT_THROW((Matrix<TypeParam>(std::vector<std::vector<TypeParam>>{{}})), std::invalid_argument);
    EXPECT_THROW((Matrix<TypeParam>(std::vector<std::vector<TypeParam>>{{1}, {}})), std::invalid_argument);
    EXPECT_THROW((Matrix<TypeParam>(std::vector<std::vector<TypeParam>>{{1}, {2, 3}})), std::invalid_argument);
}

TYPED_TEST(Construction, RejectsDimensionsThatOverflowStorageOrLoopIndices) {
    const auto huge = std::numeric_limits<std::size_t>::max();
    EXPECT_THROW((Matrix<TypeParam>(huge, 2)), std::length_error);
    EXPECT_THROW((Matrix<TypeParam>(2, huge)), std::length_error);
    EXPECT_THROW((Matrix<TypeParam>(huge, huge)), std::length_error);
    EXPECT_THROW((Matrix<TypeParam>(1, huge)), std::length_error);
}

TYPED_TEST(Construction, ChecksBothCoordinatesIncludingNegativeConversions) {
    Matrix<TypeParam> a(2, 3);
    for (const auto [row, col] : std::array<std::pair<std::size_t, std::size_t>, 6>{
            {{2, 0}, {0, 3}, {2, 3}, {99, 1}, {1, std::size_t(-1)}, {std::size_t(-1), 1}}}) {
        EXPECT_THROW(a.getElement(row, col), std::out_of_range);
        EXPECT_THROW(a.setElement(row, col, TypeParam{1}), std::out_of_range);
    }
    EXPECT_EQ(a.getElement(0, 0), TypeParam{});
}

TYPED_TEST(Construction, CopiesHaveIndependentLifetimeAndValues) {
    auto a = matrix<TypeParam>({{1, 2}, {3, 4}});
    a.setEchelon(0.01);
    Matrix<TypeParam> copy(a);
    copy.setElement(0, 0, TypeParam{9});
    EXPECT_EQ(a.getElement(0, 0), TypeParam{1});
    EXPECT_EQ(copy.getEchelon(), 0.01);
    const auto fromTemporary = matrix<TypeParam>({{7, 8}});
    EXPECT_EQ(fromTemporary.getElement(0, 1), TypeParam{8});
    Matrix<TypeParam> fromRvalue(std::move(a));
    EXPECT_EQ(fromRvalue.getElement(1, 1), TypeParam{4});
    EXPECT_EQ(a.getNumRows(), 2);
    EXPECT_EQ(a.getElement(0, 0), TypeParam{1});
}

TYPED_TEST(Construction, AssignmentPreservesDimensionsToleranceAndSource) {
    auto a = matrix<TypeParam>({{1, 2}, {3, 4}});
    auto b = matrix<TypeParam>({{5, 6}, {7, 8}});
    a.setEchelon(0.1);
    b.setEchelon(0.2);
    EXPECT_EQ(&(a = b), &a);
    EXPECT_EQ(a, b);
    EXPECT_EQ(a.getEchelon(), 0.1);
    a.setElement(0, 0, TypeParam{9});
    EXPECT_EQ(b.getElement(0, 0), TypeParam{5});
    auto original = a;
    Matrix<TypeParam> wrong(1, 4);
    EXPECT_THROW(a = wrong, std::invalid_argument);
    EXPECT_EQ(a, original);
    Matrix<TypeParam> empty;
    EXPECT_THROW(empty = a, std::invalid_argument);
    EXPECT_EQ(empty.getNumRows(), 0);
    EXPECT_EQ(&(a = a), &a);
    EXPECT_EQ(a, original);
}

TEST(ConstructionContract, AccessReturnsAnIndependentValue) {
    static_assert(std::is_same_v<decltype(std::declval<const Matrix<int>&>().getElement(0, 0)), int>);
    static_assert(std::is_same_v<decltype(std::declval<Matrix<int>&>() += std::declval<const Matrix<int>&>()), Matrix<int>&>);
    auto a = matrix<int>({{3}});
    auto value = a.getElement(0, 0);
    value = 9;
    EXPECT_EQ(value, 9);
    EXPECT_EQ(a.getElement(0, 0), 3);
}
