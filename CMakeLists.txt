cmake_minimum_required(VERSION 3.19)

project(scenegraph LANGUAGES CXX)

# Disable in-source builds to prevent source tree corruption.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "FATAL: In-source builds are not allowed.\nYou should create a separate directory for build files.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo MinSizeRel)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)

file(GLOB
  SOURCE_FILES
    *.cpp
    ${PROJECT_SOURCE_DIR}/include/scenegraph/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

# Set up directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Build the targets
add_subdirectory(src)
add_subdirectory(sgapp)
add_subdirectory(sgtest)
add_subdirectory(sgbenchmark)
