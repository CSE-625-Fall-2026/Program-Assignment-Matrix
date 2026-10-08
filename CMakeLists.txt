cmake_minimum_required(VERSION 3.24)
project(MatrixClass VERSION 1.0.0 LANGUAGES CXX)

option(MATRIXCLASS_BUILD_EXAMPLES "Build MatrixClass examples" ${PROJECT_IS_TOP_LEVEL})
option(MATRIXCLASS_BUILD_TESTS "Build MatrixClass tests" ${PROJECT_IS_TOP_LEVEL})
option(MATRIXCLASS_BUILD_NUMBER_EXTRAS "Build examples and tests using ArbitraryInteger and RationalNumber" ${PROJECT_IS_TOP_LEVEL})

if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang)$"
    AND NOT DEFINED OpenMP_ROOT AND NOT DEFINED ENV{OpenMP_ROOT})
    find_program(MATRIXCLASS_BREW_EXECUTABLE brew HINTS /opt/homebrew/bin /usr/local/bin)
    if(MATRIXCLASS_BREW_EXECUTABLE)
        execute_process(
            COMMAND "${MATRIXCLASS_BREW_EXECUTABLE}" --prefix libomp
            OUTPUT_VARIABLE MATRIXCLASS_LIBOMP_ROOT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE MATRIXCLASS_LIBOMP_RESULT
            ERROR_QUIET
            TIMEOUT 10
        )
        if(MATRIXCLASS_LIBOMP_RESULT EQUAL 0)
            set(OpenMP_ROOT "${MATRIXCLASS_LIBOMP_ROOT}")
        endif()
    endif()
endif()

find_package(OpenMP REQUIRED COMPONENTS CXX)
add_subdirectory(src)

if(MATRIXCLASS_BUILD_NUMBER_EXTRAS AND (MATRIXCLASS_BUILD_EXAMPLES OR MATRIXCLASS_BUILD_TESTS))
    include(cmake/NumberDependencies.cmake)
endif()

if(MATRIXCLASS_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(MATRIXCLASS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
