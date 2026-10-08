#ifndef MATRIXCLASS_MATRIX_TPP
#define MATRIXCLASS_MATRIX_TPP

#include <algorithm>
#include <MatrixStrings.hpp>
#include <cmath>
#include <exception>
#include <limits>
#include <stdexcept>
#include <utility>
#include <omp.h>

template<typename T>
double Matrix<T>::defaultEchelon() {
    return 0.0;
}

template<typename T>
std::unique_ptr<T[]> Matrix<T>::allocate(size_type rows, size_type cols) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
Matrix<T>::Matrix() {
    numRows_ = 0;
    numCols_ = 0;
    numRowsSet_ = false;
    numColsSet_ = false;
    elements_ = nullptr;
    echelon_ = defaultEchelon();
    determinant_ = T{};
    p_ = nullptr;
    l_ = nullptr;
    u_ = nullptr;
    determinantDirty_ = true;
    pluDirty_ = true;
    singular_ = false;
    permutationSign_ = 1;
    determinantComputations_ = 0;
    pluComputations_ = 0;
}

template<typename T>
Matrix<T>::Matrix(size_type numRows, size_type numCols) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
Matrix<T>::Matrix(const std::vector<std::vector<T>>& values) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
Matrix<T>::Matrix(const Matrix& other) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
typename Matrix<T>::size_type Matrix<T>::getNumRows() const noexcept { return numRows_; }

template<typename T>
typename Matrix<T>::size_type Matrix<T>::getNumCols() const noexcept { return numCols_; }

template<typename T>
void Matrix<T>::setNumRows(size_type numRows) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
void Matrix<T>::setNumCols(size_type numCols) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
void Matrix<T>::requireReady() const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
void Matrix<T>::requireIndex(size_type row, size_type col) const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
void Matrix<T>::requireSameShape(const Matrix& other) const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
void Matrix<T>::requireSquare(bool nonempty) const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
T Matrix<T>::getElement(size_type row, size_type col) const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
void Matrix<T>::setElement(size_type row, size_type col, const T& value) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
void Matrix<T>::markDirty() noexcept {
    //something more should be here
}

template<typename T>
Matrix<T> Matrix<T>::operator+(const Matrix& other) const {
    //something more should be here
    const size_type count = numRows_ * numCols_;
    std::exception_ptr failure;
    #pragma omp parallel for schedule(static) if(count >= 256 && !omp_in_parallel())
    for (size_type index = 0; index < count; ++index) {
        try {
            //something more should be here
        } catch (...) {
            #pragma omp critical(matrixclass_exception)
            {
                if (!failure) {
                    failure = std::current_exception();
                }
            }
        }
    }
    if (failure) {
        std::rethrow_exception(failure);
    }
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
Matrix<T> Matrix<T>::operator-(const Matrix& other) const {
    //something more should be here
    const size_type count = numRows_ * numCols_;
    std::exception_ptr failure;
    #pragma omp parallel for schedule(static) if(count >= 256 && !omp_in_parallel())
    for (size_type index = 0; index < count; ++index) {
        try {
            //something more should be here
        } catch (...) {
            #pragma omp critical(matrixclass_exception)
            {
                if (!failure) {
                    failure = std::current_exception();
                }
            }
        }
    }
    if (failure) {
        std::rethrow_exception(failure);
    }
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
Matrix<T> Matrix<T>::operator*(const Matrix& other) const {
    //something more should be here
    const size_type count = numRows_ * other.numCols_;
    std::exception_ptr failure;
    #pragma omp parallel for schedule(static) if(count >= 256 && !omp_in_parallel())
    for (size_type index = 0; index < count; ++index) {
        try {
            //something more should be here
        } catch (...) {
            #pragma omp critical(matrixclass_exception)
            {
                if (!failure) {
                    failure = std::current_exception();
                }
            }
        }
    }
    if (failure) {
        std::rethrow_exception(failure);
    }
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

//something more should be here

template<typename T>
T Matrix<T>::cofactor() const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
T Matrix<T>::determinant() const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
void Matrix<T>::setEchelon(double value) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
double Matrix<T>::getEchelon() const noexcept { return echelon_; }

template<typename T>
typename Matrix<T>::CacheInfo Matrix<T>::getCacheInfo() const noexcept {
    return {determinantDirty_, pluDirty_, determinantComputations_, pluComputations_};
}

template<typename T>
void Matrix<T>::requireFinite(const T&) {}

template<typename T>
T Matrix<T>::finite(const T& value) {
    return value;
}

template<typename T>
bool Matrix<T>::greaterMagnitude(const T& left, const T& right) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
T Matrix<T>::divisionTolerance() const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
bool Matrix<T>::negligiblePivot(const T& pivot, const T& scale, const T& tolerance) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
bool Matrix<T>::calculatePLU() const {
    //something more should be here
    for (size_type col = 0; col < numCols_; ++col) {
        //something more should be here
        const size_type remaining = numRows_ - col - 1;
        std::exception_ptr failure;
        #pragma omp parallel for schedule(static) if(remaining >= 16 && !omp_in_parallel())
        for (size_type row = col + 1; row < numRows_; ++row) {
            try {
                //something more should be here
            } catch (...) {
                #pragma omp critical(matrixclass_exception)
                {
                    if (!failure) {
                        failure = std::current_exception();
                    }
                }
            }
        }
        if (failure) {
            std::rethrow_exception(failure);
        }
    }
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
void Matrix<T>::PLU() const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
Matrix<T> Matrix<T>::getPLU_P() const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
Matrix<T> Matrix<T>::getPLU_L() const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
Matrix<T> Matrix<T>::getPLU_U() const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<typename T>
Matrix<T> Matrix<T>::solve(const Matrix& b) const {
    //something more should be here
    std::exception_ptr failure;
    #pragma omp parallel for schedule(static) if(b.numCols_ >= 4 && !omp_in_parallel())
    for (size_type col = 0; col < b.numCols_; ++col) {
        try {
            //something more should be here
        } catch (...) {
            #pragma omp critical(matrixclass_exception)
            {
                if (!failure) {
                    failure = std::current_exception();
                }
            }
        }
    }
    if (failure) {
        std::rethrow_exception(failure);
    }
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

#endif
