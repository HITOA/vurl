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

void Vurl::GraphicsPass::SetDepthStencilAttachment(std::shared_ptr<Resource<Texture>> texture) {
    TextureHandle handle = graph->GetTextureHandle(texture);
    if (handle == VURL_NULL_HANDLE)
        return;
    depthStencilAttachment = handle;
}

void Vurl::GraphicsPass::ClearAttachment(uint32_t attachmentIdx, VkClearColorValue color) {
    if (colorAttachments.size() <= attachmentIdx)
        return;
    clearAttachmentInfo.emplace_back(attachmentIdx, color);
}   

void Vurl::GraphicsPass::AddBufferInput(std::shared_ptr<Resource<Buffer>> buffer) {
    BufferHandle handle = graph->GetBufferHandle(buffer);
    if (handle == VURL_NULL_HANDLE)
        return;
    inputBuffers.push_back(handle);
}