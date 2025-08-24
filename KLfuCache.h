#pragma once

#include <cmath>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "KICachePolicy.h"

namespace KamaCache
{
    // Forward Declaration
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
        using Node = typename FreqList<Key, Value>::Node
        using NodePtr = std::shared_ptr<Node>;
        using NodMap = std::unordered_map<Key, NodePtr>;

        private:
        int capacity_;
        std::mutex mutex_;
        NodeMap nodeMap_;
        std::unordered_map<int, FreqList<Key, Value>*> freqToFreqList_;

    };
}