cmake_minimum_required(VERSION 3.0)
project(ImGuizmo)

add_library(${PROJECT_NAME} include/glad/glad.h src/glad.c)
target_include_directories(glad PUBLIC include/)
