project(UrbanEngine)

# add_subdirectory(vendor/glm)
add_subdirectory(vendor/entt)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/rImGui)
add_subdirectory(vendor/spdlog)
# set(YAML_CPP_BUILD_CONTRIB OFF)
# set(YAML_CPP_BUILD_TOOLS OFF)
# set(YAML_CPP_BUILD_TESTS OFF)
# set(YAML_BUILD_SHARED_LIBS OFF)
# add_subdirectory(vendor/yaml-cpp)
# set(USE_STATIC_MSVC_RUNTIME_LIBRARY ON)
# add_subdirectory(vendor/JoltPhysics/build)
add_subdirectory(vendor/raylib)

# gather all source files
if(WIN32)
    file(GLOB_RECURSE windowFile ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Windows/*.cpp)
else()
    file(GLOB_RECURSE windowFile )
endif(WIN32)
if(UNIX)
    file(GLOB_RECURSE linuxFile ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Linux/*.cpp)
else()
    file(GLOB_RECURSE linuxFile )
endif(UNIX)
if(APPLE)
    file(GLOB_RECURSE macFile ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Mac/*.cpp)
else()
    file(GLOB_RECURSE macFile )
endif()

file(GLOB_RECURSE source ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

file(GLOB ImGuizmo ${CMAKE_CURRENT_SOURCE_DIR}/vendor/ImGuizmo/ImGuizmo.cpp)
file(GLOB ImGuizmoH ${CMAKE_CURRENT_SOURCE_DIR}/vendor/ImGuizmo/ImGuizmo.h)

file(GLOB CoreFiles ${CMAKE_CURRENT_SOURCE_DIR}/src/Core/*.cpp)
file(GLOB RendererFiles ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/*.cpp)
file(GLOB SceneFiles ${CMAKE_CURRENT_SOURCE_DIR}/src/Scene/*.cpp)

file(GLOB_RECURSE openGLFile ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/OpenGL/*.cpp)

add_library(${PROJECT_NAME} ${source} ${CoreFiles} ${RendererFiles} ${SceneFiles} ${headers} 
            ${windowFile} ${linuxFile} ${macFile} 
            ${openGLFile} 
            ${ImGuizmoH} ${ImGuizmo} 
            )

target_precompile_headers(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/uepch.h)

# export engine symbols
target_compile_definitions(${PROJECT_NAME} PUBLIC
  -DUE_EXPORT
)

# target_precompile_headers(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/tbpch.h)

# includes directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include  
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Core  
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/entt/src/entt
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm     
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends    
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/ImGuizmo
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/yaml-cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/JoltPhysics
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/raylib/src  
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/rImGui  
    
)


# link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC    
    raylib
    rImGui
#   glm
  spdlog
  imgui
  EnTT       
#   yaml-cpp
#   Jolt
)