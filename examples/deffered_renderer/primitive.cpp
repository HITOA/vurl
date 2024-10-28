#include "primitive.hpp"


Primitive::Primitive(std::shared_ptr<Vurl::RenderGraph> graph) : graph{ graph } {
    vertexBuffer = graph->CreateBuffer<Vurl::Resource<Vurl::Buffer>>("Vertex Buffer", false);
    indexBuffer = graph->CreateBuffer<Vurl::Resource<Vurl::Buffer>>("Index Buffer", false);

    std::shared_ptr<Vurl::Buffer> vertexBufferSlice = std::make_shared<Vurl::Buffer>();
    std::shared_ptr<Vurl::Buffer> indexBufferSlice = std::make_shared<Vurl::Buffer>();

    vertexBufferSlice->usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    indexBufferSlice->usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    vertexBuffer->SetSliceCount(1);
    indexBuffer->SetSliceCount(1);
    vertexBuffer->SetResourceSlice(vertexBufferSlice, 0);
    indexBuffer->SetResourceSlice(indexBufferSlice, 0);
}

Primitive::~Primitive() {

}

void Primitive::SetVertices(std::vector<Vertex>& vertices) {
    std::shared_ptr<Vurl::Buffer> vertexBufferSlice = vertexBuffer->GetResourceSlice(0);
    vertexBufferSlice->size = vertices.size() * sizeof(Vertex);

    graph->CommitBuffer(vertexBuffer, (const uint8_t*)vertices.data(), vertices.size() * sizeof(Vertex));
}

void Primitive::SetIndices(std::vector<uint32_t>& indices) {
    std::shared_ptr<Vurl::Buffer> indexBufferSlice = indexBuffer->GetResourceSlice(0);
    indexBufferSlice->size = indices.size() * sizeof(uint32_t);

    graph->CommitBuffer(indexBuffer, (const uint8_t*)indices.data(), indices.size() * sizeof(uint32_t));
}
