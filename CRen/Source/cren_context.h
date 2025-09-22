#ifndef CREN_CONTEXT_INCLUDED
#define CREN_CONTEXT_INCLUDED

#include "cren_defines.h"
#include "cren_types.h"
#include "cren_callbacks.h"

#ifdef __cplusplus 
extern "C" {
#endif

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
	void* window; // opaque pointer to underneath window
} CRenCreateInfo;

/// @brief creates the cren context, initializing the library
/// @param createInfo the address of a create info specification
/// @return the cren context or NULL if failed
CREN_API CRenContext* cren_initialize(CRenCreateInfo createInfo);

/// @brief shutsdown and free used resources
/// @param context cren context
CREN_API void cren_shutdown(CRenContext* context);

/// @brief returns the curernt status of vertical syncronization
/// param@ context cren context
/// @return 0 = off; 1 = on
CREN_API bool cren_get_vsync(CRenContext* context);

/// @brief returns the underneath vulkan backend, must be casted as CRenVulkanBackend*
CREN_API void* cren_get_vulkan_backend(CRenContext* context);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief sets the user defined pointer address
/// @param context cren context memory address
/// @param pointer user-defined pointer
CREN_API void cren_set_user_pointer(CRenContext* context, void* pointer);

/// @brief returns the user defined pointer address
/// @param context cren context memory address
CREN_API void* cren_get_user_pointer(CRenContext* context);

/// @brief allows the application to know when it's time to render the world
/// @param context cren context memory address
/// @param callback callback to redirect the code to
CREN_API void cren_set_render_callback(CRenContext* context, CRenCallback_Render callback);

/// @brief allows the application to know when the renderer was resized
/// @param context cren context memory address
/// @param callback callback to redirect the code to
CREN_API void cren_set_resize_callback(CRenContext* context, CRenCallback_Resize callback);

/// @brief allows the application to know when the renderer has changed the ammount of swapchain image it has
/// @param context cren context memory address
/// @param callback callback to redirect the code to
CREN_API void cren_set_ui_image_count_callback(CRenContext* context, CRenCallback_ImageCount callback);

/// @brief allows the application to know when it's time to draw the ui raw data
/// @param context cren context memory address
/// @param callback callback to redirect the code to
CREN_API void cren_set_draw_ui_raw_data_callback(CRenContext* context, CRenCallback_DrawUIRawData callback);

#ifdef __cplusplus 
}
#endif

#endif // CREN_CONTEXT_INCLUDED
