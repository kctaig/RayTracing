cmake_minimum_required(VERSION 3.12)

project(
    RayTracing
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME ${CMAKE_PROJECT_NAME})
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

add_subdirectory(3rdparty)
add_subdirectory(source)

