cmake_minimum_required(VERSION 3.7)

project(BKAL)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE "Debug")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wextra -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wextra -Wall")


include_directories(
    "include"
    "libraries/Fake.BKAL/include"
)

set(CTEST_ENVIRONMENT CTEST_OUTPUT_ON_FAILURE=1)
enable_testing()

add_subdirectory(libraries)
