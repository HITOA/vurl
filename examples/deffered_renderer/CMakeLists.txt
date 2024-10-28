project(vurl_example_deffered_renderer)

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/primitive.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny_gltf.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE vurl glfw glm::glm)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory 
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory 
        ${CMAKE_CURRENT_SOURCE_DIR}/scenes $<TARGET_FILE_DIR:${PROJECT_NAME}>/scenes)