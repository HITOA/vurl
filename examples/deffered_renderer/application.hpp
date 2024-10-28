#pragma once

#include <vurl/rendering_context.hpp>
#include <vurl/surface.hpp>
#include <vurl/render_graph.hpp>
#include <GLFW/glfw3.h>

#include "scene.hpp"


class Application {
public:
    Application() = default;
    ~Application() = default;

    bool Initialize();
    void Uninitialize();
    void Run();

    inline std::shared_ptr<Scene> GetScene() const { return scene; }

private:
    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos);

private:
    GLFWwindow* window = nullptr;
    std::shared_ptr<Vurl::RenderingContext> context = nullptr;
    std::shared_ptr<Vurl::Surface> surface = nullptr;
    std::shared_ptr<Scene> scene = nullptr;

    bool keyStats[GLFW_KEY_LAST];
    double xMouseDelta = 0.0;
    double yMouseDelta = 0.0;
};