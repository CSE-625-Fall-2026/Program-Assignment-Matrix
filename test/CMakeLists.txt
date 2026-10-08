if(NOT TARGET GTest::gtest_main)
    find_package(GTest 1.17 CONFIG QUIET)
endif()
if(NOT TARGET GTest::gtest_main)
    include(FetchContent)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(googletest)
endif()

file(GLOB student_tests CONFIGURE_DEPENDS "student/*.cpp")
add_executable(matrixclass_tests
    instructor/ConstructionTest.cpp
    instructor/DimensionsTest.cpp
    instructor/ArithmeticTest.cpp
    instructor/DeterminantTest.cpp
    instructor/PLUTest.cpp
    instructor/SolveTest.cpp
    instructor/CacheTest.cpp
    instructor/GenericParallelTest.cpp
    instructor/NumberContractTest.cpp
    instructor/NumericalContractTest.cpp
    instructor/ComplexTest.cpp
    ${student_tests}
)
target_link_libraries(matrixclass_tests PRIVATE MatrixClass::MatrixClass GTest::gtest_main)
set_target_properties(matrixclass_tests PROPERTIES CXX_EXTENSIONS OFF)
add_test(NAME matrixclass.serial COMMAND matrixclass_tests)
add_test(NAME matrixclass.parallel COMMAND matrixclass_tests)
set_tests_properties(matrixclass.serial PROPERTIES
    ENVIRONMENT "OMP_NUM_THREADS=1;OMP_DYNAMIC=FALSE" TIMEOUT 30 LABELS "instructor;serial")
set_tests_properties(matrixclass.parallel PROPERTIES
    ENVIRONMENT "OMP_NUM_THREADS=4;OMP_DYNAMIC=FALSE" TIMEOUT 30 LABELS "instructor;parallel")

if(MATRIXCLASS_BUILD_NUMBER_EXTRAS)
    add_executable(matrixclass_number_tests instructor/NumberContractTest.cpp
        instructor/NumericalContractTest.cpp instructor/ExactNumberTest.cpp)
    target_compile_definitions(matrixclass_number_tests PRIVATE MATRIXCLASS_EXTERNAL_NUMBER_TESTS)
    target_link_libraries(matrixclass_number_tests PRIVATE MatrixClass::MatrixClass GTest::gtest_main
        ArbitraryInteger::ArbitraryInteger RationalNumber::RationalNumber)
    set_target_properties(matrixclass_number_tests PROPERTIES CXX_EXTENSIONS OFF)
    add_test(NAME matrixclass.numbers.serial COMMAND matrixclass_number_tests)
    add_test(NAME matrixclass.numbers.parallel COMMAND matrixclass_number_tests)
    set_tests_properties(matrixclass.numbers.serial PROPERTIES
        ENVIRONMENT "OMP_NUM_THREADS=1;OMP_DYNAMIC=FALSE" TIMEOUT 30 LABELS "instructor;numbers;serial")
    set_tests_properties(matrixclass.numbers.parallel PROPERTIES
        ENVIRONMENT "OMP_NUM_THREADS=4;OMP_DYNAMIC=FALSE" TIMEOUT 30 LABELS "instructor;numbers;parallel")
endif()

if(MATRIXCLASS_BUILD_EXAMPLES)
    add_test(NAME matrixclass.example.double COMMAND matrixclass_double_example)
    add_test(NAME matrixclass.example.basic COMMAND matrixclass_basic_example)
    set_tests_properties(matrixclass.example.double matrixclass.example.basic PROPERTIES
        ENVIRONMENT "OMP_NUM_THREADS=4;OMP_DYNAMIC=FALSE" TIMEOUT 10 LABELS example)
    if(MATRIXCLASS_BUILD_NUMBER_EXTRAS)
        add_test(NAME matrixclass.example.exact COMMAND matrixclass_exact_example)
        set_tests_properties(matrixclass.example.exact PROPERTIES
            ENVIRONMENT "OMP_NUM_THREADS=4;OMP_DYNAMIC=FALSE" TIMEOUT 10 LABELS "example;numbers")
    endif()
endif()
