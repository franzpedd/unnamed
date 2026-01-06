#include "crenvk_context.h"

#include "cren_error.h"
#include "cren_context.h"
#include "crenvk_buffer.h"
#include <memm/memm.h>

#if defined (CREN_BUILD_WITH_VULKAN)

/// @brief handles the resize code, used internally
static void internal_crenvk_resize(CRenVulkanBackend* backend, CRenContext* context, const CRenCallbacks* callbacks, CRenCamera* camera, bool customViewport, bool vsync, bool* hintResize)
{
    vkDeviceWaitIdle(backend->device.device);

	CRen_MSAA msaa = cren_get_msaa(context);
    float2 framebufferSize = cren_get_framebuffer_size(context);
    VkExtent2D newExtent = { (uint32_t)framebufferSize.xy.x, (uint32_t)framebufferSize.xy.y };

	// If you wish to make a vulkan resize, you must first re-invent the universe
	if (customViewport) {
		crenvk_renderphase_viewport_destroy(backend->viewportRenderphase, backend->device.device, true);
	}
    crenvk_renderphase_ui_destroy(backend->uiRenderphase, backend->device.device, true);
    crenvk_renderphase_picking_destroy(backend->pickingRenderphase, backend->device.device, true, false);
    crenvk_renderphase_default_destroy(backend->defaultRenderphase, backend->device.device, true, false);
	
    crenvk_swapchain_destroy(&backend->swapchain, backend->device.device);
	crenvk_swapchain_create(&backend->swapchain, backend->device.device, backend->device.physicalDevice, backend->device.surface, newExtent.width, newExtent.height, vsync);

   	crenvk_renderphase_default_create(backend->device.device, backend->device.physicalDevice, backend->device.surface, backend->swapchain.swapchainFormat.format, (VkSampleCountFlagBits)msaa, !customViewport, backend->defaultRenderphase);
    crenvk_renderphase_default_create_framebuffers(backend->defaultRenderphase, backend->device.device, backend->device.physicalDevice, backend->swapchain.swapchainImageViews, backend->swapchain.swapchainImageCount, backend->swapchain.swapchainExtent, backend->swapchain.swapchainFormat.format);
    crenvk_renderphase_picking_create(backend->device.device, backend->device.physicalDevice, backend->device.surface, backend->pickingRenderphase);
    crenvk_renderphase_picking_create_framebuffers(backend->pickingRenderphase, backend->device.device, backend->device.physicalDevice, backend->device.graphicsQueue, backend->swapchain.swapchainImageViews, backend->swapchain.swapchainImageCount, backend->swapchain.swapchainExtent);
    crenvk_renderphase_ui_create(backend->device.device, backend->device.physicalDevice, backend->device.surface, backend->swapchain.swapchainFormat.format, VK_SAMPLE_COUNT_1_BIT, 1, backend->uiRenderphase);
    crenvk_renderphase_ui_framebuffers_create(backend->uiRenderphase, backend->device.device, backend->swapchain.swapchainExtent, backend->swapchain.swapchainImageViews, backend->swapchain.swapchainImageCount);
    if (customViewport) {
        crenvk_renderphase_viewport_create(backend->device.device, backend->device.physicalDevice, backend->device.surface, VK_FORMAT_R8G8B8A8_SRGB, VK_SAMPLE_COUNT_1_BIT, backend->viewportRenderphase);
        crenvk_renderphase_viewport_create_framebuffers(backend->viewportRenderphase, backend->device.device, backend->device.physicalDevice, backend->device.graphicsQueue, backend->swapchain.swapchainImageViews, backend->swapchain.swapchainImageCount, backend->swapchain.swapchainExtent);
    }

 	// update camera aspect ratio
 	float aspect = (newExtent.height > 0) ? (float)newExtent.width / (float)newExtent.height : 1.0f;
 	cren_camera_set_aspect_ratio(camera, aspect);
 	
 	// notify application
 	if (callbacks->resize != NULL) {
 	    CRenCallback_Resize fnResize = (CRenCallback_Resize)callbacks->resize;
 	    fnResize(context, newExtent.width, newExtent.height);
 	}
 	
 	if (callbacks->imageCount != NULL) {
 	    CRenCallback_ImageCount fnImageCount = (CRenCallback_ImageCount)callbacks->imageCount;
 	    fnImageCount(context, backend->swapchain.swapchainImageCount);
 	}
 	
 	*hintResize = false;
}

CREN_API CRenVulkanBackend crenvk_initialize(CRenContext* context, unsigned int width, unsigned int height, const CRenCallbacks* callbacks, const char* appName, const char* rootPath, unsigned int appVersion, CRen_RendererAPI api, CRen_MSAA msaa, bool vsync, bool validations, bool customViewport)
{
    VkResult res = VK_SUCCESS;
    ctoolbox_result toolboxres = CTOOLBOX_SUCCESS;
    CRenVulkanBackend backend = { 0 };

    crenvk_instance_create(context, &backend.instance, appName, appVersion, api, validations, callbacks->getVulkanRequiredInstanceExtensions);
	callbacks->createVulkanSurfaceCallback(context, backend.instance.instance, &backend.device.surface);
    crenvk_device_create(&backend.device, backend.instance.instance, validations);
    crenvk_swapchain_create(&backend.swapchain, backend.device.device, backend.device.physicalDevice, backend.device.surface, width, height, vsync);

    // default renderphase
    backend.defaultRenderphase = (vkDefaultRenderphase*)malloc(sizeof(vkDefaultRenderphase));
    CREN_ASSERT(backend.defaultRenderphase != NULL, "Failed to allocate memory for default renderphase");

    res = crenvk_renderphase_default_create(backend.device.device, backend.device.physicalDevice, backend.device.surface, backend.swapchain.swapchainFormat.format, (VkSampleCountFlagBits)msaa, !customViewport, backend.defaultRenderphase);
    CREN_ASSERT(res == VK_SUCCESS, "CRen has failed to create it's default/main render phase");
    res = crenvk_renderphase_default_create_framebuffers(backend.defaultRenderphase, backend.device.device, backend.device.physicalDevice, backend.swapchain.swapchainImageViews, backend.swapchain.swapchainImageCount, backend.swapchain.swapchainExtent, backend.swapchain.swapchainFormat.format);
    CREN_ASSERT(res == VK_SUCCESS, "CRen has failed to create it's default/main framebuffers");

    // picking renderphase
    backend.pickingRenderphase = (vkPickingRenderphase*)malloc(sizeof(vkPickingRenderphase));
    CREN_ASSERT(backend.defaultRenderphase != NULL, "Failed to allocate memory for default renderphase");

    res = crenvk_renderphase_picking_create(backend.device.device, backend.device.physicalDevice, backend.device.surface, backend.pickingRenderphase);
    CREN_ASSERT(res == VK_SUCCESS, "CRen has failed to create it's picking render phase");
    res = crenvk_renderphase_picking_create_framebuffers(backend.pickingRenderphase, backend.device.device, backend.device.physicalDevice, backend.device.graphicsQueue, backend.swapchain.swapchainImageViews, backend.swapchain.swapchainImageCount, backend.swapchain.swapchainExtent);
    CREN_ASSERT(res == VK_SUCCESS, "CRen has failed to create it's picking framebuffers");

    // ui renderphase
    backend.uiRenderphase = (vkUIRenderphase*)malloc(sizeof(vkUIRenderphase));
    CREN_ASSERT(backend.uiRenderphase != NULL, "Failed to allocate memory for ui renderphase");

    res = crenvk_renderphase_ui_create(backend.device.device, backend.device.physicalDevice, backend.device.surface, backend.swapchain.swapchainFormat.format, VK_SAMPLE_COUNT_1_BIT, 1, backend.uiRenderphase);
    CREN_ASSERT(res == VK_SUCCESS, "CRen has failed to create it's ui render phase");
    res = crenvk_renderphase_ui_framebuffers_create(backend.uiRenderphase, backend.device.device, backend.swapchain.swapchainExtent, backend.swapchain.swapchainImageViews, backend.swapchain.swapchainImageCount);
    CREN_ASSERT(res == VK_SUCCESS, "CRen has failed to create it's ui framebuffer");

    // viewport renderphase
    if (customViewport) {
        backend.viewportRenderphase = (vkViewportRenderphase*)malloc(sizeof(vkViewportRenderphase));
        CREN_ASSERT(backend.viewportRenderphase != NULL, "Failed to allocate memory for viewport renderphase");

        res = crenvk_renderphase_viewport_create(backend.device.device, backend.device.physicalDevice, backend.device.surface, VK_FORMAT_R8G8B8A8_SRGB, VK_SAMPLE_COUNT_1_BIT, backend.viewportRenderphase);
        CREN_ASSERT(res == VK_SUCCESS, "CRen has failed to create it's viewport render phase");
        res = crenvk_renderphase_viewport_create_framebuffers(backend.viewportRenderphase, backend.device.device, backend.device.physicalDevice, backend.device.graphicsQueue, backend.swapchain.swapchainImageViews, backend.swapchain.swapchainImageCount, backend.swapchain.swapchainExtent);
        CREN_ASSERT(res == VK_SUCCESS, "CRen has failed to create it's viewport framebuffers");
    }

    // buffers
    vkBuffer* cameraBuffer = crenvk_buffer_create(backend.device.device, backend.device.physicalDevice, sizeof(BufferCamera), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, CREN_CONCURRENTLY_RENDERED_FRAMES);
	CREN_ASSERT(cameraBuffer != NULL, "Failed to create camera buffer");

    backend.buffersLib = shashtable_init();
    CREN_ASSERT(backend.buffersLib != NULL, "Failed to create buffer's library");
    toolboxres = shashtable_insert(backend.buffersLib, "Camera", cameraBuffer);
    CREN_ASSERT(toolboxres == CTOOLBOX_SUCCESS, "Failed to insert camera's buffer into the buffer library");

    // pipelines
    backend.pipelinesLib = shashtable_init();
    CREN_ASSERT(backend.pipelinesLib != NULL, "Failed to create buffer's library");

    vkRenderpass* sceneRenderpass = customViewport ? backend.viewportRenderphase->renderpass : backend.defaultRenderphase->renderpass;
    res = crenvk_pipeline_create_quad(backend.pipelinesLib, sceneRenderpass, backend.pickingRenderphase->renderpass, backend.device.device, rootPath);
    CREN_ASSERT(res == VK_SUCCESS, "Failed to create quad pipeline");

    return backend;
}

CREN_API void crenvk_terminate(CRenVulkanBackend* backend)
{
    if (!backend) return;

    // buffers and pipelines
    crenvk_pipeline_destroy(backend->device.device, (vkPipeline*)shashtable_lookup(backend->pipelinesLib, CREN_PIPELINE_QUAD_DEFAULT_NAME));
    crenvk_pipeline_destroy(backend->device.device, (vkPipeline*)shashtable_lookup(backend->pipelinesLib, CREN_PIPELINE_QUAD_PICKING_NAME));
	shashtable_destroy(backend->pipelinesLib);
    //
	crenvk_buffer_destroy(backend->device.device, (vkBuffer*)shashtable_lookup(backend->buffersLib, "Camera"));
	shashtable_destroy(backend->buffersLib);

    // renderphases
    if (backend->viewportRenderphase) {
        crenvk_renderphase_viewport_destroy(backend->viewportRenderphase, backend->device.device, true);
        free(backend->viewportRenderphase);
    }
    crenvk_renderphase_ui_destroy(backend->uiRenderphase, backend->device.device, true);
    free(backend->uiRenderphase);
    crenvk_renderphase_picking_destroy(backend->pickingRenderphase, backend->device.device, true, false);
    free(backend->pickingRenderphase);
    crenvk_renderphase_default_destroy(backend->defaultRenderphase, backend->device.device, true, false);
    free(backend->defaultRenderphase);

    // core objects
    crenvk_swapchain_destroy(&backend->swapchain, backend->device.device);
    crenvk_device_destroy(&backend->device, backend->instance.instance);
    crenvk_instance_destroy(&backend->instance);
}

CREN_API void crenvk_update(CRenVulkanBackend* backend, float timestep, CRenCamera* camera)
{
	// send information about the camera to the gpu camera buffer
	BufferCamera cameraData = { 0 };
	cameraData.view = cren_camera_get_view(camera);
    cameraData.viewInverse = cren_camera_get_view_inverse(camera);
	cameraData.proj = cren_camera_get_perspective(camera);
	
	vkBuffer* cameraBuffer = (vkBuffer*)shashtable_lookup(backend->buffersLib, "Camera");
	crenvk_buffer_copy(cameraBuffer, backend->swapchain.currentFrame, &cameraData, sizeof(BufferCamera), 0);
}

CREN_API void crenvk_render(void* context, CRenVulkanBackend* backend, float timestep, const CRenCallbacks* callbacks, CRenCamera* camera, bool* hintResize)
{
	CRenContext* ctx = (CRenContext*)context;
	bool customViewport = cren_using_custom_viewport(ctx);
	bool vsync = cren_using_vsync(ctx);

	uint32_t currentFrame = backend->swapchain.currentFrame;
	vkWaitForFences(backend->device.device, 1, &backend->swapchain.framesInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	VkResult res = vkAcquireNextImageKHR(backend->device.device, backend->swapchain.swapchain, UINT64_MAX, backend->swapchain.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &backend->swapchain.imageIndex);

	// failed to acquire next image, must recreate
	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		internal_crenvk_resize(backend, ctx, callbacks, camera, customViewport, vsync, hintResize);

		// advance frame even on recreation to avoid stalling
		backend->swapchain.currentFrame = (currentFrame + 1) % CREN_CONCURRENTLY_RENDERED_FRAMES;
		return;
	}

	CREN_ASSERT(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR, "Renderer update was not able to aquire an image from the swapchain");
	vkResetFences(backend->device.device, 1, &backend->swapchain.framesInFlightFences[currentFrame]);

	// manage renderpasses/render phases
	crenvk_renderphase_default_update(backend->defaultRenderphase, context, backend, currentFrame, backend->swapchain.imageIndex, customViewport, timestep, callbacks->render);
	crenvk_renderphase_picking_update(backend->pickingRenderphase, context, backend, currentFrame, backend->swapchain.imageIndex, customViewport, timestep, callbacks->render);
	crenvk_renderphase_viewport_update(backend->viewportRenderphase, context, backend, currentFrame, backend->swapchain.imageIndex, customViewport, timestep, callbacks->render);
	crenvk_renderphase_ui_update(backend->uiRenderphase, context, backend, currentFrame, backend->swapchain.imageIndex, callbacks->drawUIRawData);

	// submit command buffers
	VkSwapchainKHR swapChains[] = { backend->swapchain.swapchain };
	VkSemaphore waitSemaphores[] = { backend->swapchain.imageAvailableSemaphores[currentFrame] };
	VkSemaphore signalSemaphores[] = { backend->swapchain.finishedRenderingSemaphores[backend->swapchain.imageIndex] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo = { 0 };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (customViewport) {
		VkCommandBuffer commandBuffers[4] = { 0 };
		commandBuffers[0] = backend->defaultRenderphase->renderpass->commandBuffers[currentFrame];
		commandBuffers[1] = backend->pickingRenderphase->renderpass->commandBuffers[currentFrame];
		commandBuffers[2] = backend->viewportRenderphase->renderpass->commandBuffers[currentFrame];
		commandBuffers[3] = backend->uiRenderphase->renderpass->commandBuffers[currentFrame];

		submitInfo.commandBufferCount = CREN_STATIC_ARRAY_SIZE(commandBuffers);
		submitInfo.pCommandBuffers = commandBuffers;

		VkResult queueSubmit = vkQueueSubmit(backend->device.graphicsQueue, 1, &submitInfo, backend->swapchain.framesInFlightFences[currentFrame]);
		if (queueSubmit != VK_SUCCESS) {
			CREN_ASSERT(1, "Renderer update was not able to submit frame to graphics queue");
		}
	}

	else {
		VkCommandBuffer commandBuffers[3] = { 0 };
		commandBuffers[0] = backend->defaultRenderphase->renderpass->commandBuffers[currentFrame];
		commandBuffers[1] = backend->pickingRenderphase->renderpass->commandBuffers[currentFrame];
		commandBuffers[2] = backend->uiRenderphase->renderpass->commandBuffers[currentFrame];

		submitInfo.commandBufferCount = CREN_STATIC_ARRAY_SIZE(commandBuffers);
		submitInfo.pCommandBuffers = commandBuffers;

		VkResult queueSubmit = vkQueueSubmit(backend->device.graphicsQueue, 1, &submitInfo, backend->swapchain.framesInFlightFences[currentFrame]);
		if (queueSubmit != VK_SUCCESS) {
			CREN_ASSERT(1, "Renderer update was not able to submit frame to graphics queue");
		}
	}

	// present the image
	VkPresentInfoKHR presentInfo = { 0 };
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &backend->swapchain.imageIndex;

	res = vkQueuePresentKHR(backend->device.graphicsQueue, &presentInfo);

	// failed to present the image, must recreate
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || *hintResize == true) {
		internal_crenvk_resize(backend, ctx, callbacks, camera, customViewport, vsync, hintResize);
	}

	else if (res != VK_SUCCESS) {
		CREN_ASSERT(1, "Renderer update was not able to properly present the graphics queue frame");
	}

	// advance to the next frame for the next render call
	backend->swapchain.currentFrame = (currentFrame + 1) % CREN_CONCURRENTLY_RENDERED_FRAMES;
}

CREN_API uint32_t crenvk_pick_object(void* context, CRenVulkanBackend* backend, float2 screenCoord)
{
    VkResult res;
    uint32_t pixelValue = 0;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;

    VkBufferCreateInfo bufferCI = { 0 };
    bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCI.size = sizeof(uint32_t);
    bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    res = vkCreateBuffer(backend->device.device, &bufferCI, NULL, &stagingBuffer);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create staging buffer for picking");
        return 0;
    }

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(backend->device.device, stagingBuffer, &memReq);

    VkDeviceSize alignedSize = (memReq.size + 3) & ~3;

    uint32_t memType = crenvk_device_find_memory_type(backend->device.physicalDevice, memReq.memoryTypeBits,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (memType == UINT32_MAX) {
        vkDestroyBuffer(backend->device.device, stagingBuffer, NULL);
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "No suitable memory type for picking");
        return 0;
    }

    VkMemoryAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = alignedSize;
    allocInfo.memoryTypeIndex = memType;

    res = vkAllocateMemory(backend->device.device, &allocInfo, NULL, &stagingMemory);
    if (res != VK_SUCCESS) {
        vkDestroyBuffer(backend->device.device, stagingBuffer, NULL);
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate memory for picking");
        return 0;
    }

    res = vkBindBufferMemory(backend->device.device, stagingBuffer, stagingMemory, 0);
    if (res != VK_SUCCESS) {
        vkFreeMemory(backend->device.device, stagingMemory, NULL);
        vkDestroyBuffer(backend->device.device, stagingBuffer, NULL);
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to bind buffer memory for picking");
        return 0;
    }

    VkCommandBufferAllocateInfo cmdAlloc = { 0 };
    cmdAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAlloc.commandPool = backend->pickingRenderphase->renderpass->commandPool;
    cmdAlloc.commandBufferCount = 1;

    res = vkAllocateCommandBuffers(backend->device.device, &cmdAlloc, &cmdBuffer);
    if (res != VK_SUCCESS) {
        vkFreeMemory(backend->device.device, stagingMemory, NULL);
        vkDestroyBuffer(backend->device.device, stagingBuffer, NULL);
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate command buffer for picking");
        return 0;
    }

    VkCommandBufferBeginInfo beginInfo = { 0 };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    res = vkBeginCommandBuffer(cmdBuffer, &beginInfo);
    if (res != VK_SUCCESS) {
        vkFreeMemory(backend->device.device, stagingMemory, NULL);
        vkDestroyBuffer(backend->device.device, stagingBuffer, NULL);
        vkFreeCommandBuffers(backend->device.device, backend->pickingRenderphase->renderpass->commandPool, 1, &cmdBuffer);
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to begin command buffer for picking");
        return 0;
    }

    uint32_t pickingImageIndex = backend->swapchain.imageIndex;

    VkImageMemoryBarrier barrier = { 0 };
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = backend->pickingRenderphase->colorImage[pickingImageIndex];
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

    // adjust screen coordinates
    float2 winSize = { (float)backend->swapchain.swapchainExtent.width, (float)backend->swapchain.swapchainExtent.height };
    if (cren_using_custom_viewport((CRenContext*)context)) {
        winSize = cren_get_viewport_size((CRenContext*)context);
    }
    uint32_t fbX = (uint32_t)(screenCoord.xy.x * backend->swapchain.swapchainExtent.width / winSize.xy.x);
    uint32_t fbY = (uint32_t)(screenCoord.xy.y * backend->swapchain.swapchainExtent.height / winSize.xy.y);

    VkBufferImageCopy region = { 0 };
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = (VkOffset3D){ (int32_t)fbX, (int32_t)fbY, 0 };
    region.imageExtent = (VkExtent3D){ 1, 1, 1 };
    vkCmdCopyImageToBuffer(cmdBuffer, backend->pickingRenderphase->colorImage[pickingImageIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier( cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

    res = vkEndCommandBuffer(cmdBuffer);
    if (res != VK_SUCCESS) {
        vkFreeMemory(backend->device.device, stagingMemory, NULL);
        vkDestroyBuffer(backend->device.device, stagingBuffer, NULL);
        vkFreeCommandBuffers(backend->device.device, backend->pickingRenderphase->renderpass->commandPool, 1, &cmdBuffer);
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to end command buffer for picking");
        return 0;
    }

    VkFenceCreateInfo fenceCI = { 0 };
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    res = vkCreateFence(backend->device.device, &fenceCI, NULL, &fence);
    if (res != VK_SUCCESS) {
        vkFreeMemory(backend->device.device, stagingMemory, NULL);
        vkDestroyBuffer(backend->device.device, stagingBuffer, NULL);
        vkFreeCommandBuffers(backend->device.device, backend->pickingRenderphase->renderpass->commandPool, 1, &cmdBuffer);
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create fence for picking");
        return 0;
    }

    VkSubmitInfo submit = { 0 };
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmdBuffer;

    res = vkQueueSubmit(backend->device.graphicsQueue, 1, &submit, fence);
    if (res != VK_SUCCESS) {
        vkDestroyFence(backend->device.device, fence, NULL);
        vkFreeMemory(backend->device.device, stagingMemory, NULL);
        vkDestroyBuffer(backend->device.device, stagingBuffer, NULL);
        vkFreeCommandBuffers(backend->device.device, backend->pickingRenderphase->renderpass->commandPool, 1, &cmdBuffer);
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to submit picking command buffer");
        return 0;
    }

    res = vkWaitForFences(backend->device.device, 1, &fence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(backend->device.device, fence, NULL);

    if (res != VK_SUCCESS) {
        vkFreeMemory(backend->device.device, stagingMemory, NULL);
        vkDestroyBuffer(backend->device.device, stagingBuffer, NULL);
        vkFreeCommandBuffers(backend->device.device, backend->pickingRenderphase->renderpass->commandPool, 1, &cmdBuffer);
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to wait for picking fence");
        return 0;
    }

    // read the pixel value
    void* data = NULL;
    res = vkMapMemory(backend->device.device, stagingMemory, 0, sizeof(uint32_t), 0, &data);

    if (res == VK_SUCCESS && data != NULL) {
        pixelValue = *(uint32_t*)data;
        vkUnmapMemory(backend->device.device, stagingMemory);

        if (pixelValue != 0) {
            CREN_LOG(CREN_LOG_SEVERITY_INFO, "Picked entity ID: %u", pixelValue);
        }
    }
    else {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to map memory for picking read");
    }

    vkFreeCommandBuffers(backend->device.device, backend->pickingRenderphase->renderpass->commandPool, 1, &cmdBuffer);
    vkFreeMemory(backend->device.device, stagingMemory, NULL);
    vkDestroyBuffer(backend->device.device, stagingBuffer, NULL);

    return pixelValue;
}

#endif // CREN_BUILD_WITH_VULKAN
