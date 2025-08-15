#include_directories(${PROJECT_SOURCE_DIR}/src)

include(FetchContent)

FetchContent_Declare(
  SDL3
  URL https://github.com/libsdl-org/SDL/releases/download/release-3.2.20/SDL3-3.2.20.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(SDL_TEST OFF)

FetchContent_MakeAvailable(SDL3)


file(GLOB_RECURSE
  SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/scenegraph/*.cpp
    ${PROJECT_SOURCE_DIR}/src/scenegraph/*.h
    ${PROJECT_SOURCE_DIR}/include/scenegraph/*.h
    ${PROJECT_SOURCE_DIR}/include/scenegraph/*.inl)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_library(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE
	SDL3::SDL3-static
)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)
