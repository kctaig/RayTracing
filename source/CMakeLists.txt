set(CMAKE_CXX_STANDARD 17)

aux_source_directory(src SOURCE_FILES)
file(GLOB_RECURSE HEADER_FILES "include/*.hpp")

find_package(OpenMP REQUIRED)

add_executable(RayTracing ${SOURCE_FILES} ${HEADER_FILES})

# 设置头文件
set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/3rdparty")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(GLM_INCLUDE "${THIRD_PARTY_DIR}/glm")
set(MODEL_LOADER_INCLUDE "${THIRD_PARTY_DIR}/tiny_obj_loader")
set(TINYXML_DIR "${THIRD_PARTY_DIR}/tinyxml2")
set(TEXTURE_LOADER_INCLUDE "${THIRD_PARTY_DIR}/stb_image")

# 指定目标需要链接的库
target_link_libraries(RayTracing 
glm
OpenMP::OpenMP_CXX
tinyxml2
)

# 头文件目录
target_include_directories(${TARGET_NAME} PUBLIC ${THIRD_PARTY_DIR})
target_include_directories(${TARGET_NAME} PUBLIC ${INCLUDE_DIR})
target_include_directories(${TARGET_NAME} PUBLIC ${GLM_INCLUDE})
target_include_directories(${TARGET_NAME} PUBLIC ${MODEL_LOADER_INCLUDE})
target_include_directories(${TARGET_NAME} PUBLIC ${TEXTURE_LOADER_INCLUDE})