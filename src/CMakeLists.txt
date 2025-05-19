include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB
  SOURCE_FILES
    *.cpp
    ${PROJECT_SOURCE_DIR}/include/scenegraph/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)
