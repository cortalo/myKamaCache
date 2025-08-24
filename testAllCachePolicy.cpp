#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <iomanip>
#include <random>
#include <algorithm>
#include <array>

#include "KICachePolicy.h"
#include "KLruCache.h"
#include "KLfuCache.h"

void testHotDataAccess() {
    std::cout << "=== Testbench 1: Hot Data Access ===" << std::endl;
    
    const int CAPACITY = 20;
    const int OPERATIONS = 500000;   // total #operaions
    const int HOT_KEYS = 20;
    const int COLD_KEYS = 5000;

    KamaCache::KLruCache<int, std::string> lru(CAPACITY);
    KamaCache::KLfuCache<int, std::string> lfu(CAPACITY);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::array<KamaCache::KICachePolicy<int, std::string>*, 2> caches = {&lru, &lfu};
    // hits and get_operations vector to calculate hits percentage
    std::vector<int> hits(2, 0);
    std::vector<int> get_operations(2, 0);
    std::vector<std::string> names = {"LRU", "LFU"};

    for (int i = 0; i < caches.size(); ++i)
    {
        // init cache with some hot data
        for (int key = 0; key < HOT_KEYS; ++key) {
            std::string value = "Value" + std::to_string(key);
            caches[i]->put(key, value);
        }

        for (int op = 0; op < OPERATIONS; ++op) {
            // put operation with 30% possibility
            bool isPut = (gen() % 100 < 30);
            int key{};

            // 70% possibility with hot keys
            // 30% possibility with cold keys
            if (gen() % 100 < 70) {
                key = gen() % HOT_KEYS;
            } else {
                key = HOT_KEYS + (gen() % COLD_KEYS);
            }

            if (isPut) {
                std::string value = "value" + std::to_string(key) + "_v" + std::to_string(op % 100);
                caches[i]->put(key, value);
            } else {
                std::string result{};
                get_operations[i]++;
                if (caches[i]->get(key, result)) {
                    hits[i]++;
                }
            }
        }

        double hitRate = 100.0 * hits[i] / get_operations[i];
        std::cout << "Algorithm " + names[i] + " hit percenrage: "
                << std::fixed << std::setprecision(2) << hitRate << "% ";
        std::cout << "(" << hits[i] << "/" << get_operations[i] << ")" << std::endl;
    }
}

int main() {
    testHotDataAccess();
    return 0;
}