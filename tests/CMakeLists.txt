set(TEST_SOURCES test_lru.cpp)

add_executable(runTests ${TEST_SOURCES})
target_link_libraries(runTests lrucache ${Boost_LIBRARIES})

enable_testing()
add_test(NAME LRUCacheTests COMMAND runTests)
