#include <gtest/gtest.h>
#include <memory>
#include "../KLfuCache.h"

using namespace KamaCache;

class FreqListTest : public ::testing::Test {
protected:
    void SetUp() override {
        freqList = std::make_unique<FreqList<int, std::string>>(1);
    }
    
    std::unique_ptr<FreqList<int, std::string>> freqList;
};

// Test FreqList construction and isEmpty
TEST_F(FreqListTest, Construction) {
    EXPECT_TRUE(freqList->isEmpty());
}


class LfuCacheTest : public ::testing::Test {
protected:
    void SetUp() override {
        cache = std::make_unique<KamaCache::KLfuCache<int, std::string>>(3);
    }
    std::unique_ptr<KamaCache::KLfuCache<int, std::string>> cache;
};


TEST_F(LfuCacheTest, PutAndGet) {
    cache->put(1, "one");
    cache->put(2, "two");
    
    std::string value;
    EXPECT_TRUE(cache->get(1, value));
    EXPECT_EQ(value, "one");
    
    EXPECT_TRUE(cache->get(2, value));
    EXPECT_EQ(value, "twe");
}