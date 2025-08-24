#pragma once

#include <cmath>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include <climits>

#include "KICachePolicy.h"

namespace KamaCache
{
    // Forward Declaration
    // using hash map (for cache itself) and another hash map tp FreqList to keep track
    // of the minFreq
    template<typename Key, typename Value> class KLfuCache;

    template<typename Key, typename Value>
    class FreqList
    {
        private:
        struct Node
        {
            int freq;
            Key key;
            Value value;
            std::weak_ptr<Node> pre;
            std::shared_ptr<Node> next;

            Node()
            : freq(1), next(nullptr) {}
            Node(Key key, Value value)
            : freq(1), key(key), value(value), next(nullptr) {}
        };

        using NodePtr = std::shared_ptr<Node>;
        int freq_;
        NodePtr head_;
        NodePtr tail_;

        public:
        // rule of thumb: use explicit for single-parameter constructors
        // unless you specifically want implicit conversions.
        explicit FreqList(int n)
        : freq_(n)
        {
            head_ = std::make_shared<Node>();
            tail_ = std::make_shared<Node>();
            head_->next = tail_;
            tail_->pre = head_;
        }

        bool isEmpty() const
        {
            return head_->next == tail_;
        }

        void addNode(NodePtr node)
        {
            if (!node || !head_ || !tail_) return;

            node->pre = tail_->pre;
            node->next = tail_;
            tail_->pre.lock()->next = node;
            tail_->pre = node;
        }

        void removeNode(NodePtr node)
        {
            if (!node || !head_ || !tail_) return;
            if (node->pre.expired() || !node->next) return;

            auto pre = node->pre.lock();
            pre->next = node->next;
            node->next->pre = pre;
            node->next = nullptr;
        }

        NodePtr getFirstNode() const { return head_->next; }
        friend class KLfuCache<Key, Value>;
    };

    template <typename Key, typename Value>
    class KLfuCache : public KICachePolicy<Key, Value>
    {
        public:
        using Node = typename FreqList<Key, Value>::Node;
        using NodePtr = std::shared_ptr<Node>;
        using NodeMap = std::unordered_map<Key, NodePtr>;

        private:
        int capacity_;
        int minFreq_;
        int curAverageNum_;
        int curTotalNum_;
        int maxAverageNum_;
        std::mutex mutex_;
        NodeMap nodeMap_;
        std::unordered_map<int, FreqList<Key, Value>*> freqToFreqList_;

        private:
        void putInternal(Key key, Value value);
        void getInternal(NodePtr node, Value& value);
        void kickOut();
        void removeFromFreqList(NodePtr node);
        void addToFreqList(NodePtr node);
        void decreaseFreqNum(int num); // decrease cutTotalNum_ and update curAverageNum_
        void addFreqNum(); // add curTotalNum_
        void handleOverMaxAverageNum();
        void updateMinFreq();

        public:
        KLfuCache(int capacity, int maxAverageNum = 1000000)
        : capacity_(capacity), minFreq_(INT_MAX), maxAverageNum_(maxAverageNum),
            curAverageNum_(0), curTotalNum_(0)
            {}

        ~KLfuCache() override = default;

        void put(const Key& key, const Value& value) override
        {
            if (capacity_ == 0) { return; }

            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                it->second->value = value;
                Value new_value;
                getInternal(it->second, new_value);
                return;
            }

            putInternal(key, value);
        }

        bool get(Key key, Value& value) override
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                getInternal(it->second, value);
                return true;
            }
            return false;
        }

        Value get(Key key) override
        {
            Value value;
            get(key, value);
            return value;
        }

    };


template<typename Key, typename Value>
void KLfuCache<Key, Value>::putInternal(Key key, Value value)
{
    if (nodeMap_.size() == capacity_) { kickOut(); }

    NodePtr node = std::make_shared<Node>(key, value);
    nodeMap_[key] = node;
    addToFreqList(node);
    addFreqNum();
    minFreq_ = std::min(minFreq_, 1);
}

template<typename Key, typename Value>
void KLfuCache<Key, Value>::getInternal(NodePtr node, Value& value)
{   
    value = node->value;
    removeFromFreqList(node);
    node->freq++;
    addToFreqList(node);
    if (node->freq - 1 == minFreq_ && freqToFreqList_[node->freq - 1]->isEmpty()) { minFreq_ ++;}
    addFreqNum();
}

template<typename Key, typename Value>
void KLfuCache<Key, Value>::kickOut()
{
    NodePtr node = freqToFreqList_[minFreq_]->getFirstNode();
    removeFromFreqList(node);
    nodeMap_.erase(node->key);
    decreaseFreqNum(node->freq);
}

template<typename Key, typename Value>
void KLfuCache<Key, Value>::removeFromFreqList(NodePtr node)
{
    if (!node) return;

    auto freq = node->freq;
    freqToFreqList_[freq]->removeNode(node);
}

template<typename Key, typename Value>
void KLfuCache<Key, Value>::addToFreqList(NodePtr node)
{
    if (!node) return;

    auto freq = node->freq;
    if (freqToFreqList_.find(node->freq) == freqToFreqList_.end())
    {
        freqToFreqList_[freq] = new FreqList<Key, Value>(node->freq);
    }
    freqToFreqList_[freq]->addNode(node);
}

template<typename Key, typename Value>
void KLfuCache<Key, Value>::decreaseFreqNum(int num)
{
    curTotalNum_ -= num;
    if (nodeMap_.empty()) {
        curAverageNum_ = 0;
    } else {
        curAverageNum_ = curTotalNum_ / nodeMap_.size();
    }
}

template<typename Key, typename Value>
void KLfuCache<Key, Value>::addFreqNum()
{
    curTotalNum_++;
    if (nodeMap_.empty()) {
        curAverageNum_ = 0;
    } else {
        curAverageNum_ = curTotalNum_ / nodeMap_.size();
    }

    if (curAverageNum_ > maxAverageNum_) {
        handleOverMaxAverageNum();
    }
}

template<typename Key, typename Value>
void KLfuCache<Key, Value>::handleOverMaxAverageNum()
{
    if (nodeMap_.empty()) { return; }

    for (auto it = nodeMap_.begin(); it != nodeMap_.end(); ++it)
    {
        if (!it->second) { continue; }

        NodePtr node = it->second;
        removeFromFreqList(node);
        node->freq -= maxAverageNum_ / 2;
        if (node->freq < 1) { node->freq = 1; }

        addToFreqList(node);
    }
    updateMinFreq();
}

template<typename Key, typename Value>
void KLfuCache<Key, Value>::updateMinFreq()
{
    minFreq_ = INT_MAX;
    for (const auto& pair : freqToFreqList_)
    {
        if (pair.second && !pair.second->isEmpty())
        {
            minFreq_ = std::min(minFreq_, pair.first);
        }
        if (minFreq_ == INT_MAX) {
            minFreq_ = 1;
        }
    }

}

} // namespace KamaCache