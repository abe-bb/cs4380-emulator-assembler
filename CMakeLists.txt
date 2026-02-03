cmake_minimum_required(VERSION 3.14)
project(cs4380_emulator)

# GoogleTest requirement
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/7d7e750850c65099e49cc1a1aac94a79a914bba7.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  runTests
  test/tests.cpp include/emu4380.h src/emu4380.cpp 
)
target_link_libraries(
  runTests
  GTest::gtest_main
)

add_executable(
  emu4380
  src/emu4380.cpp src/main.cpp
)
