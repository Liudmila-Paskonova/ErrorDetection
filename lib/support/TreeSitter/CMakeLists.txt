add_library(tree_sitter STATIC TreeSitter.cpp)
target_include_directories(tree_sitter PUBLIC
    ${CMAKE_SOURCE_DIR}/include/support/TreeSitter
)
target_link_libraries(tree_sitter PUBLIC tree-lib)