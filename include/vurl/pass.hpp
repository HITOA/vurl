#pragma once

#include <string>

namespace Vurl {
    class RenderGraph;
    
    enum class PassType {
        Graphics,
        Compute,
        RayTracing
    };

    class Pass {
    public:
        Pass() = delete;
        Pass(const std::string& name, RenderGraph* graph) : name{ name }, graph{ graph } {};
        virtual ~Pass() = default;

        virtual PassType GetPassType() const = 0;

        inline const std::string& GetName() const { return name; }

    protected:
        std::string name{};
        RenderGraph* graph = nullptr;
    };
}