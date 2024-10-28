#pragma once

#include "primitive.hpp"
#include <vector>


class Mesh {
public:
    inline uint32_t GetPrimitiveCount() const { return primitives.size(); }
    inline std::shared_ptr<Primitive> GetPrimitive(uint32_t idx) const { return primitives[idx]; }
    inline void AddPrimitive(std::shared_ptr<Primitive> primitive) { primitives.push_back(primitive); }

private:
    std::vector<std::shared_ptr<Primitive>> primitives{};
};