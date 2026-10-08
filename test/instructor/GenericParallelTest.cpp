#include "TestSupport.hpp"
#include <atomic>
#include <complex>

struct Number {
    int value = 0;
    static inline std::atomic<int> additions = 0;
    static inline std::atomic<int> multiplications = 0;
    static inline std::atomic<int> divisions = 0;
    static inline std::atomic<unsigned> workers = 0;
    static inline std::atomic<bool> throwArithmetic = false;
    static inline std::atomic<bool> throwCopy = false;
    Number() { value = 0; }
    Number(int number) { value = number; }
    Number(const Number& other) { value = other.value; }
    Number& operator=(const Number& other) {
        if (throwCopy) throw std::runtime_error("copy failure");
        value = other.value;
        return *this;
    }
    static void record() {
        workers.fetch_or(1u << (omp_get_thread_num() % 32));
        if (throwArithmetic) throw std::runtime_error("arithmetic failure");
    }
    friend Number operator+(Number a, Number b) {
        record();
        ++additions;
        return Number(a.value + b.value);
    }
    friend Number operator-(Number a, Number b) {
        record();
        return Number(a.value - b.value);
    }
    friend Number operator*(Number a, Number b) {
        record();
        ++multiplications;
        return Number(a.value * b.value);
    }
    friend Number operator/(Number a, Number b) {
        ++divisions;
        return Number(a.value / b.value);
    }
    friend bool operator==(const Number& a, const Number& b) { return a.value == b.value; }
    friend bool operator!=(const Number& a, const Number& b) { return !(a == b); }
};

TEST(Generic, OtherFloatingPointTypesUseGenericDivisionMethods) {
    auto a = matrix<float>({{2, 1}, {1, 3}});
    auto b = matrix<long double>({{2, 1}, {1, 3}});
    a.setEchelon(0.001);
    b.setEchelon(0.001);
    a.PLU();
    b.PLU();
    EXPECT_EQ(a.getPLU_P() * a, a.getPLU_L() * a.getPLU_U());
    EXPECT_EQ(b.getPLU_P() * b, b.getPLU_L() * b.getPLU_U());
    EXPECT_EQ(a.solve(matrix<float>({{1}, {3}})), matrix<float>({{0}, {1}}));
    EXPECT_EQ(b.solve(matrix<long double>({{1}, {3}})), matrix<long double>({{0}, {1}}));
}

TEST(Generic, UserTypeNeedsNoCompoundScalarOperatorsOrOrdering) {
    const auto a = matrix<Number>({{2, 3}, {1, 4}});
    const auto b = matrix<Number>({{1, 0}, {0, 1}});
    EXPECT_EQ(a * b, a);
    EXPECT_EQ((a + b) - b, a);
    auto c = a;
    c += b;
    c -= b;
    c *= b;
    EXPECT_EQ(c, a);
    Number::divisions = 0;
    EXPECT_EQ(a.determinant().value, 5);
    EXPECT_EQ(Number::divisions, 0);
    EXPECT_EQ(matrix<Number>({{1, 2, 3}, {0, 4, 5}, {1, 0, 6}}).determinant().value, 22);
}

TEST(Generic, ComplexValuesSupportArithmeticAndCofactorDeterminant) {
    using C = std::complex<double>;
    const auto a = matrix<C>({{C(1, 1), C(2, 0)}, {C(0, 1), C(3, -1)}});
    EXPECT_EQ(a * identity<C>(2), a);
    EXPECT_EQ(a.determinant(), C(4, 0));
    EXPECT_EQ((a + a) - a, a);
}

TEST(Generic, CachedDeterminantPerformsNoAdditionalScalarArithmetic) {
    auto a = matrix<Number>({{1, 2, 3}, {0, 4, 5}, {1, 0, 6}});
    Number::multiplications = 0;
    EXPECT_EQ(a.determinant().value, 22);
    const auto calculations = Number::multiplications.load();
    EXPECT_GT(calculations, 0);
    EXPECT_EQ(a.determinant().value, 22);
    EXPECT_EQ(Number::multiplications, calculations);
    a.setElement(0, 0, Number{2});
    EXPECT_EQ(Number::multiplications, calculations);
    EXPECT_EQ(a.determinant().value, 46);
    EXPECT_GT(Number::multiplications, calculations);
}

TEST(Generic, FailedAssignmentPreservesValuesAndCachedResult) {
    auto a = matrix<Number>({{2, 3}, {1, 4}});
    const auto b = matrix<Number>({{5, 6}, {7, 8}});
    a.determinant();
    Number::throwCopy = true;
    EXPECT_THROW(a = b, std::runtime_error);
    Number::throwCopy = false;
    EXPECT_EQ(a.getElement(0, 0).value, 2);
    EXPECT_FALSE(a.getCacheInfo().determinantDirty);
    EXPECT_EQ(a.determinant().value, 5);
}

TEST(Generic, FailedElementWriteLeavesCacheDirty) {
    auto a = matrix<Number>({{2}});
    a.determinant();
    Number::throwCopy = true;
    EXPECT_THROW(a.setElement(0, 0, Number{3}), std::runtime_error);
    Number::throwCopy = false;
    EXPECT_TRUE(a.getCacheInfo().determinantDirty);
    EXPECT_EQ(a.determinant().value, 2);
}

TEST(Parallel, ArithmeticActuallyRunsOnMultipleWorkersWhenConfigured) {
    Matrix<Number> a(20, 20);
    Matrix<Number> b(20, 20);
    for (int operation = 0; operation < 3; ++operation) {
        Number::workers = 0;
        if (operation == 0) { const auto result = a + b; EXPECT_EQ(result, a); }
        if (operation == 1) { const auto result = a - b; EXPECT_EQ(result, a); }
        if (operation == 2) { const auto result = a * b; EXPECT_EQ(result, a); }
        const auto workers = Number::workers.load();
        if (omp_get_max_threads() > 1) {
            EXPECT_NE(workers & (workers - 1), 0u);
        } else {
            EXPECT_EQ(workers, 1u);
        }
    }
}

TEST(Parallel, WorkerExceptionsAreRethrownAndOperandsArePreserved) {
    Matrix<Number> a(20, 20);
    const auto b = a;
    Number::throwArithmetic = true;
    EXPECT_THROW(a + b, std::runtime_error);
    EXPECT_THROW(a - b, std::runtime_error);
    EXPECT_THROW(a * b, std::runtime_error);
    EXPECT_THROW(a += b, std::runtime_error);
    EXPECT_THROW(a -= b, std::runtime_error);
    EXPECT_THROW(a *= b, std::runtime_error);
    Number::throwArithmetic = false;
    EXPECT_EQ(a, b);
    EXPECT_EQ(a.getNumRows(), 20);
}

TEST(Parallel, FloatingPointProductsAreIdenticalAcrossThreadCounts) {
    Matrix<double> a(17, 19);
    Matrix<double> b(19, 17);
    for (std::size_t row = 0; row < 17; ++row) {
        for (std::size_t col = 0; col < 19; ++col) {
            a.setElement(row, col, (double(row) - double(col)) / 7);
            b.setElement(col, row, (double(row) + double(col)) / 11);
        }
    }
    const auto oldThreads = omp_get_max_threads();
    omp_set_num_threads(1);
    const auto serial = a * b;
    omp_set_num_threads(4);
    const auto parallel = a * b;
    omp_set_num_threads(oldThreads);
    EXPECT_EQ(serial, parallel);
}

TEST(Parallel, CallsFromAnExistingOpenMPRegionRemainCorrect) {
    const auto a = identity<double>(20);
    std::atomic<int> completed = 0;
    #pragma omp parallel for schedule(static)
    for (int iteration = 0; iteration < 4; ++iteration) {
        const auto product = a * a;
        if (product == a) ++completed;
    }
    EXPECT_EQ(completed, 4);
}
