cmake_minimum_required(VERSION 3.19)

# Disable in-source builds to prevent source tree corruption.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "FATAL: In-source builds are not allowed.\nYou should create a separate directory for build files.")
endif()

project(scenegraph LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(AddCXXCompilerFlag)

# Turn compiler warnings up to 11
add_cxx_compiler_flag(-Wall)
add_cxx_compiler_flag(-Wextra)
add_cxx_compiler_flag(-Wshadow)
add_cxx_compiler_flag(-Wfloat-equal)
add_cxx_compiler_flag(-Wold-style-cast)
add_cxx_compiler_flag(-Wconversion)
add_cxx_compiler_flag(-Werror)
add_cxx_compiler_flag(-Wsuggest-override)
add_cxx_compiler_flag(-Wshorten-64-to-32)
add_cxx_compiler_flag(-Wno-deprecated-declarations)
add_cxx_compiler_flag(-Wno-deprecated)
add_cxx_compiler_flag(-Wstrict-aliasing)
add_cxx_compiler_flag(-Wthread-safety)
add_cxx_compiler_flag(-pedantic)
add_cxx_compiler_flag(-pedantic-errors)
add_cxx_compiler_flag(-fstrict-aliasing)

#add_cxx_compiler_flag(-fno-exceptions)
add_cxx_compiler_flag(-flto)
add_cxx_compiler_flag(-Wno-lto-type-mismatch)

# Export only public symbols
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo MinSizeRel)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)

# Set up directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Build the targets
add_subdirectory(src)
add_subdirectory(sgapp)
add_subdirectory(sgtest)
add_subdirectory(sgbenchmark)
