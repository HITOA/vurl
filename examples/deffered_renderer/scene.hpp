#pragma once

#include <vurl/render_graph.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <memory>

#include "node.hpp"
#include "mesh.hpp"
#include "primitive.hpp"

struct MeshPushConstant {
    glm::mat4x4 mvp;
    glm::mat4x4 projection;
    glm::mat4x4 view;
    glm::mat4x4 model;
};

class Scene {
public:
    Scene(std::shared_ptr<Vurl::RenderingContext> context, std::shared_ptr<Vurl::Surface> surface) : context{ context }, surface{ surface } {}
    ~Scene() = default;

    void Initialize();
    void Uninitialize();
    void Draw();

    void AddNode(std::shared_ptr<Node> node);

    inline std::shared_ptr<Vurl::RenderGraph> GetGraph() const { return graph; }

    inline glm::vec3 GetCameraPosition() const { return position; }
    inline void SetCameraPosition(glm::vec3 position) { this->position = position; }

    inline glm::vec3 GetCameraDirection() const { return direction; }
    inline void SetCameraDirection(glm::vec3 direction) { this->direction = direction; }

private:
    void ProcessNode(std::shared_ptr<Node> node);
    void GPassRenderingCallback(VkCommandBuffer commandBuffer, uint32_t frameIndex);
    void LightingPassRenderingCallback(VkCommandBuffer commandBuffer, uint32_t frameIndex);

private:
    std::shared_ptr<Vurl::RenderingContext> context = nullptr;
    std::shared_ptr<Vurl::Surface> surface = nullptr;
    std::shared_ptr<Vurl::RenderGraph> graph = nullptr;
    std::vector<std::shared_ptr<Node>> nodes{};
    std::vector<std::shared_ptr<Primitive>> primitives{};
    
    //Pipeline
    std::shared_ptr<Vurl::GraphicsPipeline> gPassPipeline = nullptr;
    std::shared_ptr<Vurl::GraphicsPipeline> lightingPassPipeline = nullptr;

    //Camera Data
    glm::vec3 position = glm::vec3( 0.0f, 0.0f, 0.0f );
    glm::vec3 direction = glm::vec3( 0.0f, 0.0f, 1.0f );
    glm::mat4x4 projection;
    glm::mat4x4 view;
};