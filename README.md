# CSE 625 MatrixClass: Parallel Matrix Calculations

## What this assignment evaluates

* C++ templates, operator overloading, and independent object ownership
* OpenMP loops, data dependencies, and exception handling
* Matrix arithmetic, determinants, partial pivoting, and linear systems
* Lazy calculation, cache invalidation, and numerical tolerance

## Project manager statement

* Build a reusable `Matrix<T>` library that performs matrix calculations in parallel and reuses calculated results until its entries change.
* Complete the marked method bodies and add the missing operator implementations using this specification and the instructor tests. The starter includes the complete header, default initialization, and OpenMP exception handling. The initial build will fail until the missing operator definitions are supplied.

## Required final file layout

* `CMakeLists.txt`: Configure the library, examples, and GoogleTest targets.
* `include/Matrix.hpp`: Complete declarations for `Matrix<T>`. **Do not modify any part of this file.** Implement the declared methods in `include/Matrix.tpp` and `src/Matrix.cpp`.
* `include/Matrix.tpp`: Define the generic implementation.
* `include/MatrixStrings.hpp`: Define library exception messages as `inline constexpr char` arrays in `MatrixStrings`.
* `src/Matrix.cpp`: Define compiled specializations and explicit instantiations.
* `example/double.cpp`, `example/basic_types.cpp`, and `example/exact_types.cpp`: Demonstrate the public interface for the supported numeric types.
* `test/instructor/`: Supplied tests. **Do not modify these tests or disable their execution.**
* `test/student/`: Add your own GoogleTest cases here.
* Keep the supplied CMake support under `cmake/` and grading configuration under `.github/`.

## Class components

### Types and storage

* Use C++17. Define public aliases `value_type` for `T` and `size_type` for `std::size_t`.
* Support `double`, `int`, `std::complex<double>`, `ArbitraryInteger`, and `RationalNumber<ArbitraryInteger>`. Keep the template usable with other types providing the required operations.
* Treat `T{}` as zero and `T{1}` as one. Basic arithmetic requires copying, equality, addition, subtraction, and multiplication. PLU and solve also require division and magnitude comparisons. Basic operations must not require scalar compound assignment, ordering, or conversion to `double`.
* Store entries in one owned, contiguous `T[]` array, indexed by `row * numCols + col`. Use `std::unique_ptr<T[]>` for ownership. Vectors are accepted as constructor input but must not remain as matrix storage.
* Keep dimensions private and fixed once assigned. Initialize fields in constructor bodies. Copies own independent arrays and preserve the source tolerance. Construction from an rvalue must leave the source usable.
* Perform arithmetic using `T`. Do not reject integer division or promote entries to another type. Truncation can produce incorrect PLU factors, solutions, and singular classifications. Builtin integer intermediates must remain representable.

### Construction and element access

* `Matrix()`: Leave both dimensions unset, with counts of zero and no allocated entries.
* `Matrix(size_type numRows, size_type numCols)`: Set both dimensions and initialize entries to `T{}`. Accept two positive counts or `0, 0`. Reject exactly one zero with `std::invalid_argument`.
* `explicit Matrix(const std::vector<std::vector<T>>& values)`: Copy rectangular input. An empty outer vector creates an empty matrix. Reject unequal row lengths and nonempty input with empty rows using `std::invalid_argument`.
* `Matrix(const Matrix& other)`: Copy entries, dimensions, and which dimensions are set. Start with dirty caches and zero calculation counters.
* `size_type getNumRows() const noexcept` and `size_type getNumCols() const noexcept`: Return the corresponding count, or zero when unset.
* `void setNumRows(size_type numRows)` and `void setNumCols(size_type numCols)`: Set each positive dimension exactly once, in either order. Allocate and initialize entries only when both are set.
* A repeated dimension setter throws `std::logic_error`, even for the same count. An unset dimension rejects zero with `std::invalid_argument`. Failed validation or allocation must leave that dimension unset and available for another attempt.
* Reject dimensions exceeding array size or loop index capacity with `std::length_error`. Allocation failure may throw `std::bad_alloc`.
* `T getElement(size_type row, size_type col) const`: Return a copy of the selected entry.
* `void setElement(size_type row, size_type col, const T& value)`: Replace an entry and invalidate calculations, even when the new value equals the old value.
* Indices start at zero. Invalid row or column indices throw `std::out_of_range`, including access to an empty matrix.
* With exactly one dimension set, element access, arithmetic, assignment, comparisons, determinant, PLU, and solve throw `std::logic_error`. Copying, dimension getters, setting the remaining dimension, and tolerance or cache inspection remain available.

### Operators

* `Matrix operator+(const Matrix& other) const`: Add corresponding entries of matrices with equal dimensions. Binary subtraction has the same signature and subtracts corresponding entries.
* `Matrix operator*(const Matrix& other) const`: Compute matrix multiplication. An `m × n` matrix multiplied by an `n × p` matrix produces an `m × p` matrix. Do not conjugate complex entries.
* `Matrix& operator=(const Matrix& other)`: Copy entries into an equal shape. Preserve the destination dimensions and tolerance. Assignment cannot configure an empty matrix.
* `Matrix& operator+=(const Matrix& other)`: Replace entries with their sum. Compound subtraction has the same parameter and return types and replaces entries with their difference.
* `Matrix& operator*=(const Matrix& other)`: Replace entries with the product only when the result preserves the destination shape.
* All assignment operators return `*this`. Handle self assignment and self compound operations correctly.
* `bool operator==(const Matrix& other) const`: Compare shapes and corresponding entries exactly. Tolerance does not affect equality. A matrix containing `NaN` does not equal itself.
* `bool operator!=(const Matrix& other) const`: Return the negation of equality.
* Incompatible arithmetic or assignment dimensions throw `std::invalid_argument`. Equality returns false for unequal shapes.
* Empty matrices support arithmetic, assignment, and equality with other empty matrices. New arithmetic results inherit the left operand's tolerance and start with uncomputed caches.
* Failed arithmetic or assignment must preserve operands and existing caches. If a custom element assignment throws during `setElement`, leave the caches dirty because the entry may have changed.

### Determinant

* `T determinant() const`: Calculate and cache the determinant. Rectangular matrices throw `std::invalid_argument`.
* The empty determinant is `T{1}`. For one entry, return that entry. For a two by two matrix, the determinant is the main diagonal product minus the other diagonal product.
* For `double`, obtain the determinant from the PLU row permutation sign and the diagonal product of `U`. Reuse cached factors. Return zero when PLU classifies the matrix as singular at the current tolerance.
* For other element types, use cofactor expansion without division or PLU. These determinants are independent of pivot tolerance.

### PLU and solve

* `void PLU() const`: Calculate and cache partial pivoting factors with the convention `P * A = L * U`. Store `P`, `L`, and `U` as separate matrices.
* `Matrix getPLU_P() const`, `Matrix getPLU_L() const`, and `Matrix getPLU_U() const`: Request PLU when needed and return independent copies of the corresponding factors.
* Select the largest remaining pivot magnitude in each column. Use complex modulus for complex entries and absolute value for ordered types. Resolve equal magnitudes using the first available row.
* Swap rows in `P` and `U`, along with previously calculated columns of `L`. Give `L` a unit diagonal and eliminate entries below each pivot in `U`.
* Use the same elimination and substitution algorithms for every element type. Rational arithmetic must remain exact without conversion to floating point.
* `Matrix solve(const Matrix& b) const`: Solve `A * x = b` using the current matrix as `A`. **A must be nonempty, square, and nonsingular.** Nonzero entries alone do not establish nonsingularity.
* For `A` of size `n × n`, accept `b` of size `n × k`, with `k >= 1`, and return `x` of size `n × k`. Multiple columns represent independent right hand sides.
* Use forward substitution for `L * y = P * b`, then back substitution for `U * x = y`. Preserve both inputs and give the result the tolerance of `A`.
* Reuse PLU across calls. Calculate each solution from the current `b`. Solving must not calculate the determinant.
* Empty or rectangular `A` and incompatible right hand sides throw `std::invalid_argument`. A zero pivot at the configured tolerance causes PLU, its getters, and solve to throw `std::domain_error`.
* Cache singular classifications as completed PLU calculations. Do not return incomplete factors.

### Tolerance and numerical errors

* `void setEchelon(double value)`: Set the relative pivot tolerance. Require a finite value in `[0, 1)`, otherwise throw `std::invalid_argument` without changing the matrix.
* `double getEchelon() const noexcept`: Return the configured tolerance. Defaults are `0.000000000001` for `double` and `std::complex<double>`, and zero for other types.
* Let `scale` be the largest magnitude in the original matrix. Treat a pivot as zero when it is exactly zero or its magnitude divided by `scale` is at most the tolerance. With zero tolerance, reject only an exactly zero pivot.
* For rational entries, represent the tolerance as the exact binary fraction of the supplied `double`. For integer entries, fractional tolerance truncates to zero.
* For `double` and `std::complex<double>`, PLU and solve reject nonfinite inputs with `std::invalid_argument` and nonfinite results with `std::overflow_error`. These checks also apply to the double determinant. Check both complex components and reject complex pivot magnitudes beyond the finite double range.
* Return unrounded results. Floating point rounding and underflow follow the element type's arithmetic. Tests compare approximate results with tolerances and exact results directly.

### Caches and OpenMP

* Provide public `CacheInfo getCacheInfo() const noexcept`. The public nested `CacheInfo` contains `bool determinantDirty`, `bool pluDirty`, `size_type determinantComputations`, and `size_type pluComputations`.
* Start with both dirty flags true and both counters zero. Construction, copying, element access, arithmetic, equality, and cache inspection must not calculate determinant or PLU.
* Calculate only on request. Determinant for `double` and solve may request PLU as a dependency. Repeated requests reuse valid results.
* Successful dimension setters, entry writes, assignment, compound assignment, and tolerance changes invalidate both caches. Self assignment and setting the existing tolerance preserve them.
* Count completed calculations over each object's lifetime, including singular PLU classifications. Invalidation does not reset counters. Copies start with zero counters. Failed calculations must not publish partial results.
* Parallelize independent output entries in addition, subtraction, and multiplication. Accumulate each dot product in a fixed serial order so results agree across thread counts.
* Parallelize independent elimination rows within each PLU step and independent right hand side columns in solve. Finish a pivot step before beginning the next.
* Use ordinary loops with OpenMP directives and static scheduling. Use multiple workers for at least 256 arithmetic outputs, 16 remaining elimination rows, or four right hand sides. Smaller operations may execute serially.
* Avoid nested teams when called within an OpenMP region. Capture exceptions inside worker iterations and rethrow after the team finishes.
* Independent matrices must work concurrently. Calls that modify entries or caches on the same matrix require caller synchronization.

### Building and testing

* Use CMake 3.24 or newer, a C++17 compiler with OpenMP support, and GoogleTest 1.17 or newer. OpenMP is required.
* On WSL with Ubuntu 24.04 or newer, install the build tools in the Ubuntu terminal:

```bash
sudo apt update
sudo apt install g++ make cmake git
```

* On macOS, install Apple's Command Line Tools and Homebrew, then install the build tools and OpenMP runtime:

```bash
brew install cmake git libomp
```

* On either platform, run these commands from the repository root to configure, build, and run the tests:

```bash
mkdir build
cd build
cmake ..
make
ctest
```

* After editing your code, run `make` and `ctest` again from `build`.
* If `build` already exists, skip `mkdir build`. To retry a failed configuration, run `cmake ..` from `build` before building again.
* CMake locates Homebrew's OpenMP installation automatically on macOS. A custom OpenMP installation can be selected with the `OpenMP_ROOT` environment variable.
* Link applications to `MatrixClass::MatrixClass`. The target must propagate its include directory, C++17 requirement, and OpenMP dependency, and work when fetched with CMake `FetchContent`.
* Enable `MATRIXCLASS_BUILD_TESTS`, `MATRIXCLASS_BUILD_EXAMPLES`, and `MATRIXCLASS_BUILD_NUMBER_EXTRAS` by default for a standalone build. Default them to off when another project fetches the library.
* Examples and tests use [ArbitraryInteger v4.0.0](https://github.com/AbstractClassroom%2DCPP/ArbitraryInteger/tree/v4.0.0) and [RationalNumber v5.0.0](https://github.com/AbstractClassroom%2DCPP/RationalNumber/tree/v5.0.0). These dependencies must not become requirements for consumers of the Matrix library.
* CMake finds or fetches GoogleTest and fetches the pinned number libraries. Initial downloads need network access. Local number repositories can be selected with `MATRIXCLASS_ARBITRARY_INTEGER_REPOSITORY` and `MATRIXCLASS_RATIONAL_NUMBER_REPOSITORY`.
* The three examples give the same short overview for all five types. CTest runs all examples and runs student and instructor tests using one and four OpenMP threads.

## How grading works

* Use failing tests to identify missing behavior. Keep the required API, instructor tests, and grading configuration intact. Grading restores protected headers, instructor tests, examples, and build files from the instructor source before running your submission.
* Build succeeds and every test passes: **100 points**.
* Build fails, any test fails, or no tests execute: **10 points**.
* There are no intermediate scores. Tests check correctness, validation, cache reuse, and actual OpenMP worker participation. They do not require a particular speedup.
* After pushing to the published student repository, check **Score** in the grading run summary. A green workflow alone does not establish a score of 100.

## How to submit your grade

* Grade Token submission becomes available with the published student assignment.
* Choose the grading run to submit and use **Grade Token** to copy the complete string beginning with `ACGT1_`.
* Paste only that token into Blackboard's text field.
* Check the token before submitting. **Only one submission is allowed.**
