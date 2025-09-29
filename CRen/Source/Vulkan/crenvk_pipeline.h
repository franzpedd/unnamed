#ifndef CRENVK_PIPELINE_INCLUDED
#define CRENVK_PIPELINE_INCLUDED
#ifdef CREN_BUILD_WITH_VULKAN

#include "cren_defines.h"

#include <ctoolbox/shashtable.h>
#include <vecmath/vecmath.h>
#include <vulkan/vulkan.h>

/// @brief default pipeline name, used for hashtable look-ups
#define CREN_PIPELINE_QUAD_DEFAULT_NAME "Quad:Default"
#define CREN_PIPELINE_QUAD_PICKING_NAME "Quad:Picking"

/// @brief How many push constants at max may exist for a given pipeline
#define CREN_PIPELINE_PUSH_CONSTANTS_MAX 8 

/// @brief How many descriptors sets at max a layout binding may have
#define CREN_PIPELINE_DESCRIPTOR_SET_LAYOUT_BINDING_MAX 32 

/// @brief How many shader stages a pipeline may have, since we only support Vertex and Fragment for now, 2
#define CREN_PIPELINE_SHADER_STAGES_COUNT 2

/// @brief all types of shaders
typedef enum ShaderType 
{
	SHADER_TYPE_VERTEX = 0,
	SHADER_TYPE_FRAGMENT,
	SHADER_TYPE_COMPUTE,
	SHADER_TYPE_GEOMETRY,
	SHADER_TYPE_TESS_CTRL,
	SHADER_TYPE_TESS_EVAL
} ShaderType;

/// @brief all types of vertex components, custom attributes not supported, also only covering 1 of each type at the momment
typedef enum VertexComponent
{
	VERTEX_COMPONENT_POSITION = 0,
	VERTEX_COMPONENT_NORMAL,
	VERTEX_COMPONENT_UV_0,          // UV_1, UV_2 ...
	VERTEX_COMPONENT_COLOR_0,       // COLOR_1, COLOR_2 ...
	VERTEX_COMPONENT_JOINTS_0,      // JOINTS_1, JOINTS_2 ...
	VERTEX_COMPONENT_WEIGHTS_0,     // WEIGHTS_1, WEIGHTS_2, ...

	VERTEX_COMPONENTS_MAX
} VertexComponent;

/// @brief cren vertex structure
typedef struct Vertex
{
	align_as(16) float3 position;
	align_as(16) float3 normal;
	align_as(8)  float2 uv_0;
	align_as(16) float4 color_0;
	align_as(16) float4 joints_0;
	align_as(16) float4 weights_0;
} Vertex;

/// @brief vulkan shader
typedef struct vkShader
{
	const char* name;
	const char* path;
	ShaderType type;
	VkShaderModule shaderModule;
	VkPipelineShaderStageCreateInfo shaderStageCI;
} vkShader;

/// @brief holds information about a vulkan renderpass 
typedef struct vkRenderpass
{
	const char* name;
	VkSampleCountFlagBits msaa;
	VkFormat surfaceFormat;
	VkRenderPass renderPass;
	VkCommandPool commandPool;
	VkDescriptorPool descriptorPool;
	VkCommandBuffer commandBuffers[CREN_CONCURRENTLY_RENDERED_FRAMES];
	VkFramebuffer* framebuffers;
	uint32_t framebuffersCount;
} vkRenderpass;

/// @brief cren pipeline create info, needed data to create a pipeline
typedef struct vkPipelineCreateInfo
{
	vkRenderpass* renderpass;
	VkPipelineCache pipelineCache;
	vkShader vertexShader;
	vkShader fragmentShader;
	bool passingVertexData;
	bool alphaBlending;
	VkDescriptorSetLayoutBinding bindings[CREN_PIPELINE_DESCRIPTOR_SET_LAYOUT_BINDING_MAX];
	uint32_t bindingsCount;
	VkPushConstantRange pushConstants[CREN_PIPELINE_PUSH_CONSTANTS_MAX];
	uint32_t pushConstantsCount;
	VertexComponent vertexComponents[VERTEX_COMPONENTS_MAX];
	uint32_t vertexComponentsCount;
} vkPipelineCreateInfo;

/// @brief holds information about the pipeline
typedef struct vkPipeline
{
	vkRenderpass* renderpass;
	bool passingVertexData;
	bool alphaBlending;
	VkPipelineCache cache;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout layout;
	VkPipeline pipeline;
	VkVertexInputBindingDescription* bindingsDescription;
	VkVertexInputAttributeDescription* attributesDescription;
	uint32_t bindingsDescriptionCount;
	uint32_t attributesDescriptionCount;

	// auto-generated, can be modified before building pipeline
	VkPipelineShaderStageCreateInfo shaderStages[CREN_PIPELINE_SHADER_STAGES_COUNT];
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	VkPipelineInputAssemblyStateCreateInfo inputVertexAssemblyState;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineRasterizationStateCreateInfo rasterizationState;
	VkPipelineMultisampleStateCreateInfo multisampleState;
	VkPipelineDepthStencilStateCreateInfo depthStencilState;
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
	VkPipelineColorBlendStateCreateInfo colorBlendState;
} vkPipeline;

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief creates a vulkan pipeline
CREN_API VkResult crenvk_pipeline_create(VkDevice device, vkPipelineCreateInfo* ci, vkPipeline* outPipe);

/// @brief release all used resources by a pipeline
CREN_API void crenvk_pipeline_destroy(VkDevice device, vkPipeline* pipeline);

/// @brief builds a pipeline with it's setup configuration
CREN_API VkResult crenvk_pipeline_build(VkDevice device, vkPipeline* pipeline);

/// @brief creates a cren vulkan shader
CREN_API vkShader crenvk_pipeline_create_shader(VkDevice device, const char* name, const char* path, ShaderType type);

/// @brief releases all resources used by a renderpass, this is a func so it's more automatic than manually handling the struct
CREN_API void crenvk_pipeline_renderpass_release(VkDevice device, vkRenderpass* renderpass);

/// @brief creates the quad-related pipelines
CREN_API VkResult crenvk_pipeline_create_quad(shashtable* pipelines, vkRenderpass* usedRenderpass, vkRenderpass* pickingRenderpass, VkDevice device, const char* rootPath);

#ifdef __cplusplus 
}
#endif

#endif // CREN_BUILD_WITH_VULKAN
#endif // CRENVK_PIPELINE_INCLUDED