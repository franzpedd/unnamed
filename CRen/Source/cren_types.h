#ifndef CREN_CONTEXT_DEFINES_INCLUDED
#define CREN_CONTEXT_DEFINES_INCLUDED

/// @brief opaque cren context object
typedef struct CRenContext CRenContext;

/// @brief what version of the rendering API is desired
typedef enum CRen_RendererAPI
{
	CREN_RENDERER_API_VULKAN_1_0,
	CREN_RENDERER_API_VULKAN_1_1,
	CREN_RENDERER_API_VULKAN_1_2,
	CREN_RENDERER_API_VULKAN_1_3
} CRen_Renderer;

/// @brief how many samples per pixel the images has
typedef enum CRen_MSAA
{
	CREN_MSAA_X1,
	CREN_MSAA_X2,
	CREN_MSAA_X4,
	CREN_MSAA_X8,
	CREN_MSAA_X16,
	CREN_MSAA_X32,
	CREN_MSAA_X64
} CRen_MSAA;

/// @brief right now there's only two stages
typedef enum CRen_RenderStage
{
	CREN_RENDER_STAGE_DEFAULT,
	CREN_RENDER_STAGE_PICKING
} CRen_RenderStage;

#endif // CREN_CONTEXT_DEFINES_INCLUDED