project(EbonyEditor;)

# gather all source files
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
add_executable(${PROJECT_NAME} ${sources} ${headers})

# export engine symbols
target_compile_definitions(${PROJECT_NAME} PUBLIC
  -DUE_IMPORT -DUE_ENABLE_ASSERTS
)

# includes directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include        
    ${CMAKE_SOURCE_DIR}/Engine/include
)

# link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC             
    UrbanEngine
)

# copy resources
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Resources)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND 
        ${CMAKE_COMMAND} -E copy_directory 
        ${CMAKE_CURRENT_SOURCE_DIR}/Resources
        ${EXECUTABLE_OUTPUT_PATH}/Resources
    )
else()
    message(WARNING "[WARNING] no resource directory!")
endif()

