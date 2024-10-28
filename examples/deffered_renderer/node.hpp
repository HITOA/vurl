#pragma once

#include "mesh.hpp"


class Node {
public:
    inline void SetMesh(std::shared_ptr<Mesh> mesh) { this->mesh = mesh; }
    inline std::shared_ptr<Mesh> GetMesh() const { return mesh; }

    inline uint32_t GetChildCount() const { return children.size(); }
    inline std::shared_ptr<Node> GetChild(uint32_t idx) const { return children[idx]; }
    inline void AddChild(std::shared_ptr<Node> child) { children.push_back(child); }

private:
    std::shared_ptr<Mesh> mesh = nullptr;
    std::vector<std::shared_ptr<Node>> children{};
};