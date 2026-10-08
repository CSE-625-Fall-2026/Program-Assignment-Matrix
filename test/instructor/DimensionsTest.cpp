#include "TestSupport.hpp"

class Dimensions : public testing::TestWithParam<bool> {};

TEST_P(Dimensions, EitherDimensionCanBeSetFirstAndEachIsSetExactlyOnce) {
    Matrix<int> a;
    if (GetParam()) {
        a.setNumRows(2);
        EXPECT_EQ(a.getNumRows(), 2);
        EXPECT_EQ(a.getNumCols(), 0);
        EXPECT_THROW(a.setNumRows(2), std::logic_error);
        EXPECT_THROW(a.setNumRows(3), std::logic_error);
        a.setNumCols(3);
    } else {
        a.setNumCols(3);
        EXPECT_EQ(a.getNumRows(), 0);
        EXPECT_EQ(a.getNumCols(), 3);
        EXPECT_THROW(a.setNumCols(3), std::logic_error);
        EXPECT_THROW(a.setNumCols(4), std::logic_error);
        a.setNumRows(2);
    }
    EXPECT_EQ(a, Matrix<int>(2, 3));
    EXPECT_THROW(a.setNumRows(1), std::logic_error);
    EXPECT_THROW(a.setNumCols(1), std::logic_error);
    a.setElement(1, 2, 7);
    EXPECT_EQ(a.getElement(1, 2), 7);
    EXPECT_EQ(a.getNumRows(), 2);
    EXPECT_EQ(a.getNumCols(), 3);
}

TEST_P(Dimensions, PartialMatricesRejectOperationsUntilTheSecondDimensionIsSet) {
    Matrix<double> partial;
    if (GetParam()) {
        partial.setNumRows(2);
    } else {
        partial.setNumCols(2);
    }
    const auto complete = identity<double>(2);
    Matrix<double> destination(2, 2);
    EXPECT_THROW(partial.getElement(0, 0), std::logic_error);
    EXPECT_THROW(partial.setElement(0, 0, 1), std::logic_error);
    EXPECT_THROW(partial + complete, std::logic_error);
    EXPECT_THROW(complete + partial, std::logic_error);
    EXPECT_THROW(partial - complete, std::logic_error);
    EXPECT_THROW(complete - partial, std::logic_error);
    EXPECT_THROW(partial * complete, std::logic_error);
    EXPECT_THROW(complete * partial, std::logic_error);
    EXPECT_THROW(partial += complete, std::logic_error);
    EXPECT_THROW(partial -= complete, std::logic_error);
    EXPECT_THROW(partial *= complete, std::logic_error);
    EXPECT_THROW(partial = complete, std::logic_error);
    EXPECT_THROW(destination = partial, std::logic_error);
    EXPECT_THROW(static_cast<void>(partial == complete), std::logic_error);
    EXPECT_THROW(static_cast<void>(complete != partial), std::logic_error);
    EXPECT_THROW(partial.determinant(), std::logic_error);
    EXPECT_THROW(partial.PLU(), std::logic_error);
    EXPECT_THROW(partial.getPLU_P(), std::logic_error);
    EXPECT_THROW(partial.getPLU_L(), std::logic_error);
    EXPECT_THROW(partial.getPLU_U(), std::logic_error);
    EXPECT_THROW(partial.solve(complete), std::logic_error);
    EXPECT_THROW(complete.solve(partial), std::logic_error);
    EXPECT_EQ(partial.getCacheInfo().pluComputations, 0);
    EXPECT_EQ(partial.getCacheInfo().determinantComputations, 0);
}

TEST_P(Dimensions, CopyingAPartialMatrixPreservesWhichDimensionIsLocked) {
    Matrix<int> original;
    if (GetParam()) {
        original.setNumRows(2);
    } else {
        original.setNumCols(3);
    }
    Matrix<int> copy(original);
    EXPECT_EQ(copy.getNumRows(), original.getNumRows());
    EXPECT_EQ(copy.getNumCols(), original.getNumCols());
    if (GetParam()) {
        EXPECT_THROW(copy.setNumRows(2), std::logic_error);
        copy.setNumCols(3);
        original.setNumCols(4);
    } else {
        EXPECT_THROW(copy.setNumCols(3), std::logic_error);
        copy.setNumRows(2);
        original.setNumRows(4);
    }
    EXPECT_EQ(copy, Matrix<int>(2, 3));
    copy.setElement(0, 0, 9);
    EXPECT_EQ(original.getElement(0, 0), 0);
}

TEST_P(Dimensions, AFailedSecondAllocationDoesNotLockTheUnsetDimension) {
    Matrix<int> a;
    const auto huge = std::numeric_limits<std::size_t>::max();
    if (GetParam()) {
        a.setNumRows(2);
        EXPECT_THROW(a.setNumCols(huge), std::length_error);
        EXPECT_EQ(a.getNumCols(), 0);
        a.setNumCols(3);
    } else {
        a.setNumCols(3);
        EXPECT_THROW(a.setNumRows(huge), std::length_error);
        EXPECT_EQ(a.getNumRows(), 0);
        a.setNumRows(2);
    }
    EXPECT_EQ(a, Matrix<int>(2, 3));
}

INSTANTIATE_TEST_SUITE_P(BothOrders, Dimensions, testing::Bool());

TEST(DimensionContract, InvalidValuesLeaveSettersAvailableForRetry) {
    Matrix<int> a;
    EXPECT_THROW(a.setNumRows(0), std::invalid_argument);
    EXPECT_THROW(a.setNumCols(0), std::invalid_argument);
    EXPECT_EQ(a.getNumRows(), 0);
    EXPECT_EQ(a.getNumCols(), 0);
    a.setNumRows(2);
    EXPECT_THROW(a.setNumCols(0), std::invalid_argument);
    a.setNumCols(3);
    EXPECT_EQ(a, Matrix<int>(2, 3));
}

TEST(DimensionContract, SizedAndVectorConstructorsLockBothDimensions) {
    for (auto& a : std::vector<Matrix<int>>{Matrix<int>(2, 3), matrix<int>({{1, 2, 3}, {4, 5, 6}})}) {
        EXPECT_THROW(a.setNumRows(2), std::logic_error);
        EXPECT_THROW(a.setNumCols(3), std::logic_error);
    }
    Matrix<int> empty(0, 0);
    empty.setNumCols(1);
    empty.setNumRows(1);
    EXPECT_EQ(empty, Matrix<int>(1, 1));
    Matrix<int> fromEmptyVector(std::vector<std::vector<int>>{});
    fromEmptyVector.setNumRows(1);
    fromEmptyVector.setNumCols(1);
    EXPECT_EQ(fromEmptyVector, empty);
}

TEST(DimensionContract, SettingDimensionsInvalidatesAnEmptyDeterminantCache) {
    Matrix<double> a;
    EXPECT_DOUBLE_EQ(a.determinant(), 1);
    a.setNumRows(2);
    EXPECT_TRUE(a.getCacheInfo().determinantDirty);
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 1);
    a.setNumCols(2);
    EXPECT_TRUE(a.getCacheInfo().pluDirty);
    EXPECT_EQ(a.getCacheInfo().pluComputations, 0);
    a.setElement(0, 0, 2);
    a.setElement(1, 1, 3);
    EXPECT_DOUBLE_EQ(a.determinant(), 6);
    EXPECT_EQ(a.getCacheInfo().determinantComputations, 2);
}

struct LifetimeValue {
    static inline int alive = 0;
    int value;
    LifetimeValue() { value = 0; ++alive; }
    LifetimeValue(const LifetimeValue& other) { value = other.value; ++alive; }
    LifetimeValue& operator=(const LifetimeValue& other) { value = other.value; return *this; }
    ~LifetimeValue() { --alive; }
};

TEST(DimensionContract, ElementArrayIsAllocatedOnlyAfterBothDimensionsAreSet) {
    EXPECT_EQ(LifetimeValue::alive, 0);
    {
        Matrix<LifetimeValue> a;
        const auto initialObjects = LifetimeValue::alive;
        a.setNumRows(2);
        EXPECT_EQ(LifetimeValue::alive, initialObjects);
        a.setNumCols(3);
        EXPECT_EQ(LifetimeValue::alive, initialObjects + 6);
        EXPECT_EQ(a.getElement(1, 2).value, 0);
        EXPECT_EQ(LifetimeValue::alive, initialObjects + 6);
    }
    EXPECT_EQ(LifetimeValue::alive, 0);
}
