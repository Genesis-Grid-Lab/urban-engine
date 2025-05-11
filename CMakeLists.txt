project(UrbanEngine)

set(PHYSX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/physx)
set(PHYSX_LIB ${PHYSX_DIR}/libs)

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glad)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/entt)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/assimp)

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif(UNIX)

file(GLOB_RECURSE source ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_library(${PROJECT_NAME} SHARED ${source} ${headers} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image/stb_image.cpp ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image/stb_truetype.cpp )

# export engine symbols
target_compile_definitions(${PROJECT_NAME} PUBLIC
  -DUE_EXPORT
)

# target_precompile_headers(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/tbpch.h)

# includes directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include  
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Core  
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/entt/src/entt
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include    
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/assimp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image
    ${PHYSX_DIR}/include
)


# link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC  
  glfw    
  glad    
  glm
  spdlog
  imgui
  EnTT     
  assimp
  ${PHYSX_LIB}/PhysX_64.lib
  ${PHYSX_LIB}/PhysXCommon_64.lib
  ${PHYSX_LIB}/PhysXFoundation_64.lib
  ${PHYSX_LIB}/PhysXExtensions_static_64.lib
  ${PHYSX_LIB}/PhysXPvdSDK_static_64.lib
)

file(GLOB PHYSX_DLLS "${PHYSX_DIR}/dlls/*")
# copy dlls
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND 
    ${CMAKE_COMMAND} -E copy 
    ${PHYSX_DLLS}
    ${EXECUTABLE_OUTPUT_PATH}
)
