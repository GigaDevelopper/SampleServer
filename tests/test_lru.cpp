#define BOOST_TEST_MODULE LRUCacheTests
#include <boost/test/unit_test.hpp>

#include "../utils/lrucache.h"

BOOST_AUTO_TEST_CASE(test_cache_basic_operations) {
    constexpr int CACHE_CAPACITY = 3;
    server::utils::lrucache cache(CACHE_CAPACITY);

    cache.put("key1", "value1");
    cache.put("key2", "value2");
    cache.put("key3", "value3");

    BOOST_CHECK_EQUAL(cache.get("key1"), "value1");
    BOOST_CHECK_EQUAL(cache.get("key2"), "value2");
    BOOST_CHECK_EQUAL(cache.get("key3"), "value3");

    cache.put("key2", "new_value2");
    BOOST_CHECK_EQUAL(cache.get("key2"), "new_value2");

    cache.put("key4", "value4");
    BOOST_CHECK(cache.get("key1").empty());
}

BOOST_AUTO_TEST_CASE(test_cache_capacity_handling) {
    constexpr int CACHE_CAPACITY = 2;
    server::utils::lrucache cache(CACHE_CAPACITY);

    cache.put("key1", "value1");
    cache.put("key2", "value2");

    cache.put("key3", "value3");
    BOOST_CHECK(cache.get("key1").empty());
    BOOST_CHECK_EQUAL(cache.get("key2"), "value2");
    BOOST_CHECK_EQUAL(cache.get("key3"), "value3");
}

BOOST_AUTO_TEST_CASE(test_cache_invalid_capacity) {
    BOOST_CHECK_THROW(server::utils::lrucache cache(-1), std::invalid_argument);
}

