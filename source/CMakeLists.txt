set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE source CONFIGURE_DEPENDS src/*.cpp)

add_executable(RayTracing ${source})

# 该目录和头文件拼接
include_directories("../third_party")

target_include_directories(RayTracing PUBLIC include)

target_link_libraries(RayTracing glm)
