#include "cren_context.h"

#include "cren_camera.h"
#include "cren_error.h"
#include "Vulkan/crenvk_context.h"
#include <memm/memm.h>

struct CRenContext
{
    // needed info
	CRenCreateInfo createInfo;
    CRenCamera camera;

    // hints
    bool currentlyMinimized;
    bool usingCustomViewport;
    bool mustResize;

    // callbacks
    void* userPointer;
    CRenCallback_Render render;
    CRenCallback_Resize resize;
    CRenCallback_ImageCount imageCount;
    CRenCallback_DrawUIRawData drawUIRawData;

    // backend renderer api
    #ifdef CREN_BUILD_WITH_VULKAN
    CRenVulkanBackend backend;
    #endif
};

CRenContext* cren_initialize(CRenCreateInfo createInfo)
{
	memm_init();

	CRenContext* context = malloc(sizeof(CRenContext));
	CREN_ASSERT(context != NULL, "Failed to allocate memory for CRen");
	context->createInfo = createInfo;
	context->usingCustomViewport = createInfo.customViewport;

	#ifdef CREN_BUILD_WITH_VULKAN
	context->backend = crenvk_initialize
	(
		createInfo.width, createInfo.height, createInfo.window,
		createInfo.appName, createInfo.assetsPath,
		createInfo.appVersion, createInfo.api,
		createInfo.vsync, createInfo.msaa, createInfo.validations, createInfo.customViewport
	);
	#else
	#error "Unsupported Renderer Backend"
	#endif

	context->camera = cren_camera_create(CREN_CAMERA_TYPE_FREE_LOOK, (float)createInfo.width / (float)createInfo.height, createInfo.api);

	return context;
}

void cren_shutdown(CRenContext* context)
{
	char leaksMessage[2028];
	if (memm_get_leaks_string(leaksMessage, sizeof(leaksMessage)) > 0) {
		CREN_LOG(CRenLogSeverity_Error, "There were leaks detected uppon shutdown. Details:\n %s", leaksMessage);
	}

	memm_shutdown();
}

bool cren_get_vsync(CRenContext* context)
{
	// WARN: if I ever develop a way to change vsync on the go, this variable needs to be update
	return context->createInfo.vsync;
}

void* cren_get_vulkan_backend(CRenContext* context)
{
	if (!context) return NULL;
	return &context->backend;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cren_set_user_pointer(CRenContext* context, void* pointer)
{
    context->userPointer = pointer;
}

void* cren_get_user_pointer(CRenContext* context)
{
    return context->userPointer;
}

void cren_set_render_callback(CRenContext* context, CRenCallback_Render callback)
{
    context->render = callback;
}

void cren_set_resize_callback(CRenContext* context, CRenCallback_Resize callback)
{
    context->resize = callback;
}

void cren_set_ui_image_count_callback(CRenContext* context, CRenCallback_ImageCount callback)
{
    context->imageCount = callback;
}

void cren_set_draw_ui_raw_data_callback(CRenContext* context, CRenCallback_DrawUIRawData callback)
{
    context->drawUIRawData = callback;
}
