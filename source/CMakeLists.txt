set(CMAKE_CXX_STANDARD 17)

aux_source_directory(src SOURCE_FILES)
file(GLOB_RECURSE HEADER_FILES "include/*.hpp")

add_executable(${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES})

set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/3rdparty")

# 头文件目录
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET_NAME} PUBLIC ${THIRD_PARTY_DIR}/glm)
target_include_directories(${TARGET_NAME} PUBLIC ${THIRD_PARTY_DIR}/tiny_obj_loader)
target_include_directories(${TARGET_NAME} PUBLIC ${THIRD_PARTY_DIR}/tinyxml2)
target_include_directories(${TARGET_NAME} PUBLIC ${THIRD_PARTY_DIR}/stb_image)
target_include_directories(${TARGET_NAME} PUBLIC ${THIRD_PARTY_DIR}/imgui)
target_include_directories(${TARGET_NAME} PUBLIC ${THIRD_PARTY_DIR}/glfw/include)

# 添加OpenMP
find_package(OpenMP REQUIRED)
if (NOT OPENMP_FOUND)
    message("Not find OpenMP")
    return ()
else ()
    target_link_libraries(${TARGET_NAME} OpenMP::OpenMP_CXX)
endif()

# 连接其他库
target_link_libraries(${TARGET_NAME} 
    glm
    OpenMP::OpenMP_CXX
    tinyxml2
    glfw
    imgui
    opengl32
)
