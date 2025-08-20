#pragma once 

#include <cstring>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "KICachePolicy.h"

namespace KamaCache
{
    // Forward Declaration
    template<typename Key, typename Value> class KLruCache;

    template<typename Key, typename Value>
    class LruNode
    {
        private:
        Key key_;
        Value value_;
        size_t accessCount_;
        std::weak_ptr<LruNode<Key, Value>> prev_;
        std::shared_ptr<LruNode<Key, Value>> next_;

        public:
        LruNode(Key key, Value value)
            : key_(key)
            , value_(value)
            , accessCount_(1)
            {}

        Key getKey() const { return key_; }
        Value getValue() const { return value_; }
        size_t getAccessCount() const { return accessCount_; }
        void setValue(const Value& value) { value_ = value; }
        void incrementAccessCount() { ++accessCount_; }

        friend class KLruCache<Key, Value>;

    };

    template<typename Key, typename Value>
    class KLruCache : public KICachePolicy<Key, Value>
    {

        public:
        using LruNodeType = LruNode<Key, Value>;
        using NodePtr = std::shared_ptr<LruNodeType>;
        using NodeMap = std::unordered_map<Key, NodePtr>;

        private:
        int capacity_;
        NodePtr dummyHead_;
        NodePtr dummyTail_; 

        void initializeList()
        {
            dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyHead_->next_ = dummyTail_;
            dummyTail_->prev_ = dummyHead_;
        }

        public:
        KLruCache(int capacity)
        : capacity_(capacity)
        {
            std::cout<< "construct KLruCache" << std::endl;
            initializeList();
        }

        ~KLruCache() override = default;

        void put(Key key, Value value) override
        {

        }

        bool get(Key key, Value& value) override {
            return false;
        }

        Value get(Key key) override
        {
            Value value{};
            return value;
        }

    };
}