#include "cren_context.h"

#include "cren_error.h"
#include "Vulkan/crenvk_context.h"
#include <memm/memm.h>
#include <ctoolbox/idgen.h>

struct CRenContext
{
    // core info
	CRenCreateInfo createInfo;
    CRenCamera* camera;
	idgen* idgen;

    // hints
    bool currentlyMinimized;
	bool usingCustomViewport;
	bool usingVSync;
    bool mustResize;

    // callbacks
	CRenCallbacks callbacks;

	// useful info
	float2 mousePos;
	float2 framebufferSize;
	float2 viewportPos;		// only when usingCustomViewport
	float2 viewportSize;	// only when usingCustomViewport

    // backend renderer api
	CRen_MSAA msaa;
    #ifdef CREN_BUILD_WITH_VULKAN
    CRenVulkanBackend backend;
    #endif
};

CREN_API CRenContext* cren_initialize(const CRenCreateInfo createInfo)
{
	memm_init();

	CRenContext* context = malloc(sizeof(CRenContext));
	CREN_ASSERT(context != NULL, "Failed to allocate memory for CRen");

	context->createInfo = createInfo;
	context->usingCustomViewport = createInfo.customViewport;
	context->usingVSync = createInfo.vsync;
	context->currentlyMinimized = false;
	context->mustResize = true;
	context->framebufferSize.xy.x = context->createInfo.width;
	context->framebufferSize.xy.y = context->createInfo.height;
	context->msaa = context->createInfo.msaa;
	
	context->camera = cren_camera_create(CREN_CAMERA_TYPE_FREE_LOOK, (float)createInfo.width / (float)createInfo.height, createInfo.api);
	CREN_ASSERT(context->camera != NULL, "Faield to create CRen's main camera");

	context->idgen = idgen_create(1);
	CREN_ASSERT(context->idgen != NULL, "Failed to create CRen's id generator");

	return context;
}

CREN_API void cren_create_renderer(CRenContext* context)
{
    #ifdef CREN_BUILD_WITH_VULKAN
	context->backend = crenvk_initialize
	(
		context, context->createInfo.width, context->createInfo.height, &context->callbacks,
		context->createInfo.appName, context->createInfo.assetsPath,
		context->createInfo.appVersion, context->createInfo.api, context->createInfo.msaa,
		context->createInfo.vsync, context->createInfo.validations, context->createInfo.customViewport
	);
	#else
	#error "Unsupported Renderer Backend"
	#endif
}

CREN_API void cren_shutdown(CRenContext* context)
{
	CREN_ASSERT(context != NULL, "CRen context is NULL and this should not occur");

	idgen_destroy(context->idgen);
	cren_camera_destroy(context->camera);

	#ifdef CREN_BUILD_WITH_VULKAN
	if (context) crenvk_terminate(&context->backend);
	#endif

	free(context);

	// memory leak checkage
	char leaksMessage[2028];
	memm_get_leaks_string(leaksMessage, sizeof(leaksMessage));
	CREN_LOG(CREN_LOG_SEVERITY_INFO, "%s", leaksMessage);

	memm_shutdown();
}

CREN_API void cren_update(CRenContext* context, float timestep)
{
	cren_camera_update(context->camera, timestep);
}

CREN_API void cren_render(CRenContext* context, float timestep)
{
	#ifdef CREN_BUILD_WITH_VULKAN
	if (!context->currentlyMinimized) {
		crenvk_update(&context->backend, timestep, context->camera);
		crenvk_render(context, &context->backend, timestep, &context->callbacks, context->camera, &context->mustResize);
	}
	#else
	#error "Undefined backend"
	#endif
}

CREN_API void cren_resize(CRenContext* context, int width, int height)
{
	context->framebufferSize.xy.x = width;
	context->framebufferSize.xy.y = height;
	context->mustResize = true; // vulkan will pickup the change automatically
}

CREN_API void cren_minimize(CRenContext* context)
{
	context->currentlyMinimized = 1; // vulkan will pickup the change automatically
}

CREN_API void cren_restore(CRenContext* context)
{
	context->currentlyMinimized = 0; // vulkan will pickup the change automatically
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Getters/Setters
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CREN_API CRenCamera* cren_get_main_camera(CRenContext* context)
{
	if (!context) return NULL;
	return context->camera;
}

CREN_API bool cren_are_validations_enabled(CRenContext* context)
{
	if (!context) return false;
	return context->createInfo.validations;
}

bool cren_using_vsync(CRenContext* context)
{
	return context->usingVSync;
}

CREN_API CRen_MSAA cren_get_msaa(CRenContext *context)
{
    return context->msaa;
}

CREN_API bool cren_using_custom_viewport(CRenContext* context)
{
	return context->usingCustomViewport;
}

CREN_API void* cren_get_vulkan_backend(CRenContext* context)
{
	if (!context) return NULL;
	return &context->backend;
}

CREN_API float2 cren_get_mousepos(CRenContext* context)
{
	if (!context) return (float2) {.xy.x = 0.0f, .xy.y = 0.0f };
	return context->mousePos;
}

CREN_API void cren_set_mousepos(CRenContext* context, const float2 pos)
{
	if (!context) return;
	context->mousePos = pos;
}

CREN_API float2 cren_get_viewport_pos(CRenContext* context)
{
	if (!context) return (float2){ 0.0f, 0.0f };
	if (!context->usingCustomViewport) return (float2) { 0.0f, 0.0f };
	return context->viewportPos;
}

CREN_API void cren_set_viewport_pos(CRenContext* context, const float2 pos)
{
	if (!context) return;
	if (!context->usingCustomViewport) return;
	context->viewportPos = pos;
}

CREN_API float2 cren_get_viewport_size(CRenContext* context)
{
	if (!context) return (float2) { 0.0f, 0.0f };
	if (!context->usingCustomViewport) return (float2) { 0.0f, 0.0f };
	return context->viewportSize;
}

CREN_API void cren_set_viewport_size(CRenContext* context, const float2 size)
{
	if (!context) return;
	if (!context->usingCustomViewport) return;
	context->viewportSize = size;
}

CREN_API float2 cren_get_framebuffer_size(CRenContext* context)
{
	if (!context) return (float2) { .xy.x = 0.0f, .xy.y = 0.0f };
	return context->framebufferSize;
}

CREN_API void cren_set_framebuffer_size(CRenContext* context, const float2 size)
{
	if (!context) return;
	context->framebufferSize = size;
	context->mustResize = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Fun
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CREN_API uint32_t cren_pick_object(CRenContext* context, float2 screenCoord)
{
	if (!context) return 0;
	uint32_t res = 0;
	#ifdef CREN_BUILD_WITH_VULKAN
	res = crenvk_pick_object(context, &context->backend, screenCoord);
	#else 
	#error "Undefined backend"
	#endif
	return res;
}

CREN_API uint32_t cren_create_id(CRenContext* context)
{
	if (!context) return 0;
	return idgen_next(context->idgen);
}

CREN_API bool cren_register_id(CRenContext* context, uint32_t id)
{
	if (!context) return false;
	return idgen_register(context->idgen, id);
}

CREN_API bool cren_unregister_id(CRenContext* context, uint32_t id)
{
	if (!context) return false;
	return idgen_unregister(context->idgen, id);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CREN_API void cren_set_user_pointer(CRenContext* context, void* pointer)
{
    context->callbacks.userPtr = pointer;
}

CREN_API void* cren_get_user_pointer(CRenContext* context)
{
    return context->callbacks.userPtr;
}

CREN_API void cren_set_render_callback(CRenContext* context, CRenCallback_Render callback)
{
    context->callbacks.render = callback;
}

CREN_API void cren_set_resize_callback(CRenContext* context, CRenCallback_Resize callback)
{
    context->callbacks.resize = callback;
}

CREN_API void cren_set_ui_image_count_callback(CRenContext* context, CRenCallback_ImageCount callback)
{
    context->callbacks.imageCount = callback;
}

CREN_API void cren_set_draw_ui_raw_data_callback(CRenContext* context, CRenCallback_DrawUIRawData callback)
{
	context->callbacks.drawUIRawData = callback;
}

CREN_API void cren_set_get_vulkan_instance_required_extensions_callback(CRenContext* context, CRenCallback_GetVulkanRequiredInstanceExtensions callback)
{
    context->callbacks.getVulkanRequiredInstanceExtensions = callback;
}

CREN_API void cren_set_create_vulkan_surface_callback(CRenContext *context, CRenCallback_CreateVulkanSurfaceCallback callback)
{
    context->callbacks.createVulkanSurfaceCallback = callback;
}
