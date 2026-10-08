#include <Matrix.hpp>
#include <gtest/gtest.h>
#include <vector>

TEST(Student, ASystemCanBeSolvedUsingThePublicAPI) {
    const Matrix<double> a(std::vector<std::vector<double>>{{2, 0}, {0, 4}});
    const Matrix<double> b(std::vector<std::vector<double>>{{6}, {8}});
    const auto x = a.solve(b);
    EXPECT_DOUBLE_EQ(x.getElement(0, 0), 3);
    EXPECT_DOUBLE_EQ(x.getElement(1, 0), 2);
}
