#include "crenvk_context.h"

#include "cren_error.h"
#include "cren_context.h"
#include "crenvk_buffer.h"
#include <memm/memm.h>

#ifdef CREN_BUILD_WITH_VULKAN

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
 	CREN_LOG(CREN_LOG_SEVERITY_INFO, "Resize completed successfully");
}

CREN_API CRenVulkanBackend crenvk_initialize(CRenContext* context, unsigned int width, unsigned int height, const CRenCallbacks* callbacks, const char* appName, const char* rootPath, unsigned int appVersion, CRen_Renderer api, CRen_MSAA msaa, bool vsync, bool validations, bool customViewport)
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
    vkBuffer* cameraBuffer = crenvk_buffer_create(backend.device.device, backend.device.physicalDevice, sizeof(vkBufferCamera), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, CREN_CONCURRENTLY_RENDERED_FRAMES);
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
	fmat4 view = cren_camera_get_view(camera);
	vkBufferCamera cameraData = { 0 };
	cameraData.view = view;
	cameraData.viewInverse = fmat4_inverse(&view);
	cameraData.proj = cren_camera_get_perspective(camera);
	cameraData.proj.data[1][1] *= -1.0f; // flip y because vulkan
	
	vkBuffer* cameraBuffer = (vkBuffer*)shashtable_lookup(backend->buffersLib, "Camera");
	crenvk_buffer_copy(cameraBuffer, backend->swapchain.currentFrame, &cameraData, sizeof(vkBufferCamera), 0);
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

#endif // CREN_BUILD_WITH_VULKAN
