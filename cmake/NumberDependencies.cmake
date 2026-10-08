include_guard(GLOBAL)
include(FetchContent)

set(MATRIXCLASS_ARBITRARY_INTEGER_REPOSITORY "https://github.com/AbstractClassroom-CPP/ArbitraryInteger.git"
    CACHE STRING "ArbitraryInteger repository URL or absolute local Git repository path")
set(MATRIXCLASS_RATIONAL_NUMBER_REPOSITORY "https://github.com/AbstractClassroom-CPP/RationalNumber.git"
    CACHE STRING "RationalNumber repository URL or absolute local Git repository path")

set(ARBITRARY_INTEGER_BUILD_EXAMPLE OFF)
set(RATIONAL_NUMBER_BUILD_EXAMPLE OFF)

FetchContent_Declare(arbitrary_integer
    GIT_REPOSITORY "${MATRIXCLASS_ARBITRARY_INTEGER_REPOSITORY}"
    GIT_TAG eaee528f09a32231ef9841dd2a2c90d2a6ece234
)
FetchContent_Declare(rational_number
    GIT_REPOSITORY "${MATRIXCLASS_RATIONAL_NUMBER_REPOSITORY}"
    GIT_TAG c9d841504443b92c476527170d5a1ba4b9ab16cb
)
FetchContent_MakeAvailable(arbitrary_integer rational_number)
