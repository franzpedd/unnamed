#ifndef CREN_CONTEXT_INCLUDED
#define CREN_CONTEXT_INCLUDED

#include "cren_defines.h"
#include "cren_types.h"
#include "cren_camera.h"
#include "cren_callbacks.h"
#include <vecmath/vecmath.h>

/// @brief cren creation needed information
typedef struct CRenCreateInfo
{
	const char* appName;
	const char* assetsPath;
	CRen_Renderer api;
	uint32_t appVersion;
	uint32_t width;
	uint32_t height;
	CRen_MSAA msaa;
	bool validations;
	bool vsync;
	bool customViewport;
} CRenCreateInfo;

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief creates the cren context, initializing the library
CREN_API CRenContext* cren_initialize(const CRenCreateInfo pCreateInfo);

/// @brief creates the renderer underneath the library, this is a separate function in order to callbacks to be properly assigned
CREN_API void cren_create_renderer(CRenContext* context);

/// @brief shutsdown and free used resources
CREN_API void cren_shutdown(CRenContext* context);

/// @brief updates the renderer frame, sending useful information to the gpu
CREN_API void cren_update(CRenContext* context, float timestep);

/// @brief performs the frame rendering, setting-up all resources required for drawing the current frame and presenting the previously rendered
CREN_API void cren_render(CRenContext* context, float timestep);

/// @brief resizes the renderer, call this on every window resize event
CREN_API void cren_resize(CRenContext* context, int width, int height);

/// @brief minimizes the renderer, stopping rendering without staling it
CREN_API void cren_minimize(CRenContext* context);

/// @brief restores the renderer to it's last known size, resuming the rendering process
CREN_API void cren_restore(CRenContext* context);

/// @brief returns the cren main camera, we're only using one right now
CREN_API CRenCamera* cren_get_camera(CRenContext* context);

/// @brief returns if validation errors are enabled
CREN_API bool cren_are_validations_enabled(CRenContext* context);

/// @brief returns the curernt status of vertical syncronization
CREN_API bool cren_using_vsync(CRenContext* context);

/// @brief returns the current msaa applyed
CREN_API CRen_MSAA cren_get_msaa(CRenContext* context);

/// @brief returns if scene is being rendered on a separate viewport
CREN_API bool cren_using_custom_viewport(CRenContext* context);

/// @brief returns the underneath vulkan backend, must be casted as CRenVulkanBackend*
CREN_API void* cren_get_vulkan_backend(CRenContext* context);

/// @brief returns the mouse position, this must be previously set to be correct
CREN_API float2 cren_get_mousepos(CRenContext* context);

/// @brief setsthe mouse position, this must be properly configured
CREN_API void cren_set_mousepos(CRenContext* context, const float2 pos);

/// @brief sets the viewport position internally, this is used when a custom viewport is active, no effect otherwise
CREN_API void cren_set_viewport_pos(CRenContext* context, const float2 pos);

/// @brief sets the viewport size, this is used when a custom viewport is active, no effect otherwise
CREN_API void cren_set_viewport_size(CRenContext* context, const float2 size);

/// @brief returns the current framebuffer size
CREN_API float2 cren_get_framebuffer_size(CRenContext* context);

/// @brief set's the new framebuffer size (changes the current size wich will be noticded uppon render function and resized)
CREN_API void cren_set_framebuffer_size(CRenContext* context, const float2 size);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief sets the user defined pointer address
CREN_API void cren_set_user_pointer(CRenContext* context, void* pointer);

/// @brief returns the user defined pointer address
CREN_API void* cren_get_user_pointer(CRenContext* context);

/// @brief allows the application to know when it's time to render the world
CREN_API void cren_set_render_callback(CRenContext* context, CRenCallback_Render callback);

/// @brief allows the application to know when the renderer was resized
CREN_API void cren_set_resize_callback(CRenContext* context, CRenCallback_Resize callback);

/// @brief allows the application to know when the renderer has changed the ammount of swapchain image it has
CREN_API void cren_set_ui_image_count_callback(CRenContext* context, CRenCallback_ImageCount callback);

/// @brief allows the application to know when it's time to draw the ui raw data
CREN_API void cren_set_draw_ui_raw_data_callback(CRenContext* context, CRenCallback_DrawUIRawData callback);

/// @brief allows the application to request the neccessary instance extensions from other library
CREN_API void cren_set_get_vulkan_instance_required_extensions_callback(CRenContext* context, CRenCallback_GetVulkanRequiredInstanceExtensions callback);

/// @brief allows the application to create the vulkan surface outside the library scope, allowing using custom windowing manager
CREN_API void cren_set_create_vulkan_surface_callback(CRenContext* context, CRenCallback_CreateVulkanSurfaceCallback callback);

#ifdef __cplusplus 
}
#endif


#endif // CREN_CONTEXT_INCLUDED
