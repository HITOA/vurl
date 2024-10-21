project(vurl_examples)

option(VURL_BUILD_EXAMPLES "Build vurl example." ON)

if (VURL_BUILD_EXAMPLES)

    if(VURL_BUILD_WSI_WIN32)
        add_compile_definitions(VURL_EXAMPLE_FOR_WIN32)
    elseif(VURL_BUILD_WSI_WAYLAND)
        add_compile_definitions(VURL_EXAMPLE_FOR_WAYLAND)
    elseif(VURL_BUILD_WSI_X11)
        add_compile_definitions(VURL_EXAMPLE_FOR_X11)
    endif()

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/glfw)

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/base)

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/triangle)
    
endif()