add_library(support STATIC Support.cpp)
target_include_directories(support PUBLIC
    ${CMAKE_SOURCE_DIR}/include/support/Support
)