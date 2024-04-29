cmake_minimum_required(VERSION 3.22)

project(Coursework_final)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(extractor)
add_executable(errors)
add_executable(tree)

target_include_directories(extractor PRIVATE "${PROJECT_SOURCE_DIR}")
target_include_directories(errors PRIVATE "${PROJECT_SOURCE_DIR}")
target_include_directories(tree PRIVATE "${PROJECT_SOURCE_DIR}")

add_subdirectory(extractor)
add_subdirectory(frontend/error_detection)
add_subdirectory(frontend/ast)

add_library(tree-lib tree-sitter/lib/src/lib.c tree-sitter-c/src/parser.c)
include_directories(
        tree-sitter/lib/src
        tree-sitter/lib/include
        )
target_link_libraries(extractor PRIVATE tree-lib)
target_link_libraries(tree PRIVATE tree-lib)