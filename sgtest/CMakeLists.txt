cmake_minimum_required(VERSION 3.19)

project(sgtest LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
  GTest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)

set(INSTALL_GTEST OFF)

FetchContent_MakeAvailable(GTest)

enable_testing()

include(GoogleTest)

add_executable(${PROJECT_NAME}
  main.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${PROJECT_NAME} PRIVATE
	scenegraph
	GTest::gtest_main
)

gtest_add_tests(TARGET ${PROJECT_NAME})
