project(UrbanEngine)

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

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif(UNIX)

file(GLOB_RECURSE source ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

file(GLOB ImGuizmo ${CMAKE_SOURCE_DIR}/vendor/ImGuizmo/ImGuizmo.cpp)
file(GLOB ImGuizmoH ${CMAKE_SOURCE_DIR}/vendor/ImGuizmo/ImGuizmo.h)

file(GLOB CoreFiles ${CMAKE_CURRENT_SOURCE_DIR}/src/Core/*.cpp)
file(GLOB RendererFiles ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/*.cpp)
file(GLOB SceneFiles ${CMAKE_CURRENT_SOURCE_DIR}/src/Scene/*.cpp)

file(GLOB_RECURSE openGLFile ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/OpenGL/*.cpp)

add_library(${PROJECT_NAME} ${source} ${CoreFiles} ${RendererFiles} ${SceneFiles} ${headers} 
            ${windowFile} ${linuxFile} ${macFile} 
            ${openGLFile} 
            ${ImGuizmoH} ${ImGuizmo} 
            ${CMAKE_SOURCE_DIR}/vendor/stb_image/stb_image.cpp ${CMAKE_SOURCE_DIR}/vendor/stb_image/stb_truetype.cpp )

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
    ${CMAKE_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_SOURCE_DIR}/vendor/entt/src/entt
    ${CMAKE_SOURCE_DIR}/vendor/glad/include    
    ${CMAKE_SOURCE_DIR}/vendor/glm
    ${CMAKE_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_SOURCE_DIR}/vendor/stb
    ${CMAKE_SOURCE_DIR}/vendor/assimp/include
    ${CMAKE_SOURCE_DIR}/vendor/spdlog/include 
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/vendor/imgui/backends
    ${CMAKE_SOURCE_DIR}/vendor/stb_image
    ${CMAKE_SOURCE_DIR}/vendor/ImGuizmo
    ${CMAKE_SOURCE_DIR}/vendor/yaml-cpp
    ${CMAKE_SOURCE_DIR}/vendor/JoltPhysics
    ${CMAKE_SOURCE_DIR}/vendor/ImGuiFileDialog
    
)


# link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC  
  glfw    
  glad    
  glm
  spdlog
  imgui
  ImGuiFileDialog
  EnTT     
  assimp
  yaml-cpp
  Jolt
)

# copy data
if(EXISTS ${CMAKE_SOURCE_DIR}/Data)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND 
        ${CMAKE_COMMAND} -E copy_directory 
        ${CMAKE_SOURCE_DIR}/Data
        ${EXECUTABLE_OUTPUT_PATH}/Data
    )
else()
    message(WARNING "[WARNING] no data directory!")
endif()
