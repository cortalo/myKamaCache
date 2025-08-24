#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "../KLruCache.h"

class LruCacheTest : public ::testing::Test {
protected:
    void SetUp() override {
        cache = std::make_unique<KamaCache::KLruCache<int, std::string>>(3);
        cacheZeroCapacity = std::make_unique<KamaCache::KLruCache<int, std::string>>(0);
    }
    
    std::unique_ptr<KamaCache::KLruCache<int, std::string>> cache;
    std::unique_ptr<KamaCache::KLruCache<int, std::string>> cacheZeroCapacity;
};

// Basic functionality tests
TEST_F(LruCacheTest, PutAndGet) {
    cache->put(1, "one");
    cache->put(2, "two");
    
    std::string value;
    EXPECT_TRUE(cache->get(1, value));
    EXPECT_EQ(value, "one");
    
    EXPECT_TRUE(cache->get(2, value));
    EXPECT_EQ(value, "two");

    cache->put(3, "three");
    cache->put(4, "four");
    EXPECT_TRUE(cache->get(3, value));
    EXPECT_EQ(value, "three");
    EXPECT_TRUE(cache->get(4, value));
    EXPECT_EQ(value, "four");
    EXPECT_FALSE(cache->get(1, value));
    EXPECT_EQ(value, "four");
}

// LRU Cache Capacity Test
TEST_F(LruCacheTest, CapacityTest) {
    cache->put(1, "one");
    cache->put(2, "two");
    cache->put(3, "three");
    cache->put(4, "four");
    std::string value;
    EXPECT_FALSE(cache->get(1, value));
    EXPECT_EQ(value, "");
    EXPECT_TRUE(cache->get(2, value));
    EXPECT_EQ(value, "two");
    EXPECT_TRUE(cache->get(3, value));
    EXPECT_EQ(value, "three");
    EXPECT_TRUE(cache->get(4, value));
    EXPECT_EQ(value, "four");
    EXPECT_FALSE(cache->get(1, value));
    EXPECT_EQ(value, "four");
}

// LRU updateExistingNode Test
TEST_F(LruCacheTest, UpdateExistingNode) {
    std::string value;
    cache->put(1, "one");
    EXPECT_TRUE(cache->get(1, value));
    EXPECT_EQ(value, "one");
    cache->put(1, "oneone");
    EXPECT_TRUE(cache->get(1, value));
    EXPECT_EQ(value, "oneone");
}

// LRU Get Test
TEST_F(LruCacheTest, GetTest) {
    std::string value;
    cache->put(1, "one");
    value = cache->get(1);
    EXPECT_EQ(value, "one");
}


// LRU Zero Capacity Test
TEST_F(LruCacheTest, ZeroCapacity) {
    cacheZeroCapacity->put(1, "one");
    std::string value;
    EXPECT_FALSE(cacheZeroCapacity->get(1, value));
}

// LRU Remove Test
TEST_F(LruCacheTest, Remove) {
    cache->put(1, "one");
    cache->remove(1);
    std::string value;
    EXPECT_FALSE(cache->get(1, value));
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}