set(CMAKE_CXX_STANDARD 17)

# 所有在 src 目录及其子目录下找到的 .cpp 文件和在 include 目录及其子目录下找到的 .hpp 文件都会被添加到 source 变量中
file(GLOB_RECURSE source CONFIGURE_DEPENDS src/*.cpp include/*.hpp)

#RayTracing 可执行文件将会由 source 变量中列出的所有源文件构建而成
add_executable(RayTracing ${source} "src/scene.cpp")

# 该目录和头文件拼接
include_directories("../third_party")

# 指定头文件
target_include_directories(RayTracing PUBLIC include)

# 指定目标需要链接的库
target_link_libraries(RayTracing glm)

#target_link_libraries(RayTracing assimp)