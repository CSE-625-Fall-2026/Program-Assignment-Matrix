cmake_minimum_required(VERSION 3.24)
project(MatrixClassConsumer LANGUAGES CXX)

include(FetchContent)
set(MATRIXCLASS_REPOSITORY "" CACHE STRING "MatrixClass repository URL or absolute local Git repository path")
set(MATRIXCLASS_REVISION "solution" CACHE STRING "MatrixClass commit, branch, or tag")
if(NOT MATRIXCLASS_REPOSITORY)
    message(FATAL_ERROR "Set MATRIXCLASS_REPOSITORY to the MatrixClass repository URL or local Git path.")
endif()
FetchContent_Declare(MatrixClass
    GIT_REPOSITORY "${MATRIXCLASS_REPOSITORY}"
    GIT_TAG "${MATRIXCLASS_REVISION}"
)
FetchContent_MakeAvailable(MatrixClass)

if(TARGET matrixclass_tests OR TARGET matrixclass_double_example OR TARGET matrixclass_basic_example
    OR TARGET matrixclass_exact_example OR TARGET matrixclass_number_tests
    OR TARGET ArbitraryInteger::ArbitraryInteger OR TARGET RationalNumber::RationalNumber)
    message(FATAL_ERROR "A fetched MatrixClass must not enable its tests or examples by default.")
endif()
add_executable(matrixclass_consumer ../double.cpp)
target_link_libraries(matrixclass_consumer PRIVATE MatrixClass::MatrixClass)
