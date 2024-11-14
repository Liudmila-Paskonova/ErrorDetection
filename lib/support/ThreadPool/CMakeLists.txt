add_library(thread_pool STATIC ThreadPool.cpp)
target_include_directories(thread_pool PUBLIC
    ${CMAKE_SOURCE_DIR}/include/support/ThreadPool
)