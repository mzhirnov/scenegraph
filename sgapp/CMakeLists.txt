cmake_minimum_required(VERSION 3.19)

project(sgapp LANGUAGES CXX)

add_executable(${PROJECT_NAME}
  main.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} PRIVATE
	scenegraph
)
