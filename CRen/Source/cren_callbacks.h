#ifndef CREN_CALLBACKS_DEFINED
#define CREN_CALLBACKS_DEFINED

#include "cren_defines.h"
#include "cren_types.h"

#ifdef __cplusplus 
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief set-up the callback that tells the application it's time to render the world objects
typedef void (*CRenCallback_Render)(CRenContext* context, CRen_RenderStage stage, float timestep);

/// @brief set-up the callback that tells the CRen to resize renderer objects, usually called from outside
typedef void (*CRenCallback_Resize)(CRenContext* context, uint32_t width, uint32_t height);

/// @brief set-up the callback that tells CRen the ammount of swapchain images has changed
typedef void (*CRenCallback_ImageCount)(CRenContext* context, uint32_t count);

/// @brief set-up the callback that tells the application it's time to draw the ui raw data
typedef void (*CRenCallback_DrawUIRawData)(CRenContext* context, void* commandbuffer);

/// @brief set-up a callback that requests needed instance extensions for vulkan context creation
typedef const char* const* (*CRenCallback_GetVulkanRequiredInstanceExtensions)(CRenContext* context, uint32_t* count);

/// @brief set-up the callback that will create a vulkan surface, this is to facilitate the usage of SDL/GLFW and custom windowing systems
typedef void (*CRenCallback_CreateVulkanSurfaceCallback)(CRenContext* context, void* instance, void* surface);

/// @brief all callbacks necessary bundled-up into a struct for easy usage
typedef struct CRenCallbacks
{
	void* userPtr;
	CRenCallback_Render render;
	CRenCallback_Resize resize;
	CRenCallback_ImageCount imageCount;
	CRenCallback_DrawUIRawData drawUIRawData;
	CRenCallback_GetVulkanRequiredInstanceExtensions getVulkanRequiredInstanceExtensions;
	CRenCallback_CreateVulkanSurfaceCallback createVulkanSurfaceCallback;
} CRenCallbacks;

#ifdef __cplusplus 
}
#endif

#endif // CREN_CALLBACKS_DEFINED