cmake_minimum_required(VERSION 3.19)

project(sgapp LANGUAGES CXX)

add_executable(${PROJECT_NAME}
  main.cpp
)

include(FetchContent)

FetchContent_Declare(
  SDL3
  URL https://github.com/libsdl-org/SDL/releases/download/release-3.2.18/SDL3-3.2.18.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(SDL_TEST OFF)

FetchContent_MakeAvailable(SDL3)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} PRIVATE
	scenegraph
	SDL3::SDL3-static
)
