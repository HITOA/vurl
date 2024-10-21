#include <vurl/graphics_pass.hpp>
#include <vurl/render_graph.hpp>


Vurl::GraphicsPass::GraphicsPass(const std::string& name, std::shared_ptr<GraphicsPipeline> pipeline, RenderGraph* graph) : 
        name{ name }, graphicsPipeline{ pipeline }, graph{ graph } {

}

void Vurl::GraphicsPass::AddColorAttachment(std::shared_ptr<Resource<Texture>> texture) {
    TextureHandle handle = graph->GetTextureHandle(texture);
    if (handle == VURL_NULL_HANDLE)
        return;
    colorAttachments.push_back(handle);
}

void Vurl::GraphicsPass::AddInputAttachment(std::shared_ptr<Resource<Texture>> texture) {
    TextureHandle handle = graph->GetTextureHandle(texture);
    if (handle == VURL_NULL_HANDLE)
        return;
    inputAttachments.push_back(handle);
}

void Vurl::GraphicsPass::SetClearAttachment(std::shared_ptr<Resource<Texture>> texture) {
    TextureHandle handle = graph->GetTextureHandle(texture);
    if (handle == VURL_NULL_HANDLE)
        return;
    clearAttachment = handle;
}