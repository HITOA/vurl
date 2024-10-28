#include "application.hpp"

#include <vurl/vulkan_header.hpp>
#include <iostream>
#include <cstring>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

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

bool Application::Initialize() {
    //Initialize Volk
    if (volkInitialize() != VK_SUCCESS) {
        std::cerr << "Could not initialize volk/vulkan." << std::endl;
        return false;
    }

    //Glfw error callback
    glfwSetErrorCallback(GflwErrorCallback);

    //Initialize and create glfw window
    if (glfwInit() != GLFW_TRUE)
        return false;
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(800, 600, "vurl example", nullptr, nullptr);
    if (window == NULL)
        return false;

    glfwSetWindowUserPointer(window, this);

    memset(keyStats, 0x0, sizeof(keyStats));

    glfwSetKeyCallback(window, Application::glfwKeyCallback);
    glfwSetCursorPosCallback(window, Application::glfwCursorPosCallback);

    //Create rendering context & vulkan instance
    context = std::make_shared<Vurl::RenderingContext>();

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    context->CreateInstance(nullptr, glfwExtensions, glfwExtensionCount, true);

    //Create window surface
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
    
    surface = std::make_shared<Vurl::Surface>(wsi);
    surface->CreateSurface(context->GetInstance());

    //Create logical device compatible with the given surface
    context->CreateDevice(surface->GetSurfaceKHR());

    //Create swapchain
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    surface->CreateSwapchain(context->GetPhysicalDevice(), context->GetDevice(), (uint32_t)width, (uint32_t)height);

    //Create Scene
    scene = std::make_shared<Scene>(context, surface);
    scene->Initialize();
    
    return true;
}

void Application::Uninitialize() {
    scene->Uninitialize();
    surface->DestroySwapchain();
    surface->DestroySurface();
    context->DestroyDevice();
    context->DestroyInstance();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::Run() {
    float speed = 1.0f;

    while (!glfwWindowShouldClose(window)) {
        glm::vec3 cameraDirection = scene->GetCameraDirection();

        cameraDirection = glm::rotate(cameraDirection, (float)xMouseDelta * -0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
        cameraDirection = glm::rotate(cameraDirection, (float)yMouseDelta * 0.01f, glm::cross(cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f)));

        scene->SetCameraDirection(cameraDirection);

        glm::vec3 direction = glm::vec3( 0.0f, 0.0f, 0.0f );
        float yTranslation = 0.0f;
        float speedFactor = 1.0f;
        
        if (keyStats[GLFW_KEY_W])
            direction += cameraDirection;

        if (keyStats[GLFW_KEY_S])
            direction -= cameraDirection;

        if (keyStats[GLFW_KEY_D])
            direction += glm::cross(cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f));
        
        if (keyStats[GLFW_KEY_A])
            direction -= glm::cross(cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f));

        if (keyStats[GLFW_KEY_SPACE])
            yTranslation -= 1;

        if (keyStats[GLFW_KEY_LEFT_CONTROL])
            yTranslation += 1;

        if (keyStats[GLFW_KEY_LEFT_SHIFT])
            speedFactor = 4.0f;
        
        if (glm::length(direction) != 0)
            direction = glm::normalize(direction);
        glm::vec3 position = scene->GetCameraPosition();
        position += direction * speed * speedFactor;
        position.y += yTranslation * speed * speedFactor;
        scene->SetCameraPosition(position);

        scene->Draw();
        xMouseDelta = 0;
        yMouseDelta = 0;
        glfwPollEvents();
    }
}

void Application::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Application* application = (Application*)glfwGetWindowUserPointer(window);

    if (action == GLFW_PRESS)
        application->keyStats[key] = true;
    else if (action == GLFW_RELEASE)
        application->keyStats[key] = false;
}

void Application::glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Application* application = (Application*)glfwGetWindowUserPointer(window);

    static double lastXPos = xpos;
    static double lastYPos = ypos;

    application->xMouseDelta = xpos - lastXPos;
    application->yMouseDelta = ypos - lastYPos;

    lastXPos = xpos;
    lastYPos = ypos;
}

