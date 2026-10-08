#include <Matrix.hpp>
#include <complex>
#include <iostream>
#include <stdexcept>
#include <vector>

int main() {
    std::cout << std::boolalpha;
    {
        std::cout << "\nint\n";
        Matrix<int> a;
        a.setNumRows(2);
        a.setNumCols(2);
        a.setElement(0, 0, 0);
        a.setElement(0, 1, 2);
        a.setElement(1, 0, 2);
        a.setElement(1, 1, 1);
        std::cout << "A is " << a.getNumRows() << " x " << a.getNumCols() << '\n';
        for (std::size_t row = 0; row < 2; ++row) {
            std::cout << a.getElement(row, 0) << ' ' << a.getElement(row, 1) << '\n';
        }

        Matrix<int> identity(std::vector<std::vector<int>>{{1, 0}, {0, 1}});
        Matrix<int> copy(a);
        std::cout << "Copy equals A: " << (copy == a) << '\n';
        copy.setElement(0, 0, 3);
        std::cout << "Changed copy differs from A: " << (copy != a) << '\n';

        Matrix<int> sum = a + identity;
        Matrix<int> difference = a - identity;
        Matrix<int> product = a * a;
        std::cout << "A + I, A - I, A * A\n";
        for (std::size_t row = 0; row < 2; ++row) {
            std::cout << sum.getElement(row, 0) << ' ' << sum.getElement(row, 1) << " | "
                      << difference.getElement(row, 0) << ' ' << difference.getElement(row, 1) << " | "
                      << product.getElement(row, 0) << ' ' << product.getElement(row, 1) << '\n';
        }

        Matrix<int> assigned(2, 2);
        assigned = a;
        assigned += identity;
        assigned -= identity;
        assigned *= identity;
        std::cout << "Assignment and compound operations preserve A here: " << (assigned == a) << '\n';

        a.setEchelon(0);
        std::cout << "Pivot tolerance: " << a.getEchelon() << '\n';
        std::cout << "Determinant: " << a.determinant() << '\n';
        std::cout << "Cached determinant: " << a.determinant() << '\n';
        std::cout << "Determinant calculations: " << a.getCacheInfo().determinantComputations << '\n';

        a.PLU();
        Matrix<int> p = a.getPLU_P();
        Matrix<int> l = a.getPLU_L();
        Matrix<int> u = a.getPLU_U();
        std::cout << "P, L, U with P * A = L * U\n";
        for (std::size_t row = 0; row < 2; ++row) {
            std::cout << p.getElement(row, 0) << ' ' << p.getElement(row, 1) << " | "
                      << l.getElement(row, 0) << ' ' << l.getElement(row, 1) << " | "
                      << u.getElement(row, 0) << ' ' << u.getElement(row, 1) << '\n';
        }

        Matrix<int> b(std::vector<std::vector<int>>{{4}, {4}});
        Matrix<int> x = a.solve(b);
        std::cout << "Solve A * x = [4, 4], expected x = [1, 2]\n";
        for (std::size_t row = 0; row < 2; ++row) {
            std::cout << x.getElement(row, 0) << '\n';
        }
        std::cout << "PLU calculations after solve: " << a.getCacheInfo().pluComputations << '\n';

        a.setElement(0, 0, 1);
        std::cout << "After updating an entry, determinant dirty: " << a.getCacheInfo().determinantDirty
                  << ", PLU dirty: " << a.getCacheInfo().pluDirty << '\n';
        std::cout << "Updated determinant: " << a.determinant() << '\n';

        try {
            a.getElement(2, 0);
        } catch (const std::out_of_range& error) {
            std::cout << "Invalid row: " << error.what() << '\n';
        }
    }
    {
        std::cout << "\nstd::complex<double>\n";
        Matrix<std::complex<double>> a;
        a.setNumRows(2);
        a.setNumCols(2);
        a.setElement(0, 0, 0);
        a.setElement(0, 1, 2);
        a.setElement(1, 0, 2);
        a.setElement(1, 1, 1);
        std::cout << "A is " << a.getNumRows() << " x " << a.getNumCols() << '\n';
        for (std::size_t row = 0; row < 2; ++row) {
            std::cout << a.getElement(row, 0) << ' ' << a.getElement(row, 1) << '\n';
        }

        Matrix<std::complex<double>> identity(std::vector<std::vector<std::complex<double>>>{{1, 0}, {0, 1}});
        Matrix<std::complex<double>> copy(a);
        std::cout << "Copy equals A: " << (copy == a) << '\n';
        copy.setElement(0, 0, 3);
        std::cout << "Changed copy differs from A: " << (copy != a) << '\n';

        Matrix<std::complex<double>> sum = a + identity;
        Matrix<std::complex<double>> difference = a - identity;
        Matrix<std::complex<double>> product = a * a;
        std::cout << "A + I, A - I, A * A\n";
        for (std::size_t row = 0; row < 2; ++row) {
            std::cout << sum.getElement(row, 0) << ' ' << sum.getElement(row, 1) << " | "
                      << difference.getElement(row, 0) << ' ' << difference.getElement(row, 1) << " | "
                      << product.getElement(row, 0) << ' ' << product.getElement(row, 1) << '\n';
        }

        Matrix<std::complex<double>> assigned(2, 2);
        assigned = a;
        assigned += identity;
        assigned -= identity;
        assigned *= identity;
        std::cout << "Assignment and compound operations preserve A here: " << (assigned == a) << '\n';

        a.setEchelon(0);
        std::cout << "Pivot tolerance: " << a.getEchelon() << '\n';
        std::cout << "Determinant: " << a.determinant() << '\n';
        std::cout << "Cached determinant: " << a.determinant() << '\n';
        std::cout << "Determinant calculations: " << a.getCacheInfo().determinantComputations << '\n';

        a.PLU();
        Matrix<std::complex<double>> p = a.getPLU_P();
        Matrix<std::complex<double>> l = a.getPLU_L();
        Matrix<std::complex<double>> u = a.getPLU_U();
        std::cout << "P, L, U with P * A = L * U\n";
        for (std::size_t row = 0; row < 2; ++row) {
            std::cout << p.getElement(row, 0) << ' ' << p.getElement(row, 1) << " | "
                      << l.getElement(row, 0) << ' ' << l.getElement(row, 1) << " | "
                      << u.getElement(row, 0) << ' ' << u.getElement(row, 1) << '\n';
        }

        Matrix<std::complex<double>> b(std::vector<std::vector<std::complex<double>>>{{4}, {4}});
        Matrix<std::complex<double>> x = a.solve(b);
        std::cout << "Solve A * x = [4, 4], expected x = [1, 2]\n";
        for (std::size_t row = 0; row < 2; ++row) {
            std::cout << x.getElement(row, 0) << '\n';
        }
        std::cout << "PLU calculations after solve: " << a.getCacheInfo().pluComputations << '\n';

        a.setElement(0, 0, 1);
        std::cout << "After updating an entry, determinant dirty: " << a.getCacheInfo().determinantDirty
                  << ", PLU dirty: " << a.getCacheInfo().pluDirty << '\n';
        std::cout << "Updated determinant: " << a.determinant() << '\n';

        try {
            a.getElement(2, 0);
        } catch (const std::out_of_range& error) {
            std::cout << "Invalid row: " << error.what() << '\n';
        }
    }
    return 0;
}
