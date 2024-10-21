project(vurl_example_base)

add_library(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/example_base.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC vurl glfw)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)