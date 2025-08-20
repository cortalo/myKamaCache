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

void testHotDataAccess() {
    std::cout << "=== Testbench 1: Hot Data Access ===" << std::endl;
    
    const int CAPACITY = 20;

    KamaCache::KLruCache<int, std::string> lru(CAPACITY);
}

int main() {
    testHotDataAccess();

    const int CAPACITY = 20;
    const int HOT_KEYS = 20;
    const int COLD_KEYS = 5000;
    return 0;
}