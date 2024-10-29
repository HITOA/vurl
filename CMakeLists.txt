cmake_minimum_required(VERSION 3.16)
project(vurl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(VURL_BUILD_WSI_WIN32 "Build window system integration for win32 window." OFF)
option(VURL_BUILD_WSI_X11 "Build window system integration for xlib (x11) window." OFF)
option(VURL_BUILD_WSI_WAYLAND "Build window system integration for wayland window." OFF)

add_library(vurl STATIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics_pass.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics_pipeline.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/render_graph.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/rendering_context.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/shader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/surface.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/vma.cpp
)

if(VURL_BUILD_WSI_WIN32)
  target_sources(vurl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/wsi/win32.cpp)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()

if(VURL_BUILD_WSI_X11)
  target_sources(vurl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/wsi/x11.cpp)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
endif()

if(VURL_BUILD_WSI_WAYLAND)
  target_sources(vurl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/wsi/wayland.cpp)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WAYLAND_KHR)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty EXCLUDE_FROM_ALL)

target_link_libraries(vurl PUBLIC volk GPUOpen::VulkanMemoryAllocator spirv-reflect-static fossilize)
target_include_directories(vurl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples)