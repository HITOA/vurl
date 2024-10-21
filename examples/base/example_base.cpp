#include "example_base.hpp"

#include <iostream>
#include <fstream>

#ifdef VURL_EXAMPLE_FOR_WIN32
    #include <vurl/wsi/win32.hpp>
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
#elif VURL_EXAMPLE_FOR_WAYLAND
    #include <vurl/wsi/wayland.hpp>
    #define GLFW_EXPOSE_NATIVE_WAYLAND
    #include <GLFW/glfw3native.h>
#elif VURL_EXAMPLE_FOR_X11
    #include <vurl/wsi/x11.hpp>
    #define GLFW_EXPOSE_NATIVE_X11
    #include <GLFW/glfw3native.h>
#endif

void GflwErrorCallback(int code, const char* msg) {
    std::cerr << msg << std::endl;
}

ExampleBase::ExampleBase() {
    if (volkInitialize() != VK_SUCCESS) {
        std::cout << "Could not initialize volk/vulkan." << std::endl;
        return;
    }

    glfwSetErrorCallback(GflwErrorCallback);

    if (glfwInit() != GLFW_TRUE)
        return;
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(800, 600, "vurl example", nullptr, nullptr);
    if (window == NULL)
        return;
}

ExampleBase::~ExampleBase() {
    Destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
}

std::shared_ptr<Vurl::WSI> ExampleBase::CreateWSI() {
#ifdef VURL_EXAMPLE_FOR_WIN32
    //TODO
    std::shared_ptr<Vurl::WSIWin32> wsi = std::make_shared<VURL::WSIWin32>();
#elif VURL_EXAMPLE_FOR_WAYLAND
    std::shared_ptr<Vurl::WSIWayland> wsi = std::make_shared<Vurl::WSIWayland>();
    wsi->SetWaylandDisplay(glfwGetWaylandDisplay());
    wsi->SetWaylandSurface(glfwGetWaylandWindow(window));
#elif VURL_EXAMPLE_FOR_X11
    //TODO
    std::shared_ptr<Vurl::WSIX11> wsi = std::make_shared<Vurl::WSIX11>();
#else
    static_assert(false, "Missing WSI to present example to surface.");
#endif

    return wsi;
}

std::shared_ptr<Vurl::Shader> ExampleBase::CreateShaderFromFile(const std::string& path, VkDevice device) {
    std::vector<char> buffer{};
    std::ifstream file{ path, std::ios::ate | std::ios::binary };
    if (!file.is_open())
        return nullptr;

    size_t size = (size_t)file.tellg();
    file.seekg(0);
    buffer.resize(size);
    file.read(buffer.data(), size);
    file.close();

    std::shared_ptr<Vurl::Shader> shader = std::make_shared<Vurl::Shader>(device);
    shader->CreateShaderModule((const uint32_t*)buffer.data(), buffer.size());

    return shader;
}

void ExampleBase::Run() {
    Initialize();

    while (!glfwWindowShouldClose(window)) {
        Update();
        glfwPollEvents();
    }
}