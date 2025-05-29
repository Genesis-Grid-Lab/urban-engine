cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 17)

# gather all source files
file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

project(ImGui)

add_library(imgui ${headers} ${sources} ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)

target_include_directories(imgui PUBLIC 
            ${CMAKE_CURRENT_SOURCE_DIR}/
            ${CMAKE_SOURCE_DIR}/vendor/glfw/include/
            )
