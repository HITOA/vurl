#pragma once

namespace Vurl {
    enum class PassType {
        Graphics,
        Compute,
        RayTracing
    };

    class Pass {
    public:
        virtual PassType GetPassType() const = 0;
    };
}