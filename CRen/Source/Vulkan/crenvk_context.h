#ifndef CRENVK_CONTEXT_INCLUDED
#define CRENVK_CONTEXT_INCLUDED
#ifdef CREN_BUILD_WITH_VULKAN

#include "cren_defines.h"
#include "cren_types.h"
#include "crenvk_core.h"

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief holds all vulkan objects used by the backend
typedef struct
{
	vkInstance instance;
	//vkDevice device;
	//vkSwapchain swapchain;
	//vkDefaultRenderphase* defaultRenderphase;
	//vkPickingRenderphase* pickingRenderphase;
	//vkUIRenderphase* uiRenderphase;
	//vkViewportRenderphase* viewportRenderphase;
	//Hashtable* buffersLib;
	//Hashtable* pipelinesLib;
} CRenVulkanBackend;

/// @brief initializes the vulkan context
CREN_API CRenVulkanBackend crenvk_initialize
(
	unsigned int width,
	unsigned int height,
	void* window,
	const char* appName,
	const char* rootPath,
	unsigned int appVersion,
	CRen_Renderer api,
	bool vsync,
	bool msaa,
	bool validations,
	bool customViewport
);

#ifdef __cplusplus 
}
#endif

#endif // CREN_BUILD_WITH_VULKAN
#endif // CRENVK_CONTEXT_INCLUDED