#ifndef MATRIXCLASS_MATRIX_STRINGS_HPP
#define MATRIXCLASS_MATRIX_STRINGS_HPP

namespace MatrixStrings {
inline constexpr char NotImplemented[] = "This operation is not implemented yet.";
inline constexpr char InvalidDimensions[] = "Both dimensions must be positive or both zero.";
inline constexpr char StorageSizeExceeded[] = "Matrix dimensions exceed the supported array size.";
inline constexpr char InvalidInputRows[] = "Input rows must have equal positive lengths.";
inline constexpr char RowsAlreadySet[] = "The number of rows has already been set.";
inline constexpr char RowsMustBePositive[] = "The number of rows must be positive.";
inline constexpr char ColumnsAlreadySet[] = "The number of columns has already been set.";
inline constexpr char ColumnsMustBePositive[] = "The number of columns must be positive.";
inline constexpr char DimensionsIncomplete[] = "Set both dimensions before using matrix entries or calculations.";
inline constexpr char IndexOutOfBounds[] = "Matrix indices are zero based and must be within bounds.";
inline constexpr char ShapeMismatch[] = "Matrix dimensions must match and cannot change.";
inline constexpr char NonemptySquareRequired[] = "This calculation requires a nonempty square matrix.";
inline constexpr char ProductShapeMismatch[] = "Left columns must equal right rows.";
inline constexpr char CompoundProductShapeMismatch[] = "Compound multiplication must preserve dimensions.";
inline constexpr char InvalidEchelon[] = "Echelon must be finite and in [0, 1).";
inline constexpr char SingularMatrix[] = "Matrix is singular at the configured echelon tolerance.";
inline constexpr char InvalidRightHandSide[] = "The right-hand side must have matching rows and at least one column.";
inline constexpr char FiniteEntriesRequired[] = "Numerical methods require finite entries.";
inline constexpr char NonfiniteResult[] = "A numerical calculation produced a nonfinite value.";
inline constexpr char PivotMagnitudeOverflow[] = "A pivot magnitude exceeds the supported range.";

}

#endif
