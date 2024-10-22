#include <vurl/render_graph.hpp>
#include <iostream>
#include <numeric>


Vurl::RenderGraph::RenderGraph(std::shared_ptr<RenderingContext> context) : context{ context } {
    
}

Vurl::RenderGraph::~RenderGraph() {

}

void Vurl::RenderGraph::SetSurface(std::shared_ptr<Surface> surface) {
    this->surface = surface;
    AddExternalTexture(surface->GetBackBuffer());
    backBufferTexture = GetTextureHandle(surface->GetBackBuffer());
}

Vurl::BufferHandle Vurl::RenderGraph::GetBufferHandle(std::shared_ptr<Resource<Buffer>> buffer) {
    for (uint32_t i = 0; i < buffers.size(); ++i)
        if (buffers[i].get() == buffer.get())
            return (int)i;
    return VURL_NULL_HANDLE;
}

void Vurl::RenderGraph::AddExternalBuffer(std::shared_ptr<Resource<Buffer>> buffer) {
    if (GetBufferHandle(buffer) != VURL_NULL_HANDLE)
        return;
    buffers.push_back(buffer);
}

void Vurl::RenderGraph::CommitBuffer(std::shared_ptr<Resource<Buffer>> buffer, const uint8_t* initialData, uint32_t size) {
    if (buffer->IsTransient())
        return;
    
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;
    VkCommandBuffer transferTransientCommandBuffer = VK_NULL_HANDLE;
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;

    if (initialData != nullptr) {
        VkBufferCreateInfo stagingBufferCreateInfo{};
        stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingBufferCreateInfo.size = size;
        stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo stagingAllocCreateInfo{};
        stagingAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        stagingAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        vmaCreateBuffer(context->GetAllocator(), &stagingBufferCreateInfo, &stagingAllocCreateInfo, &stagingBuffer, &stagingBufferAllocation, nullptr);
        vmaCopyMemoryToAllocation(context->GetAllocator(), initialData, stagingBufferAllocation, 0, size);

        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandPool = transientCommandPool;
        commandBufferAllocateInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(context->GetDevice(), &commandBufferAllocateInfo, &transferTransientCommandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(transferTransientCommandBuffer, &beginInfo);
    }

    for (uint32_t i = 0; i < buffer->GetSliceCount(); ++i) {
        std::shared_ptr<Buffer> slice = buffer->GetResourceSlice(i);

        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = slice->size;
        bufferCreateInfo.usage = slice->usage;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        vmaCreateBuffer(context->GetAllocator(), &bufferCreateInfo, &allocCreateInfo, &slice->vkBuffer, &slice->allocation, nullptr);

        if (initialData != nullptr) {
            copyRegion.size = std::min((VkDeviceSize)slice->size, (VkDeviceSize)size);
            vkCmdCopyBuffer(transferTransientCommandBuffer, stagingBuffer, slice->vkBuffer, 1, &copyRegion);
        }
    }

    if (initialData != nullptr) {
        vkEndCommandBuffer(transferTransientCommandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &transferTransientCommandBuffer;

        vkQueueSubmit(context->GetQueueInfo().queues[QUEUE_INDEX_GRAPHICS], 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(context->GetQueueInfo().queues[QUEUE_INDEX_GRAPHICS]);

        vkFreeCommandBuffers(context->GetDevice(), transientCommandPool, 1, &transferTransientCommandBuffer);

        vmaDestroyBuffer(context->GetAllocator(), stagingBuffer, stagingBufferAllocation);
    }
}


Vurl::TextureHandle Vurl::RenderGraph::GetTextureHandle(std::shared_ptr<Resource<Texture>> texture) {
    for (uint32_t i = 0; i < textures.size(); ++i)
        if (textures[i].get() == texture.get())
            return (int)i;
    return VURL_NULL_HANDLE;
}

void Vurl::RenderGraph::AddExternalTexture(std::shared_ptr<Resource<Texture>> texture) {
    if (GetTextureHandle(texture) != VURL_NULL_HANDLE)
        return;
    textures.push_back(texture);
}

std::shared_ptr<Vurl::GraphicsPass> Vurl::RenderGraph::CreateGraphicsPass(const std::string& name, std::shared_ptr<GraphicsPipeline> pipeline) {
    std::shared_ptr<GraphicsPass> pass = std::make_shared<GraphicsPass>(name, pipeline, this);
    passes.push_back(pass);
    return pass;
}

void Vurl::RenderGraph::Build() {
    if (!BuildDirectedPassesGraph()) {
        complete = false;
        return;
    }

    complete = BuildGraphicsPassGroups() & BuildCommandBuffers() & BuildSynchronizationObjects();
}

void Vurl::RenderGraph::Destroy() {
    DestroyGraphicsPassGroups();
    DestroyCommandBuffers();
    DestroySynchronizationObjects();
}

void Vurl::RenderGraph::Execute() {
    if (!complete)
        return;
    
    uint32_t inFlightFrameIndex = frameIndex % VURL_MAX_FRAMES_IN_FLIGHT;

    vkWaitForFences(context->GetDevice(), 1, &inFlightFences[inFlightFrameIndex], VK_TRUE, UINT64_MAX);

    uint32_t swapchainImageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(context->GetDevice(), surface->GetSwapchainKHR(), 
            UINT64_MAX, availableSwapchainImageSemaphores[inFlightFrameIndex], VK_NULL_HANDLE, &swapchainImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
        return;
    
    vkResetCommandBuffer(primaryCommandBuffers[inFlightFrameIndex], 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(primaryCommandBuffers[inFlightFrameIndex], &beginInfo) != VK_SUCCESS)
        return;

    for (auto& group : graphicsPassGroups) {
        ExecuteGraphicsPassGroup(&group, primaryCommandBuffers[inFlightFrameIndex], swapchainImageIndex);
    }

    if (vkEndCommandBuffer(primaryCommandBuffers[inFlightFrameIndex]) != VK_SUCCESS)
        return;
    
    vkResetFences(context->GetDevice(), 1, &inFlightFences[inFlightFrameIndex]);
    
    VkSemaphore waitSemaphores[] = { availableSwapchainImageSemaphores[inFlightFrameIndex] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &primaryCommandBuffers[inFlightFrameIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphores[inFlightFrameIndex];

    if (vkQueueSubmit(context->GetQueueInfo().queues[QUEUE_INDEX_GRAPHICS], 1, &submitInfo, inFlightFences[inFlightFrameIndex]) != VK_SUCCESS)
        return;
    
    VkSwapchainKHR swapchain = surface->GetSwapchainKHR();

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[inFlightFrameIndex];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &swapchainImageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(context->GetQueueInfo().queues[QUEUE_INDEX_GRAPHICS], &presentInfo);

    ++frameIndex;
}

void Vurl::RenderGraph::CreatePipelineCache() {
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCacheCreateInfo.initialDataSize = 0;
    pipelineCacheCreateInfo.pInitialData = nullptr;

    vkCreatePipelineCache(context->GetDevice(), &pipelineCacheCreateInfo, nullptr, &pipelineCache);
}

void Vurl::RenderGraph::DestroyPipelineCache() {
    vkDestroyPipelineCache(context->GetDevice(), pipelineCache, nullptr);
}

void Vurl::RenderGraph::CreateTransientCommandPool() {
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolCreateInfo.queueFamilyIndex = context->GetQueueInfo().familyIndices[QUEUE_INDEX_GRAPHICS];

    vkCreateCommandPool(context->GetDevice(), &poolCreateInfo, nullptr, &transientCommandPool);
}

void Vurl::RenderGraph::DestroyTransientCommandPool() {
    vkDestroyCommandPool(context->GetDevice(), transientCommandPool, nullptr);
}

bool Vurl::RenderGraph::BuildDirectedPassesGraph() {
    std::vector<std::unordered_set<uint32_t>> dependencyGraph(passes.size());
    
    for (uint32_t i = 0; i < passes.size(); ++i) {
        if (passes[i]->GetPassType() == PassType::Graphics) {
            std::shared_ptr<GraphicsPass> graphicsPass = std::static_pointer_cast<GraphicsPass>(passes[i]);
            for (uint32_t j = 0; j < graphicsPass->GetColorAttachmentCount(); ++j) {
                TextureHandle h = graphicsPass->GetColorAttachment(j);
                std::shared_ptr<Resource<Texture>> attachment = textures[h];
                attachment->SetLastWriteOperationPassIndex(i);
            }

            for (uint32_t j = 0; j < graphicsPass->GetInputAttachmentCount(); ++j) {
                TextureHandle h = graphicsPass->GetInputAttachment(j);
                std::shared_ptr<Resource<Texture>> attachment = textures[h];
                attachment->SetLastReadOperationPassIndex(i);
            }
        }
    }

    for (int i = passes.size() - 1; i >= 0; --i) {
        if (passes[i]->GetPassType() == PassType::Graphics) {
            std::shared_ptr<GraphicsPass> graphicsPass = std::static_pointer_cast<GraphicsPass>(passes[i]);
            for (uint32_t j = 0; j < graphicsPass->GetColorAttachmentCount(); ++j) {
                TextureHandle h = graphicsPass->GetColorAttachment(j);
                std::shared_ptr<Resource<Texture>> attachment = textures[h];
                uint32_t passDependency = attachment->GetFirstReadOperationPassIndex();
                if (passDependency != -1)
                    dependencyGraph[passDependency].insert(i);
                attachment->SetFirstWriteOperationPassIndex(i);
            }

            for (uint32_t j = 0; j < graphicsPass->GetInputAttachmentCount(); ++j) {
                TextureHandle h = graphicsPass->GetInputAttachment(j);
                std::shared_ptr<Resource<Texture>> attachment = textures[h];
                attachment->SetFirstReadOperationPassIndex(i);
            }
        }
    }
    
    culledDirectedPassesGraph.clear();
    culledDirectedPassesGraph.resize(passes.size());
    std::vector<uint32_t> openPasses{};

    for (int i = passes.size() - 1; i >= 0; --i) {
        if (passes[i]->GetPassType() == PassType::Graphics) {
            std::shared_ptr<GraphicsPass> graphicsPass = std::static_pointer_cast<GraphicsPass>(passes[i]);
            for (uint32_t j = 0; j < graphicsPass->GetColorAttachmentCount(); ++j) {
                TextureHandle h = graphicsPass->GetColorAttachment(j);
                std::shared_ptr<Resource<Texture>> attachment = textures[h];
                if (attachment->IsExternal())
                    openPasses.push_back(i);
            }
        }
    }

    for (uint32_t i = 0; i < openPasses.size(); ++i) {
        uint32_t currentPassIndex = openPasses[i];

        if (dependencyGraph[currentPassIndex].size() == 0)
            beginPasses.push_back(currentPassIndex);

        for (uint32_t dependencyIndex : dependencyGraph[currentPassIndex]) {
            culledDirectedPassesGraph[dependencyIndex].push_back(currentPassIndex);
            openPasses.push_back(dependencyIndex);
        }
    }

    return true;
}

bool Vurl::RenderGraph::BuildGraphicsPassGroups() {
    graphicsPassGroups.clear();
    for (uint32_t beginPass : beginPasses) {
        if (passes[beginPass]->GetPassType() == PassType::Graphics)
            BuildGraphicsPassGroup(beginPass);
    }

    for (auto& group : graphicsPassGroups) {
        BuildGraphicsPassGroupRenderPass(&group);
        BuildGraphicsPassGroupFramebuffers(&group);
        BuildGraphicsPassGroupGraphicsPipelines(&group);
    }

    return true;
}

bool Vurl::RenderGraph::BuildGraphicsPassGroup(uint32_t firstPass) {
    //TODO: Actual Implementation
    GraphicsPassGroup& group = graphicsPassGroups.emplace_back();
    std::shared_ptr<GraphicsPass> graphicsPass = std::static_pointer_cast<GraphicsPass>(passes[firstPass]);
    group.passes.push_back(graphicsPass);
    for (uint32_t passIndex : culledDirectedPassesGraph[firstPass])
        if (passes[passIndex]->GetPassType() == PassType::Graphics)
            BuildGraphicsPassGroup(passIndex);
    return true;
}

bool Vurl::RenderGraph::BuildGraphicsPassGroupRenderPass(GraphicsPassGroup* group) {
    uint32_t i = 0;
    for (const auto& pass : group->passes) {
        for (uint32_t j = 0; j < pass->GetColorAttachmentCount(); ++j) {
            TextureHandle h = pass->GetColorAttachment(j);
            if (h == backBufferTexture)
                group->minSwapchainColorAttachmentSubpassIndex = std::min(group->minSwapchainColorAttachmentSubpassIndex, i);

            auto r = group->attachmentDescriptions.emplace(h, VkAttachmentDescription{});
            VkAttachmentDescription& description = r.first->second;

            if (r.second)
                description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }

        for (uint32_t j = 0; j < pass->GetInputAttachmentCount(); ++j) {
            TextureHandle h = pass->GetInputAttachment(j);
            auto r = group->attachmentDescriptions.emplace(h, VkAttachmentDescription{});
            VkAttachmentDescription& description = r.first->second;

            if (r.second || description.loadOp == VK_ATTACHMENT_LOAD_OP_DONT_CARE)
                description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        }

        if (pass->GetClearAttachment() != VURL_NULL_HANDLE) {
            if (group->attachmentDescriptions.count(pass->GetClearAttachment()) > 0) {
                group->attachmentDescriptions[pass->GetClearAttachment()].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            }
        }

        ++i;
    }

    std::vector<VkAttachmentDescription> vkAttachmentDescriptions{};
    std::unordered_map<TextureHandle, uint32_t> handleToAttachmentIndex{};

    uint32_t minWidth = UINT32_MAX;
    uint32_t minHeight = UINT32_MAX;

    i = 0;
    for (auto& e : group->attachmentDescriptions) {
        TextureHandle h = e.first;
        VkAttachmentDescription& description = e.second;

        std::shared_ptr<Resource<Texture>> texture = textures[h];

        std::shared_ptr<Texture> slice = texture->GetResourceSlice(0);

        minWidth = minWidth > slice->width ? slice->width : minWidth;
        minHeight = minHeight > slice->height ? slice->height : minHeight;

        description.format = slice->vkFormat;
        description.samples = VK_SAMPLE_COUNT_1_BIT;

        description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        description.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

        if (h == backBufferTexture)
            description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        vkAttachmentDescriptions.push_back(description);
        handleToAttachmentIndex[h] = i;
        ++i;
    }

    group->viewport.x + 0.0f;
    group->viewport.y + 0.0f;
    group->viewport.width = (float)minWidth;
    group->viewport.height = (float)minHeight;
    group->viewport.minDepth = 0.0f;
    group->viewport.maxDepth = 1.0f;

    group->scissor.offset = { 0, 0 };
    group->scissor.extent = { minWidth, minHeight };

    std::vector<std::vector<VkAttachmentReference>> subpassesAttachmentReferences{};
    std::vector<VkSubpassDependency> subpassDependencies{};
    std::vector<VkSubpassDescription> subpassDescriptions{};

    i = 0;
    for (const auto& pass : group->passes) {
        std::vector<VkAttachmentReference>& attachmentReferences = subpassesAttachmentReferences.emplace_back();
        for (uint32_t i = 0; i < pass->GetColorAttachmentCount(); ++i) {
            TextureHandle h = pass->GetColorAttachment(i);
            VkAttachmentReference attachmentReference{};
            attachmentReference.attachment = handleToAttachmentIndex[h];
            attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentReferences.push_back(attachmentReference);
        }

        for (uint32_t i = 0; i < pass->GetInputAttachmentCount(); ++i) {
            TextureHandle h = pass->GetInputAttachment(i);
            VkAttachmentReference attachmentReference{};
            attachmentReference.attachment = handleToAttachmentIndex[h];
            attachmentReference.layout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
            attachmentReferences.push_back(attachmentReference);
        }

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = pass->GetColorAttachmentCount();
        subpass.pColorAttachments = attachmentReferences.data();
        subpass.inputAttachmentCount = pass->GetInputAttachmentCount();
        subpass.pInputAttachments = attachmentReferences.data() + pass->GetColorAttachmentCount();

        if (group->minSwapchainColorAttachmentSubpassIndex == i) {
            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = i;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            subpassDependencies.push_back(dependency);
        }

        subpassDescriptions.push_back(subpass);
        ++i;
    }

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = (uint32_t)vkAttachmentDescriptions.size();
    renderPassCreateInfo.pAttachments = vkAttachmentDescriptions.data();
    renderPassCreateInfo.subpassCount = (uint32_t)subpassDescriptions.size();
    renderPassCreateInfo.pSubpasses = subpassDescriptions.data();
    renderPassCreateInfo.dependencyCount = (uint32_t)subpassDependencies.size();
    renderPassCreateInfo.pDependencies = subpassDependencies.data();
    
    return vkCreateRenderPass(context->GetDevice(), &renderPassCreateInfo, nullptr, &group->vkRenderPass) == VK_SUCCESS;
}

bool Vurl::RenderGraph::BuildGraphicsPassGroupFramebuffers(GraphicsPassGroup* group) {
    std::unordered_map<TextureHandle, uint32_t> handleToAttachmentIndex{};

    uint32_t minWidth = UINT32_MAX;
    uint32_t minHeight = UINT32_MAX;

    uint32_t attachmentSlicesLCM = 1;

    uint32_t i = 0;
    for (auto& e : group->attachmentDescriptions) {
        TextureHandle h = e.first;
        VkAttachmentDescription& description = e.second;

        std::shared_ptr<Resource<Texture>> texture = textures[h];

        attachmentSlicesLCM = std::lcm(attachmentSlicesLCM, texture->GetSliceCount());

        std::shared_ptr<Texture> slice = texture->GetResourceSlice(0);

        minWidth = minWidth > slice->width ? slice->width : minWidth;
        minHeight = minHeight > slice->height ? slice->height : minHeight;

        handleToAttachmentIndex[h] = i;

        ++i;
    }

    std::vector<VkImageView> framebufferAttachments(group->attachmentDescriptions.size());
    group->framebuffers.resize(attachmentSlicesLCM);

    for (uint32_t i = 0; i < attachmentSlicesLCM; ++i) {
        for (auto& e : handleToAttachmentIndex) {
            TextureHandle h = e.first;
            uint32_t idx = e.second;

            std::shared_ptr<Resource<Texture>> texture = textures[h];
            framebufferAttachments[idx] = texture->GetResourceSlice(i)->vkImageView;
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = group->vkRenderPass;
        framebufferInfo.attachmentCount = (uint32_t)framebufferAttachments.size();
        framebufferInfo.pAttachments = framebufferAttachments.data();
        framebufferInfo.width = minWidth;
        framebufferInfo.height = minHeight;
        framebufferInfo.layers = 1;

        vkCreateFramebuffer(context->GetDevice(), &framebufferInfo, nullptr, &group->framebuffers[i]);
    }
    return true;
}

bool Vurl::RenderGraph::BuildGraphicsPassGroupGraphicsPipelines(GraphicsPassGroup* group) {
    struct GraphicsPipelineCreateInfo {
        VkPipelineDynamicStateCreateInfo dynamicState{};
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        VkPipelineViewportStateCreateInfo viewportState{};
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        VkPipelineMultisampleStateCreateInfo multisampling{};
        std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates{};
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        VkPipelineLayout pipelineLayout{};
        std::vector<VkPipelineShaderStageCreateInfo> stages{};
    };

    std::vector<GraphicsPipelineCreateInfo> graphicsPipelineCreateInfos{};
    std::vector<VkGraphicsPipelineCreateInfo> vkGraphicsPipelineCreateInfo{};

    uint32_t i = 0;
    for (const auto& pass : group->passes) {
        GraphicsPipelineCreateInfo& graphicsPipelineCreateInfo = graphicsPipelineCreateInfos.emplace_back();
        std::shared_ptr<GraphicsPipeline> graphicsPipeline = pass->GetGraphicsPipeline();

        graphicsPipelineCreateInfo.dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        graphicsPipelineCreateInfo.dynamicState.dynamicStateCount = graphicsPipeline->GetDynamicStatesCount();
        graphicsPipelineCreateInfo.dynamicState.pDynamicStates = graphicsPipeline->GetDynamicStates();

        graphicsPipelineCreateInfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        graphicsPipelineCreateInfo.vertexInputInfo.vertexBindingDescriptionCount = 0;
        graphicsPipelineCreateInfo.vertexInputInfo.pVertexBindingDescriptions = nullptr;
        graphicsPipelineCreateInfo.vertexInputInfo.vertexAttributeDescriptionCount = 0;
        graphicsPipelineCreateInfo.vertexInputInfo.pVertexAttributeDescriptions = nullptr;

        graphicsPipelineCreateInfo.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        graphicsPipelineCreateInfo.inputAssembly.topology = graphicsPipeline->GetPipelinePrimitiveTopology();
        graphicsPipelineCreateInfo.inputAssembly.primitiveRestartEnable = VK_FALSE;

        graphicsPipelineCreateInfo.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        graphicsPipelineCreateInfo.viewportState.viewportCount = 1;
        graphicsPipelineCreateInfo.viewportState.pViewports = &group->viewport;
        graphicsPipelineCreateInfo.viewportState.scissorCount = 1;
        graphicsPipelineCreateInfo.viewportState.pScissors = &group->scissor;

        graphicsPipelineCreateInfo.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        graphicsPipelineCreateInfo.rasterizer.depthClampEnable = VK_FALSE;
        graphicsPipelineCreateInfo.rasterizer.rasterizerDiscardEnable = VK_FALSE;
        graphicsPipelineCreateInfo.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        graphicsPipelineCreateInfo.rasterizer.lineWidth = 1.0f;
        graphicsPipelineCreateInfo.rasterizer.cullMode = graphicsPipeline->GetPipelineCullMode();
        graphicsPipelineCreateInfo.rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        graphicsPipelineCreateInfo.rasterizer.depthBiasEnable = VK_FALSE;
        graphicsPipelineCreateInfo.rasterizer.depthBiasConstantFactor = 0.0f;
        graphicsPipelineCreateInfo.rasterizer.depthBiasClamp = 0.0f;
        graphicsPipelineCreateInfo.rasterizer.depthBiasSlopeFactor = 0.0f;

        graphicsPipelineCreateInfo.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        graphicsPipelineCreateInfo.multisampling.sampleShadingEnable = VK_FALSE;
        graphicsPipelineCreateInfo.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        graphicsPipelineCreateInfo.multisampling.minSampleShading = 1.0f;
        graphicsPipelineCreateInfo.multisampling.pSampleMask = nullptr;
        graphicsPipelineCreateInfo.multisampling.alphaToCoverageEnable = VK_FALSE;
        graphicsPipelineCreateInfo.multisampling.alphaToOneEnable = VK_FALSE;

        for (uint32_t i = 0; i < pass->GetColorAttachmentCount(); ++i) {
            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            graphicsPipelineCreateInfo.blendAttachmentStates.push_back(colorBlendAttachment);
        }

        graphicsPipelineCreateInfo.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        graphicsPipelineCreateInfo.colorBlending.logicOpEnable = VK_FALSE;
        graphicsPipelineCreateInfo.colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        graphicsPipelineCreateInfo.colorBlending.attachmentCount = (uint32_t)graphicsPipelineCreateInfo.blendAttachmentStates.size();
        graphicsPipelineCreateInfo.colorBlending.pAttachments = graphicsPipelineCreateInfo.blendAttachmentStates.data();
        graphicsPipelineCreateInfo.colorBlending.blendConstants[0] = 0.0f; // Optional
        graphicsPipelineCreateInfo.colorBlending.blendConstants[1] = 0.0f; // Optional
        graphicsPipelineCreateInfo.colorBlending.blendConstants[2] = 0.0f; // Optional
        graphicsPipelineCreateInfo.colorBlending.blendConstants[3] = 0.0f; // Optional

        std::shared_ptr<Shader> vertexShader = graphicsPipeline->GetVertexShader();
        std::shared_ptr<Shader> fragmentShader = graphicsPipeline->GetFragmentShader();

        VkPipelineShaderStageCreateInfo vertexStageCreateInfo{};
        vertexStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexStageCreateInfo.module = vertexShader->GetShaderModule();
        vertexStageCreateInfo.pName = vertexShader->GetEntryPointName();

        VkPipelineShaderStageCreateInfo fragmentStageCreateInfo{};
        fragmentStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentStageCreateInfo.module = fragmentShader->GetShaderModule();
        fragmentStageCreateInfo.pName = fragmentShader->GetEntryPointName();

        graphicsPipelineCreateInfo.stages.push_back(vertexStageCreateInfo);
        graphicsPipelineCreateInfo.stages.push_back(fragmentStageCreateInfo);

        graphicsPipelineCreateInfo.pipelineLayout = graphicsPipeline->GetPipelineLayout();

        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stageCount = (uint32_t)graphicsPipelineCreateInfo.stages.size();
        pipelineCreateInfo.pStages = graphicsPipelineCreateInfo.stages.data();
        pipelineCreateInfo.pVertexInputState = &graphicsPipelineCreateInfo.vertexInputInfo;
        pipelineCreateInfo.pInputAssemblyState = &graphicsPipelineCreateInfo.inputAssembly;
        pipelineCreateInfo.pViewportState = &graphicsPipelineCreateInfo.viewportState;
        pipelineCreateInfo.pRasterizationState = &graphicsPipelineCreateInfo.rasterizer;
        pipelineCreateInfo.pMultisampleState = &graphicsPipelineCreateInfo.multisampling;
        pipelineCreateInfo.pDepthStencilState = nullptr;
        pipelineCreateInfo.pColorBlendState = &graphicsPipelineCreateInfo.colorBlending;
        pipelineCreateInfo.pDynamicState = &graphicsPipelineCreateInfo.dynamicState;
        pipelineCreateInfo.layout = graphicsPipelineCreateInfo.pipelineLayout;
        pipelineCreateInfo.renderPass = group->vkRenderPass;
        pipelineCreateInfo.subpass = i;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineCreateInfo.basePipelineIndex = -1;

        vkGraphicsPipelineCreateInfo.push_back(pipelineCreateInfo);

        ++i;
    }

    group->pipelines.resize(vkGraphicsPipelineCreateInfo.size());
    return (vkCreateGraphicsPipelines(context->GetDevice(), pipelineCache, (uint32_t)vkGraphicsPipelineCreateInfo.size(), 
            vkGraphicsPipelineCreateInfo.data(), nullptr, group->pipelines.data()) == VK_SUCCESS);
}


bool Vurl::RenderGraph::BuildCommandBuffers() {
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCreateInfo.queueFamilyIndex = context->GetQueueInfo().familyIndices[QUEUE_INDEX_GRAPHICS];

    if (vkCreateCommandPool(context->GetDevice(), &poolCreateInfo, nullptr, &commandPool) != VK_SUCCESS)
        return false;

    VkCommandBufferAllocateInfo bufferAllocInfo{};
    bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    bufferAllocInfo.commandPool = commandPool;
    bufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    bufferAllocInfo.commandBufferCount = VURL_MAX_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(context->GetDevice(), &bufferAllocInfo, primaryCommandBuffers) != VK_SUCCESS)
        return false;

    return true;
}

bool Vurl::RenderGraph::BuildSynchronizationObjects() {
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo inFlightFenceCreateInfo{};
    inFlightFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    inFlightFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < VURL_MAX_FRAMES_IN_FLIGHT; ++i) {
        vkCreateSemaphore(context->GetDevice(), &semaphoreCreateInfo, nullptr, &availableSwapchainImageSemaphores[i]);
        vkCreateSemaphore(context->GetDevice(), &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]);
        vkCreateFence(context->GetDevice(), &inFlightFenceCreateInfo, nullptr, &inFlightFences[i]);
    }

    return true;
}

void Vurl::RenderGraph::DestroyGraphicsPassGroups() {
    for (auto& group : graphicsPassGroups) {
        for (uint32_t i = 0; i < group.pipelines.size(); ++i)
            vkDestroyPipeline(context->GetDevice(), group.pipelines[i], nullptr);
        for (uint32_t i = 0; i < group.framebuffers.size(); ++i)
            vkDestroyFramebuffer(context->GetDevice(), group.framebuffers[i], nullptr);
        vkDestroyRenderPass(context->GetDevice(), group.vkRenderPass, nullptr);
    }

    graphicsPassGroups.clear();
}

void Vurl::RenderGraph::DestroyCommandBuffers() {
    vkDestroyCommandPool(context->GetDevice(), commandPool, nullptr);
}

void Vurl::RenderGraph::DestroySynchronizationObjects() {
    for (uint32_t i = 0; i < VURL_MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(context->GetDevice(), availableSwapchainImageSemaphores[i], nullptr);
        vkDestroySemaphore(context->GetDevice(), renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(context->GetDevice(), inFlightFences[i], nullptr);
    }
}

bool Vurl::RenderGraph::ExecuteGraphicsPassGroup(GraphicsPassGroup* group, VkCommandBuffer commandBuffer, uint32_t swapchainImageIndex) {
    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = group->vkRenderPass;
    if (group->minSwapchainColorAttachmentSubpassIndex != -1)
        renderPassBeginInfo.framebuffer = group->framebuffers[swapchainImageIndex];
    else
        renderPassBeginInfo.framebuffer = group->framebuffers[frameIndex % group->framebuffers.size()];
    renderPassBeginInfo.renderArea.offset = { (int)group->viewport.x, (int)group->viewport.y };
    renderPassBeginInfo.renderArea.extent = { (uint32_t)group->viewport.width, (uint32_t)group->viewport.height };

    VkClearValue clearColor = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (uint32_t i = 0; i < group->passes.size(); ++i) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, group->pipelines[i]);
        group->passes[i]->GetRenderingCallback()(commandBuffer);
    }

    vkCmdEndRenderPass(commandBuffer);

    return true;
}