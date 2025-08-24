# 设置最低 CMake 版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(KCacheSystem)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 启用测试
enable_testing()

# 查找 Google Test
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# 主程序可执行文件
add_executable(main testAllCachePolicy.cpp)

# 单元测试可执行文件
add_executable(unit_tests 
    unitTests/unitTestKLruCache.cpp
    unitTests/unitTestKLfuCache.cpp
)

# 链接 Google Test 和线程库到测试可执行文件
target_link_libraries(unit_tests 
    GTest::GTest 
    GTest::Main 
    Threads::Threads
)

# 添加测试
add_test(NAME CacheTests COMMAND unit_tests)

# 清理中间的 .o 文件
set_target_properties(main PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(unit_tests PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# 额外的编译选项（可根据需要启用）
# target_compile_options(main PRIVATE -Wall -Wextra -O2)
# target_compile_options(unit_tests PRIVATE -Wall -Wextra -O2)

target_compile_options(unit_tests PRIVATE --coverage)
target_link_libraries(unit_tests --coverage)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")