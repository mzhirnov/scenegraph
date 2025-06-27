include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB_RECURSE
  SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/scenegraph/*.cpp
    ${PROJECT_SOURCE_DIR}/include/scenegraph/*.h)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_library(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)
