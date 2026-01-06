#ifndef CRENVK_CONTEXT_INCLUDED
#define CRENVK_CONTEXT_INCLUDED
#ifdef CREN_BUILD_WITH_VULKAN

#include "cren_callbacks.h"
#include "cren_camera.h"
#include "cren_defines.h"
#include "cren_types.h"
#include "crenvk_core.h"
#include "crenvk_renderphase.h"
#include <shashtable.h>

/// @brief holds all vulkan objects used by the backend
typedef struct CRenVulkanBackend
{
	vkInstance instance;
	vkDevice device;
	vkSwapchain swapchain;
	vkDefaultRenderphase* defaultRenderphase;
	vkPickingRenderphase* pickingRenderphase;
	vkUIRenderphase* uiRenderphase;
	vkViewportRenderphase* viewportRenderphase;
	shashtable* buffersLib;
	shashtable* pipelinesLib;
} CRenVulkanBackend;

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief initializes the vulkan context
CREN_API CRenVulkanBackend crenvk_initialize
(
	CRenContext* context,
	unsigned int width,
	unsigned int height,
	const CRenCallbacks* callbacks,
	const char* appName,
	const char* rootPath,
	unsigned int appVersion,
	CRen_RendererAPI api,
	CRen_MSAA msaa,
	bool vsync,
	bool validations,
	bool customViewport
);

/// @brief shutdown the vulkan context
CREN_API void crenvk_terminate(CRenVulkanBackend* backend);

/// @brief updates the current frame, sending necessary info to the gpu about the frame
CREN_API void crenvk_update(CRenVulkanBackend* backend, float timestep, CRenCamera* camera);

/// @brief manage all renderpasses and call the rendering functions, callbacking them when appropriate
CREN_API void crenvk_render(void* context, CRenVulkanBackend* backend, float timestep, const CRenCallbacks* callbacks, CRenCamera* camera, bool* hintResize);

/// @brief attempts to pick an object based on screen coordinates
CREN_API uint32_t crenvk_pick_object(void* context, CRenVulkanBackend* backend, float2 screenCoord);

#ifdef __cplusplus 
}
#endif

#endif // CREN_BUILD_WITH_VULKAN
#endif // CRENVK_CONTEXT_INCLUDED
