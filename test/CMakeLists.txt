add_executable(test_lru_cache lru_cache/main.cpp)
target_include_directories(test_lru_cache PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
add_test(NAME test_lru_cache COMMAND test_lru_cache)

add_executable(test_circular_buffer test_circular_buffer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/circular_buffer.c)
target_include_directories(test_circular_buffer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
add_test(NAME test_circular_buffer COMMAND test_circular_buffer)