cmake_minimum_required(VERSION 3.19)

project(sgbenchmark LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
  GBenchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.9.3.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)

set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_INSTALL OFF)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
#set(BENCHMARK_ENABLE_LIBPFM ON)

FetchContent_MakeAvailable(GBenchmark)

add_executable(${PROJECT_NAME}
  main.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${PROJECT_NAME} PRIVATE
	scenegraph
	benchmark::benchmark
	benchmark::benchmark_main
)
