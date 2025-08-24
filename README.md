# myKamaCache

A thread-safe LRU (Least Recently Used) cache implementation in C++ with comprehensive unit tests.

## Features

- **Thread-safe**: Uses mutex for concurrent access
- **Template-based**: Supports any key-value types
- **Interface design**: Abstract base class for extensibility
- **Memory efficient**: Smart pointer-based doubly-linked list
- **Performance tested**: Includes hot/cold data access benchmarks

## Build and Test

### Prerequisites
- CMake 3.10+
- C++17 compiler
- Google Test library

### Install Google Test (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install libgtest-dev cmake build-essential
```

### Build
```bash
mkdir build && cd build
cmake ..
make
```

### Run Tests
```bash
# Run unit tests
./unit_tests

# Or use CTest
ctest --verbose

# Run benchmark
./main
```

### Unit Test Coverage
```
cd build
make unit_tests
./unit_tests
gcov CMakeFiles/unit_tests.dir/unitTests/unitTestKLruCache.cpp.gcda | grep -C 3 KLru
lcov --capture --directory CMakeFiles/unit_tests.dir/ --output-file coverage.info --ignore-errors mismatch
lcov --remove coverage.info '/usr/*' --output-file coverage_clean.info
genhtml coverage_clean.info --output-directory coverage_html
```


## Usage Example

```cpp
#include "KLruCache.h"

KamaCache::KLruCache<int, std::string> cache(100);

// Insert
cache.put(1, "hello");
cache.put(2, "world");

// Retrieve
std::string value;
if (cache.get(1, value)) {
    std::cout << value << std::endl; // prints "hello"
}

// Remove
cache.remove(1);
```