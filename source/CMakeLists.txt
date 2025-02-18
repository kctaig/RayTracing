set(CMAKE_CXX_STANDARD 17)

aux_source_directory(src SOURCE_FILES)
file(GLOB_RECURSE HEADER_FILES "include/*.hpp")

find_package(OpenMP REQUIRED)

add_executable(RayTracing ${SOURCE_FILES} ${HEADER_FILES} "src/log.cpp")

# 指定头文件
target_include_directories(RayTracing PUBLIC include)
target_compile_features(RayTracing PUBLIC cxx_std_17)

# 头文件目录
include_directories("../third_party")

# 指定目标需要链接的库
target_link_libraries(RayTracing 
glm
OpenMP::OpenMP_CXX
)