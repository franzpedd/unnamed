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
/// CRen delegate the rendering process to external application since its the application's job to optmize it's world, this may change in the future depending on community request, internally handling the rendering process
/// @param context cren context
/// @param stage rendering stage, default = 0; picking = 1
/// @param context deltatime/timestep, the renderstate interpolation
typedef void (*CRenCallback_Render)(CRenContext* context, CRen_RenderStage stage, float timestep);

/// @brief set-up the callback that tells the CRen to resize renderer objects, usually called from outside
/// @param context cren context
/// @param width renderer (and window) width
/// @param width renderer (and window) height
typedef void (*CRenCallback_Resize)(CRenContext* context, uint32_t width, uint32_t height);

/// @brief set-up the callback that tells CRen the ammount of swapchain images has changed
/// @param context cren context
/// @param count the new ammount of swapchain images
typedef void (*CRenCallback_ImageCount)(CRenContext* context, uint32_t count);

/// @brief set-up the callback that tells the application it's time to draw the ui raw data
/// @param context cren context
/// @param commandbuffer vulkan command object raw-ptr
typedef void (*CRenCallback_DrawUIRawData)(CRenContext* context, void* commandbuffer);

/// @brief set-up a callback that requests needed instance extensions for vulkan context creation
/// @param context cren context
/// @param count output instances count
/// @return the list of required extensions
typedef const char* const* (*CRenCallback_GetVulkanRequiredInstanceExtensions)(CRenContext* context, uint32_t* count);

/// @brief set-up the callback that will create a vulkan surface, this is to facilitate the usage of SDL/GLFW and custom windowing systems
/// @param instance opaque vulkan instance
/// @param surface opaque vulkan surface
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