cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# build output directories

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<0:>${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<0:>${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $<0:>${CMAKE_BINARY_DIR}/lib)
set(CMAKE_PDB_OUTPUT_DIRECTORY $<0:>${CMAKE_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH $<0:>${CMAKE_BINARY_DIR}/bin)

# cpp standard options

set(CMAKE_CONFIGURATION_TYPES Debug Release)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 17)

# if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
# endif()

project(UrbanEngineProgram C CXX)

# enable console logging

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    add_definitions(-DUE_DEBUG)
endif(${CMAKE_BUILD_TYPE} MATCHES Debug)

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glad)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/entt)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/spdlog)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON)
set(ASSIMP_BUILD_MINIZIP OFF)
set(ASSIMP_BUILD_ARCHIVE OFF)
add_subdirectory(vendor/assimp)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF)
add_subdirectory(vendor/yaml-cpp)
set(USE_STATIC_MSVC_RUNTIME_LIBRARY ON)
# set(JPH_DEBUG_RENDERER ON)
add_subdirectory(vendor/JoltPhysics/Build)
add_subdirectory(vendor/ImGuiFileDialog)
set(ozz_build_samples OFF)
set(ozz_build_howtos OFF)
set(ozz_build_tests OFF)
set(ozz_build_fbx ON)
add_subdirectory(vendor/ozz)

add_subdirectory(Engine)
add_subdirectory(Editor)
add_subdirectory(Sandbox)