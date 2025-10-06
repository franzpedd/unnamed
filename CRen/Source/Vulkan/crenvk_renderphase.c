#include "crenvk_renderphase.h"

#include "cren_error.h"
#include "cren_context.h"
#include "crenvk_context.h"

#include <memm/memm.h>

#if defined (CREN_BUILD_WITH_VULKAN)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Default
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CREN_API VkResult crenvk_renderphase_default_create(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkFormat format, VkSampleCountFlagBits msaa, bool finalPhase, vkDefaultRenderphase* outPhase)
{
    if (device == VK_NULL_HANDLE || physicalDevice == VK_NULL_HANDLE || surface == VK_NULL_HANDLE || outPhase == NULL) return VK_ERROR_INVALID_EXTERNAL_HANDLE;

    VkResult res = VK_SUCCESS;
    outPhase->renderpass = (vkRenderpass*)malloc(sizeof(vkRenderpass));
    outPhase->renderpass->name = "Default";
    outPhase->renderpass->surfaceFormat = format;
    outPhase->renderpass->msaa = msaa;

    VkAttachmentDescription attachments[3] = { 0 };

    // color
    attachments[0].format = format;
    attachments[0].samples = outPhase->renderpass->msaa;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // depth
    attachments[1].format = crenvk_device_find_depth_format(physicalDevice);
    attachments[1].samples = outPhase->renderpass->msaa;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // resolve
    attachments[2].format = format;
    attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[2].finalLayout = finalPhase == 1 ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // finalLayout should be present src if it's the final renderpass used, normally an UI renderpass would have such layout but we can't be sure if that'll be the case

    // attachments references
    VkAttachmentReference references[3] = { 0 };

    references[0].attachment = 0;
    references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    references[1].attachment = 1;
    references[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    references[2].attachment = 2;
    references[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // subpass
    VkSubpassDescription subpass = { 0 };
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &references[0];
    subpass.pDepthStencilAttachment = &references[1];
    subpass.pResolveAttachments = &references[2];

    // subpass dependencies for layout transitions
    VkSubpassDependency dependencies[2] = { 0 };

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassCI = { 0 };
    renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCI.attachmentCount = 3u;
    renderPassCI.pAttachments = attachments;
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subpass;
    renderPassCI.dependencyCount = 2u;
    renderPassCI.pDependencies = dependencies;
    CREN_ASSERT(vkCreateRenderPass(device, &renderPassCI, NULL, &outPhase->renderpass->renderPass) == VK_SUCCESS, "Failed to create default renderphase's renderpass");

    // command pool and command buffers
    vkRenderpass* renderpass = outPhase->renderpass;
    vkQueueFamilyIndices indices = crenvk_device_find_queue_families(physicalDevice, surface);

    VkCommandPoolCreateInfo cmdPoolInfo = { 0 };
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = indices.graphicFamily;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    res = vkCreateCommandPool(device, &cmdPoolInfo, NULL, &renderpass->commandPool);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create default renderphase command pool");
        return res;
    }

    VkCommandBufferAllocateInfo cmdBufferAllocInfo = { 0 };
    cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferAllocInfo.commandPool = renderpass->commandPool;
    cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferAllocInfo.commandBufferCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
    res = vkAllocateCommandBuffers(device, &cmdBufferAllocInfo, renderpass->commandBuffers);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create default renderphase command buffers");
        return res;
    }

    return VK_SUCCESS;
}

CREN_API void crenvk_renderphase_default_destroy(vkDefaultRenderphase* renderphase, VkDevice device, bool destroyRenderpass, bool destroyPipeline)
{
    vkDeviceWaitIdle(device);

    if (destroyRenderpass) crenvk_pipeline_renderpass_release(device, renderphase->renderpass);
    if (destroyPipeline) crenvk_pipeline_destroy(device, renderphase->pipeline);

    vkDestroyImage(device, renderphase->colorImage, NULL);
    vkFreeMemory(device, renderphase->colorMemory, NULL);
    vkDestroyImageView(device, renderphase->colorView, NULL);

    vkDestroyImage(device, renderphase->depthImage, NULL);
    vkFreeMemory(device, renderphase->depthMemory, NULL);
    vkDestroyImageView(device, renderphase->depthView, NULL);
}

CREN_API VkResult crenvk_renderphase_default_create_framebuffers(vkDefaultRenderphase* phase, VkDevice device, VkPhysicalDevice physicalDevice, VkImageView* views, uint32_t viewsCount, VkExtent2D extent, VkFormat colorFormat)
{
    vkRenderpass* renderpass = phase->renderpass;
    VkFormat depthFormat = crenvk_device_find_depth_format(physicalDevice);

    VkResult res = VK_SUCCESS;

    res = crenvk_device_create_image
    (
        extent.width,
        extent.height,
        1,
        1,
        device,
        physicalDevice,
        &phase->colorImage,
        &phase->colorMemory,
        colorFormat,
        renderpass->msaa,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        0
    );
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create default renderphase color image");
        return res;
    }

    res = crenvk_device_create_image_view(device, phase->colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, VK_IMAGE_VIEW_TYPE_2D, NULL, &phase->colorView);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create default renderphase color image view");
        return res;
    }

    res = crenvk_device_create_image
    (
        extent.width,
        extent.height,
        1,
        1,
        device,
        physicalDevice,
        &phase->depthImage,
        &phase->depthMemory,
        depthFormat,
        renderpass->msaa,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        0
    );
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create default renderphase depth image");
        return res;
    }

    res = crenvk_device_create_image_view(device, phase->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, VK_IMAGE_VIEW_TYPE_2D, NULL, &phase->depthView);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create default renderphase depth image view");
        return res;
    }

    // framebuffers conrresponds with the image views, since these are swapchain image views, they must match
    renderpass->framebuffersCount = viewsCount;
    renderpass->framebuffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * viewsCount);
    if (!renderpass->framebuffers) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to allocate memory for default renderphase framebuffers");
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    for (uint32_t i = 0; i < renderpass->framebuffersCount; i++) {
        const VkImageView attachments[3] = { phase->colorView, phase->depthView, views[i] };
        VkFramebufferCreateInfo fbci = { 0 };
        fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbci.renderPass = renderpass->renderPass;
        fbci.attachmentCount = 3U;
        fbci.pAttachments = &attachments[0];
        fbci.width = extent.width;
        fbci.height = extent.height;
        fbci.layers = 1;

        res = vkCreateFramebuffer(device, &fbci, NULL, &renderpass->framebuffers[i]);
        if (res != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create default renderphase framebuffer");
            return res;
        }
    }

    return res;
}

CREN_API void crenvk_renderphase_default_recreate(vkDefaultRenderphase* phase, vkSwapchain* swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSampleCountFlagBits msaa, VkExtent2D extent, bool finalPhase, bool vsync)
{
    // must recreate some default phase objects
    vkDestroyImageView(device, phase->depthView, NULL);
    vkDestroyImage(device, phase->depthImage, NULL);
    vkFreeMemory(device, phase->depthMemory, NULL);

    vkDestroyImageView(device, phase->colorView, NULL);
    vkDestroyImage(device, phase->colorImage, NULL);
    vkFreeMemory(device, phase->colorMemory, NULL);

    for (uint32_t i = 0; i < phase->renderpass->framebuffersCount; i++) {
        vkDestroyFramebuffer(device, phase->renderpass->framebuffers[i], NULL);
    }
    free(phase->renderpass->framebuffers);

    // recreate swapchain
    crenvk_renderphase_default_create_framebuffers(phase, device, physicalDevice, swapchain->swapchainImageViews, swapchain->swapchainImageCount, extent, swapchain->swapchainFormat.format);
}

CREN_API void crenvk_renderphase_default_update(vkDefaultRenderphase* phase, void* context, void* backend, uint32_t currentFrame, uint32_t swapchainImageIndex, bool usingViewport, float timestep, CRenCallback_Render callback)
{
    CRenVulkanBackend* vkBackend = (CRenVulkanBackend*)backend;
    VkClearValue clearValues[2] = { 0 };
    const uint32_t clearValuesCount = 2;
    clearValues[0].color = (VkClearColorValue){ 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = (VkClearDepthStencilValue){ 1.0f, 0 };

    VkCommandBuffer cmdBuffer = phase->renderpass->commandBuffers[currentFrame];
    VkFramebuffer frameBuffer = phase->renderpass->framebuffers[swapchainImageIndex];
    VkRenderPass renderPass = phase->renderpass->renderPass;

    vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

    VkCommandBufferBeginInfo cmdBeginInfo = { 0 };
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = NULL;
    cmdBeginInfo.flags = 0;
    CREN_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo) == VK_SUCCESS, "Failed to begin default renderphase command buffer");

    VkRenderPassBeginInfo renderPassBeginInfo = { 0 };
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = frameBuffer;
    renderPassBeginInfo.renderArea.offset = (VkOffset2D){ 0, 0 };
    renderPassBeginInfo.renderArea.extent = vkBackend->swapchain.swapchainExtent;
    renderPassBeginInfo.clearValueCount = clearValuesCount;
    renderPassBeginInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // set frame commandbuffer viewport
    VkViewport viewport = { 0 };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vkBackend->swapchain.swapchainExtent.width;
    viewport.height = (float)vkBackend->swapchain.swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

    // set frame commandbuffer scissor
    VkRect2D scissor = { 0 };
    scissor.offset = (VkOffset2D){ 0, 0 };
    scissor.extent = vkBackend->swapchain.swapchainExtent;
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    // not using viewport as the final target, therefore draw the objects
    if (!usingViewport) {
        if (callback != NULL) {
            callback(context, CREN_RENDER_STAGE_DEFAULT, timestep);
        }
    }

    vkCmdEndRenderPass(cmdBuffer);

    // end command buffer
    CREN_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS, "Failed to end default renderphase command buffer");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Picking
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CREN_API VkResult crenvk_renderphase_picking_create(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, vkPickingRenderphase* outPhase)
{
    if (device == VK_NULL_HANDLE || physicalDevice == VK_NULL_HANDLE || surface == VK_NULL_HANDLE || !outPhase) return VK_ERROR_INITIALIZATION_FAILED;

    outPhase->renderpass = (vkRenderpass*)malloc(sizeof(vkRenderpass));
    outPhase->imageSize = 1 * 8; // (RED Channel) * 8 bits
    outPhase->surfaceFormat = VK_FORMAT_R32_UINT;
    outPhase->renderpass->name = "Picking";
    outPhase->renderpass->msaa = VK_SAMPLE_COUNT_1_BIT; // no msaa on picking
    outPhase->depthFormat = crenvk_device_find_depth_format(physicalDevice);

    // create render-pass
    VkAttachmentDescription attachments[2] = { 0 };
    // color
    attachments[0].format = outPhase->surfaceFormat;
    attachments[0].samples = outPhase->renderpass->msaa;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // depth
    attachments[1].format = outPhase->depthFormat;
    attachments[1].samples = outPhase->renderpass->msaa;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = { 0 };
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = { 0 };
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = { 0 };
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = NULL;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = NULL;
    subpassDescription.pResolveAttachments = NULL;

    VkSubpassDependency dependencies[2] = { 0 };
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassCI = { 0 };
    renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCI.attachmentCount = 2U;
    renderPassCI.pAttachments = attachments;
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subpassDescription;
    renderPassCI.dependencyCount = 2U;
    renderPassCI.pDependencies = dependencies;

    VkResult res = vkCreateRenderPass(device, &renderPassCI, NULL, &outPhase->renderpass->renderPass);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create picking renderphase renderpass");
        return res;
    }

    vkQueueFamilyIndices indices = crenvk_device_find_queue_families(physicalDevice, surface);
    VkCommandPoolCreateInfo cmdPoolInfo = { 0 };
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = indices.graphicFamily;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    res = vkCreateCommandPool(device, &cmdPoolInfo, NULL, &outPhase->renderpass->commandPool);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create picking renderphasse command pool");
        return res;
    }

    VkCommandBufferAllocateInfo cmdBufferAllocInfo = { 0 };
    cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferAllocInfo.commandPool = outPhase->renderpass->commandPool;
    cmdBufferAllocInfo.commandBufferCount = CREN_CONCURRENTLY_RENDERED_FRAMES;

    res = vkAllocateCommandBuffers(device, &cmdBufferAllocInfo, outPhase->renderpass->commandBuffers);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate picking renderphase command buffers");
        return res;
    }

    return res;
}

CREN_API void crenvk_renderphase_picking_destroy(vkPickingRenderphase* phase, VkDevice device, bool destroyRenderpass, bool destroyPipeline)
{
    vkDeviceWaitIdle(device);

    if (destroyRenderpass) crenvk_pipeline_renderpass_release(device, phase->renderpass);
    if (destroyPipeline) crenvk_pipeline_destroy(device, phase->pipeline);

    vkDestroyImageView(device, phase->depthView, NULL);
    vkDestroyImage(device, phase->depthImage, NULL);
    vkFreeMemory(device, phase->depthMemory, NULL);

    for (uint32_t i = 0; i < phase->colorImageCount; i++) {
        vkDestroyImageView(device, phase->colorView[i], NULL);
        vkDestroyImage(device, phase->colorImage[i], NULL);
        vkFreeMemory(device, phase->colorMemory[i], NULL);
    }
    free(phase->colorImage);
    free(phase->colorMemory);
    free(phase->colorView);
}

CREN_API VkResult crenvk_renderphase_picking_create_framebuffers(vkPickingRenderphase* phase, VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkImageView* views, uint32_t viewsCount, VkExtent2D extent)
{
    // depth image
    VkResult res = crenvk_device_create_image
    (
        extent.width,
        extent.height,
        1,
        1,
        device,
        physicalDevice,
        &phase->depthImage,
        &phase->depthMemory,
        phase->depthFormat,
        phase->renderpass->msaa,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        0
    );
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create picking framebufer depth image");
        return res;
    }

    res = crenvk_device_create_image_view(device, phase->depthImage, phase->depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, VK_IMAGE_VIEW_TYPE_2D, NULL, &phase->depthView);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create picking framebufer depth image view");
        return res;
    }

    phase->colorImageCount = viewsCount;
    phase->colorImage = (VkImage*)malloc(sizeof(VkImage) * viewsCount);
    CREN_ASSERT(phase->colorImage != NULL, "Failed to allocate color images for picking framebuffer");

    phase->colorMemory = (VkDeviceMemory*)malloc(sizeof(VkDeviceMemory) * viewsCount);
    CREN_ASSERT(phase->colorMemory != NULL, "Failed to allocate color memory for picking framebuffer");

    phase->colorView = (VkImageView*)malloc(sizeof(VkImageView) * viewsCount);
    CREN_ASSERT(phase->colorView != NULL, "Failed to allocate color images view for picking framebuffer");

    phase->renderpass->framebuffersCount = viewsCount;
    phase->renderpass->framebuffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * viewsCount);
    CREN_ASSERT(phase->renderpass->framebuffers != NULL, "Failed to allocate memory for the picking renderphase framebuffers");

    for (uint32_t i = 0; i < viewsCount; i++) {
        res = crenvk_device_create_image
        (
            extent.width,
            extent.height,
            1,
            1,
            device,
            physicalDevice,
            &phase->colorImage[i],
            &phase->colorMemory[i],
            phase->surfaceFormat,
            phase->renderpass->msaa,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, // for picking
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            0
        );
        if (res != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create picking framebuffer color image");
            return res;
        }

        crenvk_device_create_image_view(device, phase->colorImage[i], phase->surfaceFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, VK_IMAGE_VIEW_TYPE_2D, NULL, &phase->colorView[i]);

        VkCommandBuffer cmdBuffer = crenvk_device_begin_commandbuffer_singletime(device, phase->renderpass->commandPool);

        VkImageSubresourceRange subresourceRange = { 0 };
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;

        crenvk_device_insert_image_memory_barrier
        (
            cmdBuffer,
            phase->colorImage[i],
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, // must get from last render pass (undefined also works)
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // must set for next render pass
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            subresourceRange
        );

        crenvk_device_end_commandbuffer_singletime(device, phase->renderpass->commandPool, cmdBuffer, graphicsQueue);

        const VkImageView attachments[2] = { phase->colorView[i], phase->depthView };

        VkFramebufferCreateInfo framebufferCI = { 0 };
        framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCI.renderPass = phase->renderpass->renderPass;
        framebufferCI.attachmentCount = 2U;
        framebufferCI.pAttachments = attachments;
        framebufferCI.width = extent.width;
        framebufferCI.height = extent.height;
        framebufferCI.layers = 1;
        res = vkCreateFramebuffer(device, &framebufferCI, NULL, &phase->renderpass->framebuffers[i]);
        if (res != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create picking framebuffer");
            return res;
        }
    }

    return VK_SUCCESS;
}

CREN_API void crenvk_renderphase_picking_recreate(vkPickingRenderphase* phase, vkSwapchain* swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, VkExtent2D extent)
{
    CREN_LOG(CREN_LOG_SEVERITY_TRACE,"Recreating %s with extent %dx%d", "Picking", extent.width, extent.height);
    vkDestroyImage(device, phase->depthImage, NULL);
    vkFreeMemory(device, phase->depthMemory, NULL);
    vkDestroyImageView(device, phase->depthView, NULL);

    for (uint32_t i = 0; i < phase->colorImageCount; i++) {
        vkDestroyImage(device, phase->colorImage[i], NULL);
        vkFreeMemory(device, phase->colorMemory[i], NULL);
        vkDestroyImageView(device, phase->colorView[i], NULL);
    }
    free(phase->colorImage);
    free(phase->colorMemory);
    free(phase->colorView);

    for (uint32_t i = 0; i < phase->renderpass->framebuffersCount; i++) {
        vkDestroyFramebuffer(device, phase->renderpass->framebuffers[i], NULL);
    }
    free(phase->renderpass->framebuffers);

    crenvk_renderphase_picking_create_framebuffers(phase, device, physicalDevice, graphicsQueue, swapchain->swapchainImageViews, swapchain->swapchainImageCount, extent);
}

CREN_API void crenvk_renderphase_picking_update(vkPickingRenderphase* phase, void* context, void* backend, uint32_t currentFrame, uint32_t swapchainImageIndex, bool usingViewport, float timestep, CRenCallback_Render callback)
{
    CRenVulkanBackend* vkBackend = (CRenVulkanBackend*)backend;

    VkClearValue clearValues[2] = { 0 };
    clearValues[0].color = (VkClearColorValue){ 0.0f,  0.0f,  0.0f, 1.0f };
    clearValues[1].depthStencil = (VkClearDepthStencilValue){ 1.0f,0 };

    VkCommandBuffer cmdBuffer = phase->renderpass->commandBuffers[currentFrame];
    VkFramebuffer frameBuffer = phase->renderpass->framebuffers[swapchainImageIndex];
    VkRenderPass renderPass = phase->renderpass->renderPass;

    vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

    VkCommandBufferBeginInfo cmdBeginInfo = { 0 };
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = NULL;
    cmdBeginInfo.flags = 0;
    CREN_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo) == VK_SUCCESS, "Failed to beging picking renderphase command buffer");

    VkRenderPassBeginInfo renderPassBeginInfo = { 0 };
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = frameBuffer;
    renderPassBeginInfo.renderArea.offset = (VkOffset2D){ 0, 0 };
    renderPassBeginInfo.renderArea.extent = vkBackend->swapchain.swapchainExtent;
    renderPassBeginInfo.clearValueCount = (uint32_t)CREN_STATIC_ARRAY_SIZE(clearValues);
    renderPassBeginInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // set frame commandbuffer viewport
    VkViewport viewport = { 0 };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vkBackend->swapchain.swapchainExtent.width;
    viewport.height = (float)vkBackend->swapchain.swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

    // if a viewport is in use, we just want to render the pixel undeneath the mousepos. This skips all other fragment computation
    if (usingViewport) {

        // these must be previously treated to handle viewport
        float2 cursorPos = cren_get_mousepos((CRenContext*)context);
        float2 mousePos;
        mousePos.xy.x = f_clamp(cursorPos.xy.x, 0.0f, (float)vkBackend->swapchain.swapchainExtent.width);
        mousePos.xy.y = f_clamp(cursorPos.xy.y, 0.0f, (float)vkBackend->swapchain.swapchainExtent.height);

        VkRect2D scissor = { 0 };
        scissor.offset = (VkOffset2D){ (int)mousePos.xy.x, (int)mousePos.xy.y };
        scissor.extent = (VkExtent2D){ 1, 1 };
        vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    }

    else {
        VkRect2D scissor = { 0 };
        scissor.offset = (VkOffset2D){ 0, 0 };
        scissor.extent = vkBackend->swapchain.swapchainExtent;
        vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    }

    if (callback != NULL) {
        callback(context, (CRen_RenderStage)CREN_RENDER_STAGE_PICKING, timestep);
    }

    // end render pass
    vkCmdEndRenderPass(cmdBuffer);

    // end command buffer
    CREN_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS, "Failed to finish picking renderphase command buffer");

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UI
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CREN_API VkResult crenvk_renderphase_ui_create(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkFormat format, VkSampleCountFlagBits msaa, bool finalPhase, vkUIRenderphase* outPhase)
{
    if (!outPhase || device == VK_NULL_HANDLE) return VK_ERROR_INITIALIZATION_FAILED;

    outPhase->renderpass = (vkRenderpass*)malloc(sizeof(vkRenderpass));
    CREN_ASSERT(outPhase->renderpass != NULL, "Failed to allocate memory for the ui renderphase renderpass");

    outPhase->renderpass->name = "UI";
    outPhase->renderpass->surfaceFormat = format;
    outPhase->renderpass->msaa = msaa;

    VkAttachmentDescription attachment = { 0 };
    attachment.format = format;
    attachment.samples = outPhase->renderpass->msaa;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = finalPhase == 1 ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachment = { 0 };
    colorAttachment.attachment = 0;
    colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = { 0 };
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachment;

    VkSubpassDependency dependency = { 0 };
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = { 0 };
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;
    VkResult res = vkCreateRenderPass(device, &info, NULL, &outPhase->renderpass->renderPass);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create ui renderphase renderpass");
        return res;
    }

    // command pool and buffers
    vkQueueFamilyIndices indices = crenvk_device_find_queue_families(physicalDevice, surface);
    VkCommandPoolCreateInfo cmdPoolInfo = { 0 };
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = indices.graphicFamily;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    res = vkCreateCommandPool(device, &cmdPoolInfo, NULL, &outPhase->renderpass->commandPool);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create ui renderphase command pool");
        return res;
    }

    VkCommandBufferAllocateInfo cmdBufferAllocInfo = { 0 };
    cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferAllocInfo.commandPool = outPhase->renderpass->commandPool;
    cmdBufferAllocInfo.commandBufferCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
    res = vkAllocateCommandBuffers(device, &cmdBufferAllocInfo, outPhase->renderpass->commandBuffers);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to allocate command buffers for the ui renderphass");
        return res;
    }

    return VK_SUCCESS;
}

CREN_API void crenvk_renderphase_ui_destroy(vkUIRenderphase* phase, VkDevice device, bool destroyRenderpass)
{
    vkDeviceWaitIdle(device);

    if (destroyRenderpass) crenvk_pipeline_renderpass_release(device, phase->renderpass);
}

CREN_API VkResult crenvk_renderphase_ui_framebuffers_create(vkUIRenderphase* phase, VkDevice device, VkExtent2D extent, VkImageView* swapchainViews, uint32_t swapchainViewsCount)
{
    phase->renderpass->framebuffersCount = swapchainViewsCount;
    phase->renderpass->framebuffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * phase->renderpass->framebuffersCount);
    CREN_ASSERT(phase->renderpass->framebuffers != NULL, "Failed to allocate memory for the ui renderphase framebuffers");

    for (uint32_t i = 0; i < phase->renderpass->framebuffersCount; i++) {
        const VkImageView attachments[] = { swapchainViews[i] };

        VkFramebufferCreateInfo framebufferCI = { 0 };
        framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCI.renderPass = phase->renderpass->renderPass;
        framebufferCI.attachmentCount = 1U;
        framebufferCI.pAttachments = attachments;
        framebufferCI.width = extent.width;
        framebufferCI.height = extent.height;
        framebufferCI.layers = 1;

        VkResult res = vkCreateFramebuffer(device, &framebufferCI, NULL, &phase->renderpass->framebuffers[i]);
        if (res != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create ui renderphase framebuffer");
            return res;
        }
    }
    return VK_SUCCESS;
}

CREN_API void crenvk_renderphase_ui_recreate(vkUIRenderphase* phase, vkSwapchain* swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkExtent2D extent)
{
    for (uint32_t i = 0; i < phase->renderpass->framebuffersCount; i++) {
        vkDestroyFramebuffer(device, phase->renderpass->framebuffers[i], NULL);
    }
    free(phase->renderpass->framebuffers);
    crenvk_renderphase_ui_framebuffers_create(phase, device, extent, swapchain->swapchainImageViews, swapchain->swapchainImageCount);
}

CREN_API void crenvk_renderphase_ui_update(vkUIRenderphase* phase, void* context, void* backend, uint32_t currentFrame, uint32_t swapchainImageIndex, CRenCallback_DrawUIRawData callback)
{
    CRenVulkanBackend* vkBackend = (CRenVulkanBackend*)backend;

    VkCommandBuffer cmdBuffer = phase->renderpass->commandBuffers[currentFrame];
    VkFramebuffer frameBuffer = phase->renderpass->framebuffers[swapchainImageIndex];
    VkRenderPass renderPass = phase->renderpass->renderPass;

    vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

    VkCommandBufferBeginInfo cmdBeginInfo = { 0 };
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = NULL;
    cmdBeginInfo.flags = 0;
    CREN_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo) == VK_SUCCESS, "Failed to begin ui renderphase command buffer");

    VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

    VkRenderPassBeginInfo renderPassBeginInfo = { 0 };
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = frameBuffer;
    renderPassBeginInfo.renderArea.offset = (VkOffset2D){ 0, 0 };
    renderPassBeginInfo.renderArea.extent = vkBackend->swapchain.swapchainExtent;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearValue;
    vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // render raw data
    if (callback != NULL) {
        callback(context, cmdBuffer);
    }

    vkCmdEndRenderPass(cmdBuffer);

    CREN_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS, "Failed to end ui renderphase command buffer");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Viewport
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CREN_API VkResult crenvk_renderphase_viewport_create(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkFormat format, VkSampleCountFlagBits msaa, vkViewportRenderphase* outPhase)
{
    outPhase->renderpass = (vkRenderpass*)malloc(sizeof(vkRenderpass));
    CREN_ASSERT(outPhase->renderpass != NULL, "Could not allocate memory for renderpass");

    outPhase->renderpass->name = "Viewport";
    outPhase->renderpass->surfaceFormat = format;
    outPhase->renderpass->msaa = msaa;

    const uint32_t attachmentsSize = 2U;
    VkAttachmentDescription attachments[2] = { 0 };

    // color attachment
    attachments[0].format = outPhase->renderpass->surfaceFormat;
    attachments[0].samples = outPhase->renderpass->msaa;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // depth attachment
    attachments[1].format = crenvk_device_find_depth_format(physicalDevice);
    attachments[1].samples = outPhase->renderpass->msaa;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = { 0 };
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = { 0 };
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = { 0 };
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = NULL;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = NULL;
    subpassDescription.pResolveAttachments = NULL;

    // subpass dependencies for layout transitions
    const uint32_t dependenciesSize = 2U;
    VkSubpassDependency dependencies[2] = { 0 };

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassCI = { 0 };
    renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCI.attachmentCount = attachmentsSize;
    renderPassCI.pAttachments = attachments;
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subpassDescription;
    renderPassCI.dependencyCount = dependenciesSize;
    renderPassCI.pDependencies = dependencies;

    VkResult res = vkCreateRenderPass(device, &renderPassCI, NULL, &outPhase->renderpass->renderPass);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create vulkan renderpass for the viewport render phase");
        return res;
    }

    // command pool and buffers
    vkQueueFamilyIndices indices = crenvk_device_find_queue_families(physicalDevice, surface);
    VkCommandPoolCreateInfo cmdPoolInfo = { 0 };
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = indices.graphicFamily;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    res = vkCreateCommandPool(device, &cmdPoolInfo, NULL, &outPhase->renderpass->commandPool);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create viewport renderphase command pool");
        return res;
    }

    VkCommandBufferAllocateInfo cmdBufferAllocInfo = { 0 };
    cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferAllocInfo.commandPool = outPhase->renderpass->commandPool;
    cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferAllocInfo.commandBufferCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
    res = vkAllocateCommandBuffers(device, &cmdBufferAllocInfo, outPhase->renderpass->commandBuffers);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to allocate command buffers for the viewport renderphase");
        return res;
    }
    return VK_SUCCESS;
}

CREN_API void crenvk_renderphase_viewport_destroy(vkViewportRenderphase* phase, VkDevice device, bool destroyRenderpass)
{
    if (!phase) return;

    vkDeviceWaitIdle(device);
    if (destroyRenderpass) crenvk_pipeline_renderpass_release(device, phase->renderpass);

    vkDestroySampler(device, phase->sampler, NULL);
    vkDestroyDescriptorPool(device, phase->descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(device, phase->descriptorSetLayout, NULL);

    vkDestroyImageView(device, phase->depthView, NULL);
    vkDestroyImage(device, phase->depthImage, NULL);
    vkFreeMemory(device, phase->depthMemory, NULL);

    vkDestroyImageView(device, phase->colorView, NULL);
    vkDestroyImage(device, phase->colorImage, NULL);
    vkFreeMemory(device, phase->colorMemory, NULL);
}

CREN_API VkResult crenvk_renderphase_viewport_create_framebuffers(vkViewportRenderphase* phase, VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkImageView* swapchainViews, uint32_t swapchainViewCount, VkExtent2D extent)
{
    unsigned int imageCount = swapchainViewCount;
    VkFormat depthFormat = crenvk_device_find_depth_format(physicalDevice);

    // descriptor pool
    VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, CREN_CONCURRENTLY_RENDERED_FRAMES } };
    VkDescriptorPoolCreateInfo poolCI = { 0 };
    poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCI.pNext = NULL;
    poolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolCI.maxSets = (unsigned int)(2 * CREN_STATIC_ARRAY_SIZE(poolSizes));
    poolCI.poolSizeCount = (unsigned int)CREN_STATIC_ARRAY_SIZE(poolSizes);
    poolCI.pPoolSizes = poolSizes;
    VkResult res = vkCreateDescriptorPool(device, &poolCI, NULL, &phase->descriptorPool);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create viewport descriptor pool");
        return res;
    }

    // descriptor set layout
    VkDescriptorSetLayoutBinding binding[1] = { 0 };
    binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding[0].descriptorCount = 1;
    binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo info = { 0 };
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 1;
    info.pBindings = binding;
    res = vkCreateDescriptorSetLayout(device, &info, NULL, &phase->descriptorSetLayout);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create vulkan descriptor set layout for the viewport render phase");
        return res;
    }

    // sampler
    res = crenvk_device_create_image_sampler
    (
        device,
        physicalDevice,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        1.0f,
        &phase->sampler
    );
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create vulkan sampler for the viewport render phase");
        return res;
    }

    // color image
    res = crenvk_device_create_image
    (
        extent.width,
        extent.height,
        1,
        1,
        device,
        physicalDevice,
        &phase->colorImage,
        &phase->colorMemory,
        phase->renderpass->surfaceFormat,
        phase->renderpass->msaa,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, // last one is for picking
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        0
    );
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create the viewport renderphase color image");
        return res;
    }

    res = crenvk_device_create_image_view(device, phase->colorImage, phase->renderpass->surfaceFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, VK_IMAGE_VIEW_TYPE_2D, NULL, &phase->colorView);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create the viewport renderphase color image view");
        return res;
    }

    // depth buffer
    res = crenvk_device_create_image
    (
        extent.width,
        extent.height,
        1,
        1,
        device,
        physicalDevice,
        &phase->depthImage,
        &phase->depthMemory,
        depthFormat,
        phase->renderpass->msaa,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        0
    );
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create the viewport renderphase depth image");
        return res;
    }

    res = crenvk_device_create_image_view(device, phase->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, VK_IMAGE_VIEW_TYPE_2D, NULL, &phase->depthView);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create the viewport renderphase depth image view");
        return res;
    }

    // command buffer
    VkCommandBuffer command = crenvk_device_begin_commandbuffer_singletime(device, phase->renderpass->commandPool);

    VkImageSubresourceRange subresourceRange = { 0 };
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    crenvk_device_insert_image_memory_barrier
    (
        command,
        phase->colorImage,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, // must get from last render pass (undefined also works)
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // must set for next render pass, but that one could also use undefined
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        subresourceRange
    );

    crenvk_device_end_commandbuffer_singletime(device, phase->renderpass->commandPool, command, graphicsQueue);

    res = crenvk_device_create_image_descriptor_set(device, phase->descriptorPool, phase->descriptorSetLayout, phase->sampler, phase->colorView, &phase->descriptorSet);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create the viewport image descriptor set");
        return res;
    }

    // framebuffer
    phase->renderpass->framebuffersCount = swapchainViewCount;
    phase->renderpass->framebuffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * phase->renderpass->framebuffersCount);
    CREN_ASSERT(phase->renderpass->framebuffers != NULL, "Failed to allocate memory for the viewport renderphass framebuffers");

    for (size_t i = 0; i < imageCount; i++) {
        const VkImageView attachments[2] = { phase->colorView, phase->depthView };

        VkFramebufferCreateInfo framebufferCI = { 0 };
        framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCI.renderPass = phase->renderpass->renderPass;
        framebufferCI.attachmentCount = 2U;
        framebufferCI.pAttachments = attachments;
        framebufferCI.width = extent.width;
        framebufferCI.height = extent.height;
        framebufferCI.layers = 1;
        res = vkCreateFramebuffer(device, &framebufferCI, NULL, &phase->renderpass->framebuffers[i]);
        if (res != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create viewport renderphase framebuffer");
            return res;
        }
    }

    return VK_SUCCESS;
}

CREN_API void crenvk_renderphase_viewport_recreate(vkViewportRenderphase* phase, vkSwapchain* swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, VkExtent2D extent)
{
    crenvk_renderphase_viewport_destroy(phase, device, false);

    for (uint32_t i = 0; i < phase->renderpass->framebuffersCount; i++) {
        vkDestroyFramebuffer(device, phase->renderpass->framebuffers[i], NULL);
    }
    free(phase->renderpass->framebuffers);

    crenvk_renderphase_viewport_create_framebuffers(phase, device, physicalDevice, graphicsQueue, swapchain->swapchainImageViews, swapchain->swapchainImageCount, extent);
}

CREN_API void crenvk_renderphase_viewport_update(vkViewportRenderphase* phase, void* context, void* backend, uint32_t currentFrame, uint32_t swapchainImageIndex, bool usingViewport, float timestep, CRenCallback_Render callback)
{
    if (!usingViewport) return;

    CRenVulkanBackend* vkBackend = (CRenVulkanBackend*)backend;

    VkClearValue clearValues[2] = { 0 };
    clearValues[0].color = (VkClearColorValue){ 0.0f,  0.0f,  0.0f, 1.0f };
    clearValues[1].depthStencil = (VkClearDepthStencilValue){ 1.0f,  0 };

    VkCommandBuffer cmdBuffer = phase->renderpass->commandBuffers[currentFrame];
    VkFramebuffer framebuffer = phase->renderpass->framebuffers[swapchainImageIndex];
    VkRenderPass renderPass = phase->renderpass->renderPass;

    vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

    VkCommandBufferBeginInfo cmdBeginInfo = { 0 };
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = NULL;
    cmdBeginInfo.flags = 0;
    CREN_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo) == VK_SUCCESS, "Failed to begin viewport renderphase command buffer");

    VkRenderPassBeginInfo renderPassBeginInfo = { 0 };
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = framebuffer;
    renderPassBeginInfo.renderArea.offset = (VkOffset2D){ 0, 0 };
    renderPassBeginInfo.renderArea.extent = vkBackend->swapchain.swapchainExtent;
    renderPassBeginInfo.clearValueCount = 2U;
    renderPassBeginInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // set frame commandbuffer viewport
    VkViewport viewport = { 0 };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vkBackend->swapchain.swapchainExtent.width;
    viewport.height = (float)vkBackend->swapchain.swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

    // set frame commandbuffer scissor
    VkRect2D scissor = { 0 };
    scissor.offset = (VkOffset2D){ 0, 0 };
    scissor.extent = vkBackend->swapchain.swapchainExtent;
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    // render objects
    if (callback != NULL) callback(context, (CRen_RenderStage)CREN_RENDER_STAGE_DEFAULT, timestep);

    vkCmdEndRenderPass(cmdBuffer);

    // end command buffer
    CREN_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS, "Failed to end viewport renderphase command buffer");
}

#endif // CREN_BUILD_WITH_VULKAN