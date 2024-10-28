#include "application.hpp"
#include "scene.hpp"
#include "node.hpp"
#include "tiny_gltf.h"

#include <iostream>

//Ugly function
std::shared_ptr<Primitive> CreatePrimitive(std::shared_ptr<Scene> scene, tinygltf::Model& model, tinygltf::Primitive& gltfPrimitive) {
    std::shared_ptr<Primitive> primitive = std::make_shared<Primitive>(scene->GetGraph());

    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};

    if (!gltfPrimitive.attributes.count("POSITION") || !gltfPrimitive.attributes.count("NORMAL"))
        return nullptr;

    tinygltf::Accessor& indexAccessor = model.accessors[gltfPrimitive.indices];//indexAccessor.bufferView
    
    if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
        indices.resize(indexBufferView.byteLength / sizeof(uint16_t));
        uint16_t* ptr = (uint16_t*)(&model.buffers[indexBufferView.buffer].data.at(0) + indexBufferView.byteOffset);
        for (size_t i = 0; i < indexBufferView.byteLength / sizeof(uint16_t); ++i) {
            uint32_t v = ptr[i];
            indices[i] = v;
        }
    } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
        tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
        indices.resize(indexBufferView.byteLength / sizeof(uint32_t));
        memcpy(indices.data(), &model.buffers[indexBufferView.buffer].data.at(0) + indexBufferView.byteOffset, indexBufferView.byteLength);
    } else {
        return nullptr;
    }


    tinygltf::Accessor& positionAccessor = model.accessors[gltfPrimitive.attributes["POSITION"]];
    tinygltf::Accessor& normalAccessor = model.accessors[gltfPrimitive.attributes["NORMAL"]];

    if (positionAccessor.count != normalAccessor.count)
        return nullptr;
    if (positionAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || positionAccessor.type != TINYGLTF_TYPE_VEC3)
        return nullptr;
    if (normalAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || normalAccessor.type != TINYGLTF_TYPE_VEC3)
        return nullptr;
    
    vertices.resize(positionAccessor.count);

    tinygltf::BufferView& positionBufferView = model.bufferViews[positionAccessor.bufferView];
    tinygltf::BufferView& normalBufferView = model.bufferViews[normalAccessor.bufferView];
    glm::vec3* positionPtr = (glm::vec3*)(&model.buffers[positionBufferView.buffer].data.at(0) + positionBufferView.byteOffset);
    glm::vec3* normalPtr = (glm::vec3*)(&model.buffers[normalBufferView.buffer].data.at(0) + normalBufferView.byteOffset);

    for (size_t i = 0; i < positionAccessor.count; ++i) {
        vertices[i].position = positionPtr[i];
        vertices[i].position.y *= -1;
        vertices[i].normal = normalPtr[i];
    }
    
    primitive->SetVertices(vertices);
    primitive->SetIndices(indices);

    return primitive;
}

std::shared_ptr<Node> CreateNode(std::shared_ptr<Scene> scene, tinygltf::Model& model, tinygltf::Node& gltfNode) {
    std::shared_ptr<Node> node = std::make_shared<Node>();

    if (gltfNode.mesh >= 0 && gltfNode.mesh < model.meshes.size()) {
        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
        tinygltf::Mesh& gltfMesh = model.meshes[gltfNode.mesh];

        for (size_t i = 0; i < gltfMesh.primitives.size(); ++i)
            mesh->AddPrimitive(CreatePrimitive(scene, model, gltfMesh.primitives[i]));
        
        node->SetMesh(mesh);
    }

    for (size_t i = 0; i < gltfNode.children.size(); ++i) {
        std::shared_ptr<Node> childNode = CreateNode(scene, model, model.nodes[gltfNode.children[i]]);
        if (childNode)
            node->AddChild(childNode);
    }

    return node;
}

void LoadScene(const char* path, std::shared_ptr<Scene> scene) {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    tinygltf::Model model;

    bool res = loader.LoadBinaryFromFile(&model, &err, &warn, path);

    if (!warn.empty())
        std::cout << warn << std::endl;
    if (!err.empty())
        std::cerr << err << std::endl;
    if (!res)
        return;

    const tinygltf::Scene& gltfScene = model.scenes[model.defaultScene];
    for (size_t i = 0; i < gltfScene.nodes.size(); ++i) {
        std::shared_ptr<Node> node = CreateNode(scene, model, model.nodes[gltfScene.nodes[i]]);
        scene->AddNode(node);
    }
}

int main(int argc, const char* argv[]) {
    Application application{};
    application.Initialize();

    if (argc > 1)
        LoadScene(argv[1], application.GetScene());

    application.Run();
    application.Uninitialize();

    return 0;
}