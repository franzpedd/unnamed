#ifndef CREN_CONTEXT_DEFINES_INCLUDED
#define CREN_CONTEXT_DEFINES_INCLUDED

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Opaque objects
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief opaque cren context
typedef struct CRenContext CRenContext;

/// @brief opaque cren camera
typedef struct CRenCamera CRenCamera;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enumerations
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief what underneath platform is being used
typedef enum CRen_Platform
{
	CREN_PLATFORM_UNKNOWN = -2,
	CREN_PLATFORM_UNKNOWN_APPLE = -1,
	CREN_PLATFORM_MACOS,
	CREN_PLATFORM_IOS,
	CREN_PLATFORM_X11,
	CREN_PLATFORM_LINUX,
	CREN_PLATFORM_ANDROID,
	CREN_PLATFORM_WINDOWS
} CRen_Platform;

/// @brief defines multiple camera types
typedef enum CRen_CameraType
{
	CREN_CAMERA_TYPE_LOOK_AT,
	CREN_CAMERA_TYPE_FREE_LOOK
} CRen_CameraType;

/// @brief define the camera moving directions
typedef enum CRen_CameraDirection
{
	CREN_CAMERA_DIRECTION_FORWARD,
	CREN_CAMERA_DIRECTION_BACKWARD,
	CREN_CAMERA_DIRECTION_LEFT,
	CREN_CAMERA_DIRECTION_RIGHT
} CRen_CameraDirection;

/// @brief all kinds of errors severity exists, a fatal error will always trigger a crash
typedef enum CRenLogSeverity
{
	CREN_LOG_SEVERITY_TRACE,
	CREN_LOG_SEVERITY_TODO,
	CREN_LOG_SEVERITY_INFO,
	CREN_LOG_SEVERITY_WARN,
	CREN_LOG_SEVERITY_ERROR,
	CREN_LOG_SEVERITY_FATAL
} CRen_LogSeverity;

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
	CREN_MSAA_X1 = 0x00000001,
	CREN_MSAA_X2 = 0x00000002,
	CREN_MSAA_X4 = 0x00000004,
	CREN_MSAA_X8 = 0x00000008,
	CREN_MSAA_X16 = 0x00000010,
	CREN_MSAA_X32 = 0x00000020,
	CREN_MSAA_X64 = 0x00000040
} CRen_MSAA;

/// @brief all types of shaders
typedef enum CRen_ShaderType
{
	SHADER_TYPE_VERTEX = 0,
	SHADER_TYPE_FRAGMENT,
	SHADER_TYPE_COMPUTE,
	SHADER_TYPE_GEOMETRY,
	SHADER_TYPE_TESS_CTRL,
	SHADER_TYPE_TESS_EVAL
} CRen_ShaderType;

/// @brief all types of vertex components, custom attributes not supported, also only covering 1 of each type at the momment
typedef enum CRen_VertexComponent
{
	VERTEX_COMPONENT_POSITION = 0,
	VERTEX_COMPONENT_NORMAL,
	VERTEX_COMPONENT_UV_0,          // UV_1, UV_2 ...
	VERTEX_COMPONENT_COLOR_0,       // COLOR_1, COLOR_2 ...
	VERTEX_COMPONENT_JOINTS_0,      // JOINTS_1, JOINTS_2 ...
	VERTEX_COMPONENT_WEIGHTS_0,     // WEIGHTS_1, WEIGHTS_2, ...

	VERTEX_COMPONENTS_MAX
} CRen_VertexComponent;

/// @brief right now there's only two stages
typedef enum CRen_RenderStage
{
	CREN_RENDER_STAGE_DEFAULT,
	CREN_RENDER_STAGE_PICKING
} CRen_RenderStage;

#endif // CREN_CONTEXT_DEFINES_INCLUDED
