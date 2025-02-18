# 设置 tinyxml2 库名称
project(tinyxml2)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 创建静态库（如果你希望创建动态库，使用 SHARED 替换 STATIC）
add_library(tinyxml2 STATIC
    tinyxml2.cpp
)

# 设置目标包含目录
target_include_directories(tinyxml2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

