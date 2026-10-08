add_executable(matrixclass_double_example double.cpp)
target_link_libraries(matrixclass_double_example PRIVATE MatrixClass::MatrixClass)

add_executable(matrixclass_basic_example basic_types.cpp)
target_link_libraries(matrixclass_basic_example PRIVATE MatrixClass::MatrixClass)

if(MATRIXCLASS_BUILD_NUMBER_EXTRAS)
    add_executable(matrixclass_exact_example exact_types.cpp)
    target_link_libraries(matrixclass_exact_example PRIVATE MatrixClass::MatrixClass
        ArbitraryInteger::ArbitraryInteger RationalNumber::RationalNumber)
endif()
