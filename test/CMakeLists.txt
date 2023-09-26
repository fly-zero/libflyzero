add_executable(test_lru_cache
    lru_cache/main.cpp)

target_include_directories(test_lru_cache
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)