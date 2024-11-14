add_library(db STATIC Metadata.cpp)
target_include_directories(db PUBLIC
    ${CMAKE_SOURCE_DIR}/include/support/Metadata
)

target_link_libraries(db PUBLIC SQLite::SQLite3)
