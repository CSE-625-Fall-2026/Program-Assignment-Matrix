#ifndef MATRIXCLASS_MATRIX_HPP
#define MATRIXCLASS_MATRIX_HPP

// This header is complete. Do not modify any part of this file.

#include <cstddef>
#include <complex>
#include <memory>
#include <vector>

template<typename T>
class Matrix {
public:
    using value_type = T;
    using size_type = std::size_t;

    struct CacheInfo {
        bool determinantDirty;
        bool pluDirty;
        size_type determinantComputations;
        size_type pluComputations;
    };

    Matrix();
    Matrix(size_type numRows, size_type numCols);
    explicit Matrix(const std::vector<std::vector<T>>& values);
    Matrix(const Matrix& other);
    Matrix& operator=(const Matrix& other);

    size_type getNumRows() const noexcept;
    size_type getNumCols() const noexcept;
    void setNumRows(size_type numRows);
    void setNumCols(size_type numCols);
    T getElement(size_type row, size_type col) const;
    void setElement(size_type row, size_type col, const T& value);

    Matrix operator+(const Matrix& other) const;
    Matrix operator-(const Matrix& other) const;
    Matrix operator*(const Matrix& other) const;
    Matrix& operator+=(const Matrix& other);
    Matrix& operator-=(const Matrix& other);
    Matrix& operator*=(const Matrix& other);
    bool operator==(const Matrix& other) const;
    bool operator!=(const Matrix& other) const;

    T determinant() const;
    void PLU() const;
    Matrix getPLU_P() const;
    Matrix getPLU_L() const;
    Matrix getPLU_U() const;
    Matrix solve(const Matrix& b) const;
    void setEchelon(double value);
    double getEchelon() const noexcept;
    CacheInfo getCacheInfo() const noexcept;

private:
    size_type numRows_;
    size_type numCols_;
    bool numRowsSet_;
    bool numColsSet_;
    std::unique_ptr<T[]> elements_;
    double echelon_;
    mutable T determinant_;
    mutable std::unique_ptr<Matrix> p_;
    mutable std::unique_ptr<Matrix> l_;
    mutable std::unique_ptr<Matrix> u_;
    mutable bool determinantDirty_;
    mutable bool pluDirty_;
    mutable bool singular_;
    mutable int permutationSign_;
    mutable size_type determinantComputations_;
    mutable size_type pluComputations_;

    static double defaultEchelon();
    static std::unique_ptr<T[]> allocate(size_type rows, size_type cols);
    void requireReady() const;
    void requireIndex(size_type row, size_type col) const;
    void requireSameShape(const Matrix& other) const;
    void requireSquare(bool nonempty) const;
    void markDirty() noexcept;
    T cofactor() const;
    bool calculatePLU() const;
    static void requireFinite(const T& value);
    static T finite(const T& value);
    static bool greaterMagnitude(const T& left, const T& right);
    T divisionTolerance() const;
    static bool negligiblePivot(const T& pivot, const T& scale, const T& tolerance);
};

template<> double Matrix<double>::defaultEchelon();
template<> double Matrix<double>::determinant() const;
template<> void Matrix<double>::requireFinite(const double& value);
template<> double Matrix<double>::finite(const double& value);
template<> bool Matrix<double>::greaterMagnitude(const double& left, const double& right);
template<> double Matrix<double>::divisionTolerance() const;
template<> bool Matrix<double>::negligiblePivot(const double& pivot, const double& scale, const double& tolerance);

template<> double Matrix<std::complex<double>>::defaultEchelon();
template<> void Matrix<std::complex<double>>::requireFinite(const std::complex<double>& value);
template<> std::complex<double> Matrix<std::complex<double>>::finite(const std::complex<double>& value);
template<> bool Matrix<std::complex<double>>::greaterMagnitude(const std::complex<double>& left, const std::complex<double>& right);
template<> std::complex<double> Matrix<std::complex<double>>::divisionTolerance() const;
template<> bool Matrix<std::complex<double>>::negligiblePivot(const std::complex<double>& pivot,
    const std::complex<double>& scale, const std::complex<double>& tolerance);

#include <Matrix.tpp>

extern template class Matrix<int>;
extern template class Matrix<double>;

#endif
