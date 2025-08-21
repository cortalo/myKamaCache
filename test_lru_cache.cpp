#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "KLruCache.h"

class LruCacheTest : public ::testing::Test {
protected:
    void SetUp() override {
        cache = std::make_unique<KamaCache::KLruCache<int, std::string>>(3);
    }
    
    std::unique_ptr<KamaCache::KLruCache<int, std::string>> cache;
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
}

TEST_F(LruCacheTest, GetNonExistentKey) {
    std::string value;
    EXPECT_FALSE(cache->get(999, value));
}

TEST_F(LruCacheTest, UpdateExistingKey) {
    cache->put(1, "one");
    cache->put(1, "updated_one");
    
    std::string value;
    EXPECT_TRUE(cache->get(1, value));
    EXPECT_EQ(value, "updated_one");
}

TEST_F(LruCacheTest, CapacityEviction) {
    // Fill cache to capacity
    cache->put(1, "one");
    cache->put(2, "two");
    cache->put(3, "three");
    
    // Add one more - should evict least recently used (1)
    cache->put(4, "four");
    
    std::string value;
    EXPECT_FALSE(cache->get(1, value)); // Should be evicted
    EXPECT_TRUE(cache->get(2, value));  // Should still exist
    EXPECT_TRUE(cache->get(3, value));  // Should still exist
    EXPECT_TRUE(cache->get(4, value));  // Should exist
}

TEST_F(LruCacheTest, LruOrdering) {
    cache->put(1, "one");
    cache->put(2, "two");
    cache->put(3, "three");
    
    // Access key 1 to make it most recently used
    std::string value;
    cache->get(1, value);
    
    // Add new key - should evict 2 (least recently used)
    cache->put(4, "four");
    
    EXPECT_TRUE(cache->get(1, value));  // Should still exist
    EXPECT_FALSE(cache->get(2, value)); // Should be evicted
    EXPECT_TRUE(cache->get(3, value));  // Should still exist
    EXPECT_TRUE(cache->get(4, value));  // Should exist
}

TEST_F(LruCacheTest, RemoveKey) {
    cache->put(1, "one");
    cache->put(2, "two");
    
    cache->remove(1);
    
    std::string value;
    EXPECT_FALSE(cache->get(1, value));
    EXPECT_TRUE(cache->get(2, value));
}

TEST_F(LruCacheTest, ZeroCapacity) {
    auto zero_cache = std::make_unique<KamaCache::KLruCache<int, std::string>>(0);
    zero_cache->put(1, "one");
    
    std::string value;
    EXPECT_FALSE(zero_cache->get(1, value));
}

TEST_F(LruCacheTest, GetWithReturnValue) {
    cache->put(1, "one");
    
    std::string value = cache->get(1);
    EXPECT_EQ(value, "one");
    
    // Non-existent key should return default value
    value = cache->get(999);
    EXPECT_EQ(value, "");
}

// Thread safety tests
TEST_F(LruCacheTest, ConcurrentAccess) {
    const int num_threads = 4;
    const int operations_per_thread = 100;
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t, operations_per_thread]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                int key = t * operations_per_thread + i;
                std::string value = "thread_" + std::to_string(t) + "_value_" + std::to_string(i);
                
                cache->put(key, value);
                
                std::string retrieved_value;
                cache->get(key, retrieved_value);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Cache should still be functional after concurrent access
    cache->put(999, "test");
    std::string value;
    EXPECT_TRUE(cache->get(999, value));
    EXPECT_EQ(value, "test");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}