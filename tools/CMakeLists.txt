add_executable(extract extract.cpp)
target_link_libraries(extract PRIVATE extractor arg_parser)

add_executable(preprocess preprocess.cpp)
target_link_libraries(preprocess PRIVATE support arg_parser)

set(CMAKE_AUTOMOC ON)
add_executable(testmarker testmarker.cpp)
target_link_libraries(testmarker PRIVATE marker db tree_sitter arg_parser)