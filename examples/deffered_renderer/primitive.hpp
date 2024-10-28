#pragma once

#include <glm/vec3.hpp>
#include <vurl/buffer.hpp>
#include <vurl/render_graph.hpp>
#include <vector>
#include "material.hpp"


struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

class Primitive {
public:
    Primitive(std::shared_ptr<Vurl::RenderGraph> graph);
    ~Primitive();

    void SetVertices(std::vector<Vertex>& vertices);
    void SetIndices(std::vector<uint32_t>& indices);

    inline std::shared_ptr<Vurl::Resource<Vurl::Buffer>> GetVertexBuffer() const { return vertexBuffer; }
    inline std::shared_ptr<Vurl::Resource<Vurl::Buffer>> GetIndexBuffer() const { return indexBuffer; }

    inline Material& GetMaterial() { return material; }
    
private:
    std::shared_ptr<Vurl::RenderGraph> graph = nullptr;
    std::shared_ptr<Vurl::Resource<Vurl::Buffer>> vertexBuffer = nullptr;
    std::shared_ptr<Vurl::Resource<Vurl::Buffer>> indexBuffer = nullptr;
    Material material;
};