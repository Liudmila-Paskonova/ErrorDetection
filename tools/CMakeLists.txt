add_executable(extract extract.cpp)
target_link_libraries(extract PRIVATE extractor arg_parser)

add_executable(preprocess preprocess.cpp)
target_link_libraries(preprocess PRIVATE support arg_parser)
