#pragma once

#include <vurl/vurl.hpp>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>

class ExampleBase {
public:
    ExampleBase();
    virtual ~ExampleBase();

    std::shared_ptr<Vurl::WSI> CreateWSI();
    std::shared_ptr<Vurl::Shader> CreateShaderFromFile(const std::string& path, VkDevice device);

    void Run();

    virtual void Initialize() = 0;
    virtual void Update() = 0;
    virtual void Destroy() {};

protected:
    GLFWwindow* window = nullptr;
};