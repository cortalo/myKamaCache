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
        // size_t getAccessCount() const { return accessCount_; }
        void setValue(const Value& value) { value_ = value; }
        // void incrementAccessCount() { ++accessCount_; }

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
        NodeMap nodeMap_;
        std::mutex mutex_;
        NodePtr dummyHead_;
        NodePtr dummyTail_; 

        private: 
        // double linked list private methods
        // - initializeList
        // - removeNode
        // - insertNode
        // - moveToMostRecent
        // - updateExistingNode

        // double linked list: initializeList
        // will be called in the constructor
        void initializeList()
        {
            dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyHead_->next_ = dummyTail_;
            dummyTail_->prev_ = dummyHead_;
        }

        // double linked list: removeNode
        void removeNode(NodePtr node)
        {
            //if(!node->prev_.expired() && node->next_)
            //{
                auto prev = node->prev_.lock(); // convert to shared_ptr
                prev->next_ = node->next_;
                node->next_->prev_ = prev;
                node->next_ = nullptr;
            //}
        }

        // double linked list: insetNode
        void insertNode(NodePtr node)
        {
            node->next_ = dummyTail_;
            node->prev_ = dummyTail_->prev_;
            dummyTail_->prev_.lock()->next_ = node;
            dummyTail_->prev_ = node;
        }

        // double linked list: moveToMostRecent
        void moveToMostRecent(NodePtr node)
        {
            removeNode(node);
            insertNode(node);
        }

        // double linked list: updateExistingNode
        // only update its value and moveToMostRecent
        void updateExistingNode(NodePtr node, const Value& value)
        {
            node->setValue(value);
            moveToMostRecent(node);
        }

        private:
        // KLruCache private methods
        // - evictLeastRecent
        // - addNewNode

        // KLruCache: evictLeaseRecent
        void evictLeastRecent()
        {
            NodePtr leastRecent = dummyHead_->next_;
            removeNode(leastRecent);
            nodeMap_.erase(leastRecent->getKey());
        }

        // KLruCache: addNewNode
        void addNewNode(const Key& key, const Value& value)
        {
            if (nodeMap_.size() >= capacity_)
            {
                evictLeastRecent();
            }
            NodePtr newNode = std::make_shared<LruNodeType>(key, value);
            insertNode(newNode);
            nodeMap_[key] = newNode;
        }


        public:
        // KLruCache constructor and destructor
        KLruCache(int capacity)
        : capacity_(capacity)
        {
            initializeList();
        }
        ~KLruCache() override = default;

        // basic access methods (need to make sure move to front)
        // 1. insertNode() ==> insert to front
        // 2. updateExistingNode()
        // 3. get(key, value)
        // all the other access methods just use the three above

        public:
        // KLruCache public methods
        // - put
        // - get
        // - remove

        // KLruCache: put
        void put(const Key& key, const Value& value) override
        {
            if (capacity_ <= 0) return;

            // the key may or may not be inside the cache already
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end())
            {
                updateExistingNode(it->second, value);
                return ;
            }
            addNewNode(key, value);
        }

        // KLruCache: get
        bool get(Key key, Value& value) override {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end())
            {
                moveToMostRecent(it->second);
                value = it->second->getValue();
                return true;
            }
            return false;
        }

        // KLruCache: get
        Value get(Key key) override
        {
            Value value{};
            get(key, value);
            return value; }

        void remove(Key key)
        {
            std::lock_guard<std::mutex> lock{mutex_};
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end())
            {
                removeNode(it->second);
                nodeMap_.erase(it);
            }
        }

    };
}