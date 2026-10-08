#include <Matrix.hpp>
#include <MatrixStrings.hpp>

template<>
double Matrix<double>::defaultEchelon() {
    return 1e-12;
}

template<>
void Matrix<double>::requireFinite(const double& value) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<>
double Matrix<double>::finite(const double& value) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<>
bool Matrix<double>::greaterMagnitude(const double& left, const double& right) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<>
double Matrix<double>::divisionTolerance() const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<>
bool Matrix<double>::negligiblePivot(const double& pivot, const double& scale, const double& tolerance) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<>
double Matrix<std::complex<double>>::defaultEchelon() {
    return 1e-12;
}

template<>
void Matrix<std::complex<double>>::requireFinite(const std::complex<double>& value) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<>
std::complex<double> Matrix<std::complex<double>>::finite(const std::complex<double>& value) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<>
bool Matrix<std::complex<double>>::greaterMagnitude(const std::complex<double>& left, const std::complex<double>& right) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<>
std::complex<double> Matrix<std::complex<double>>::divisionTolerance() const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<>
bool Matrix<std::complex<double>>::negligiblePivot(const std::complex<double>& pivot,
        const std::complex<double>& scale, const std::complex<double>& tolerance) {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template<>
double Matrix<double>::determinant() const {
    //something more should be here
    throw std::logic_error(MatrixStrings::NotImplemented);
}

template class Matrix<int>;
template class Matrix<double>;
