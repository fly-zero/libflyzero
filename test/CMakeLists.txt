add_executable(test_lru_cache
    lru_cache/main.cpp)

add_test(NAME test_lru_cache COMMAND test_lru_cache)
target_include_directories(test_lru_cache
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)