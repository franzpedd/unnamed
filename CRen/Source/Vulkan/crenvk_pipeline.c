#include "crenvk_pipeline.h"

#include "cren_defines.h"
#include "cren_error.h"
#include "cren_platform.h"
#include "crenvk_buffer.h"

#include <memm/memm.h>

#ifdef CREN_BUILD_WITH_VULKAN

/// @brief creates an array of VkVertexInputBindingDescription based on parameters
static VkVertexInputBindingDescription* internal_crenvk_pipeline_get_binding_descriptions(bool passingVertexData, uint32_t* bindingCount)
{
	if (!passingVertexData) {
		*bindingCount = 0U;
		return NULL;
	}

	VkVertexInputBindingDescription* bindings = (VkVertexInputBindingDescription*)malloc(sizeof(VkVertexInputBindingDescription));
	bindings[0].binding = 0;
	bindings[0].stride = sizeof(CRenVertex);
	bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	*bindingCount = 1U;
	return bindings;
}

/// @brief creates an array of VkVertexInputAttributeDescription
static VkVertexInputAttributeDescription* internal_crenvk_get_attribute_descriptions(CRen_VertexComponent* vertexComponents, uint32_t componentsCount, uint32_t* attributesCount)
{
	VkVertexInputAttributeDescription* bindings = (VkVertexInputAttributeDescription*)malloc(sizeof(VkVertexInputAttributeDescription) * componentsCount);

	for (uint32_t i = 0; i < componentsCount; i++) {
		CRen_VertexComponent component = vertexComponents[i];
		VkVertexInputAttributeDescription desc = { 0 };
		desc.binding = 0;
		desc.location = (uint32_t)component;

		switch (component)
		{
			case VERTEX_COMPONENT_POSITION:
			{
				desc.format = VK_FORMAT_R32G32B32_SFLOAT;
				desc.offset = offsetof(CRenVertex, position);
				break;
			}

			case VERTEX_COMPONENT_NORMAL:
			{
				desc.format = VK_FORMAT_R32G32B32_SFLOAT;
				desc.offset = offsetof(CRenVertex, normal);
				break;
			}

			case VERTEX_COMPONENT_UV_0:
			{
				desc.format = VK_FORMAT_R32G32_SFLOAT;
				desc.offset = offsetof(CRenVertex, uv_0);
				break;
			}

			case VERTEX_COMPONENT_COLOR_0:
			{
				desc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				desc.offset = offsetof(CRenVertex, color_0);
				break;
			}

			case VERTEX_COMPONENT_WEIGHTS_0:
			{
				desc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				desc.offset = offsetof(CRenVertex, weights_0);
				break;
			}

			case VERTEX_COMPONENT_JOINTS_0:
			{
				desc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				desc.offset = offsetof(CRenVertex, joints_0);
				break;
			}

			default: break;
		}
		bindings[i] = desc;
	}
	*attributesCount = componentsCount;
	return bindings;
}


static VkPipelineVertexInputStateCreateInfo internal_crenvk_pipeline_populate_visci(vkPipeline* pipeline, CRen_VertexComponent* vertexComponents, uint32_t componentsCount)
{
	pipeline->bindingsDescription = internal_crenvk_pipeline_get_binding_descriptions(pipeline->passingVertexData, &pipeline->bindingsDescriptionCount);
	pipeline->attributesDescription = internal_crenvk_get_attribute_descriptions(vertexComponents, componentsCount, &pipeline->attributesDescriptionCount);

	VkPipelineVertexInputStateCreateInfo visci = { 0 };
	visci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	visci.pNext = NULL;
	visci.flags = 0;
	visci.vertexBindingDescriptionCount = pipeline->bindingsDescriptionCount;
	visci.pVertexBindingDescriptions = pipeline->bindingsDescription;
	visci.vertexAttributeDescriptionCount = pipeline->attributesDescriptionCount;
	visci.pVertexAttributeDescriptions = pipeline->attributesDescription;

	return visci;
}

CREN_API VkResult crenvk_pipeline_create(VkDevice device, vkPipelineCreateInfo* ci, vkPipeline* outPipe)
{
	if (device == VK_NULL_HANDLE || !ci || !outPipe) return VK_ERROR_INITIALIZATION_FAILED;

	outPipe->passingVertexData = ci->passingVertexData;
	outPipe->cache = ci->pipelineCache;
	outPipe->shaderStages[0] = ci->vertexShader.shaderStageCI;
	outPipe->shaderStages[1] = ci->fragmentShader.shaderStageCI;
	outPipe->renderpass = ci->renderpass;

	// descriptor set
	VkDescriptorSetLayoutCreateInfo descSetLayoutCI = { 0 };
	descSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descSetLayoutCI.pNext = NULL;
	descSetLayoutCI.flags = 0;
	descSetLayoutCI.bindingCount = ci->bindingsCount;
	descSetLayoutCI.pBindings = ci->bindings;
	CREN_ASSERT(vkCreateDescriptorSetLayout(device, &descSetLayoutCI, NULL, &outPipe->descriptorSetLayout) == VK_SUCCESS, "Failed to create descriptor set layout");

	// pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCI = { 0 };
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.pNext = NULL;
	pipelineLayoutCI.flags = 0;
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &outPipe->descriptorSetLayout;
	pipelineLayoutCI.pushConstantRangeCount = ci->pushConstantsCount;
	pipelineLayoutCI.pPushConstantRanges = ci->pushConstants;
	CREN_ASSERT(vkCreatePipelineLayout(device, &pipelineLayoutCI, NULL, &outPipe->layout) == VK_SUCCESS, "Failed to create pipeline layout");

	// vertex input state
	outPipe->vertexInputState = internal_crenvk_pipeline_populate_visci(outPipe, ci->vertexComponents, ci->vertexComponentsCount);
	// input vertex assembly state
	outPipe->inputVertexAssemblyState = (VkPipelineInputAssemblyStateCreateInfo){ 0 };
	outPipe->inputVertexAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	outPipe->inputVertexAssemblyState.pNext = NULL;
	outPipe->inputVertexAssemblyState.flags = 0;
	outPipe->inputVertexAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	outPipe->inputVertexAssemblyState.primitiveRestartEnable = VK_FALSE;
	// viewport state
	outPipe->viewportState = (VkPipelineViewportStateCreateInfo){ 0 };
	outPipe->viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	outPipe->viewportState.pNext = NULL;
	outPipe->viewportState.flags = 0;
	outPipe->viewportState.viewportCount = 1;
	outPipe->viewportState.pViewports = NULL; // using dynamic viewport
	outPipe->viewportState.scissorCount = 1;
	outPipe->viewportState.pScissors = NULL; // using dynamic scissor
	// rasterization state
	outPipe->rasterizationState = (VkPipelineRasterizationStateCreateInfo){ 0 };
	outPipe->rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	outPipe->rasterizationState.pNext = NULL;
	outPipe->rasterizationState.flags = 0;
	outPipe->rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	outPipe->rasterizationState.cullMode = VK_CULL_MODE_NONE;
	outPipe->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	outPipe->rasterizationState.depthClampEnable = VK_FALSE;
	outPipe->rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	outPipe->rasterizationState.lineWidth = 1.0f;
	// multisampling state
	outPipe->multisampleState = (VkPipelineMultisampleStateCreateInfo){ 0 };
	outPipe->multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	outPipe->multisampleState.pNext = NULL;
	outPipe->multisampleState.flags = 0;
	outPipe->multisampleState.rasterizationSamples = ci->renderpass->msaa;
	outPipe->multisampleState.sampleShadingEnable = VK_FALSE;
	// depth stencil state
	outPipe->depthStencilState = (VkPipelineDepthStencilStateCreateInfo){ 0 };
	outPipe->depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	outPipe->depthStencilState.pNext = NULL;
	outPipe->depthStencilState.flags = 0;
	outPipe->depthStencilState.depthTestEnable = VK_TRUE;
	outPipe->depthStencilState.depthWriteEnable = VK_TRUE;
	outPipe->depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	outPipe->depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
	// color blend attachment
	outPipe->colorBlendAttachmentState = (VkPipelineColorBlendAttachmentState){ 0 };
	outPipe->colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	outPipe->colorBlendAttachmentState.blendEnable = ci->alphaBlending == true ? VK_TRUE : VK_FALSE;
	outPipe->colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	outPipe->colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	outPipe->colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	outPipe->colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	outPipe->colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	outPipe->colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	// color blend state
	outPipe->colorBlendState = (VkPipelineColorBlendStateCreateInfo){ 0 };
	outPipe->colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	outPipe->colorBlendState.pNext = NULL;
	outPipe->colorBlendState.flags = 0;
	outPipe->colorBlendState.attachmentCount = 1;
	outPipe->colorBlendState.pAttachments = &outPipe->colorBlendAttachmentState;
	outPipe->colorBlendState.logicOpEnable = VK_FALSE;
	outPipe->colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	outPipe->colorBlendState.blendConstants[0] = 0.0f;
	outPipe->colorBlendState.blendConstants[1] = 0.0f;
	outPipe->colorBlendState.blendConstants[2] = 0.0f;
	outPipe->colorBlendState.blendConstants[3] = 0.0f;

	return VK_SUCCESS;
}

CREN_API void crenvk_pipeline_destroy(VkDevice device, vkPipeline* pipeline)
{
	if (!pipeline || device == VK_NULL_HANDLE) return;

	vkDeviceWaitIdle(device);

	vkDestroyPipeline(device, pipeline->pipeline, NULL);
	vkDestroyPipelineLayout(device, pipeline->layout, NULL); 
	vkDestroyDescriptorSetLayout(device, pipeline->descriptorSetLayout, NULL);

	if (pipeline->bindingsDescription != NULL) free(pipeline->bindingsDescription);
	if (pipeline->attributesDescription != NULL) free(pipeline->attributesDescription);

	// not ideal since shader module was first introduced on shader struct, but it's the same module after-all
	vkDestroyShaderModule(device, pipeline->shaderStages[0].module, NULL);
	vkDestroyShaderModule(device, pipeline->shaderStages[1].module, NULL);

	free(pipeline);
}

CREN_API VkResult crenvk_pipeline_build(VkDevice device, vkPipeline* pipeline)
{
	// dynamic state is here because dynamic states must be constant
	VkPipelineDynamicStateCreateInfo dynamicState = { 0 };
	const VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext = NULL;
	dynamicState.flags = 0;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkGraphicsPipelineCreateInfo ci = { 0 };
	ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	ci.pNext = NULL;
	ci.flags = 0;
	ci.stageCount = CREN_PIPELINE_SHADER_STAGES_COUNT;
	ci.pStages = pipeline->shaderStages;
	ci.pVertexInputState = &pipeline->vertexInputState;
	ci.pInputAssemblyState = &pipeline->inputVertexAssemblyState;
	ci.pViewportState = &pipeline->viewportState;
	ci.pRasterizationState = &pipeline->rasterizationState;
	ci.pMultisampleState = &pipeline->multisampleState;
	ci.pDepthStencilState = &pipeline->depthStencilState;
	ci.pColorBlendState = &pipeline->colorBlendState;
	ci.pDynamicState = &dynamicState;
	ci.layout = pipeline->layout;
	ci.renderPass = pipeline->renderpass->renderPass;
	ci.subpass = 0;

	VkResult res = vkCreateGraphicsPipelines(device, pipeline->cache, 1, &ci, NULL, &pipeline->pipeline);
	if (res != VK_SUCCESS) {
		CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to build the graphics pipeline {%d}", res);
		return res;
	}

	return VK_SUCCESS;
}

CREN_API vkShader crenvk_pipeline_create_shader(VkDevice device, const char* name, const char* path, CRen_ShaderType type)
{
	vkShader shader = { 0 };
    shader.name = name;
    shader.path = path;
    shader.type = type;
    shader.shaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader.shaderStageCI.pNext = NULL;
    shader.shaderStageCI.flags = 0;
    shader.shaderStageCI.pName = "main";
    shader.shaderStageCI.pSpecializationInfo = NULL;
    switch (type)
	{
        case SHADER_TYPE_VERTEX: { shader.shaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT; break; }
        case SHADER_TYPE_FRAGMENT: { shader.shaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break; }
        case SHADER_TYPE_COMPUTE: { shader.shaderStageCI.stage = VK_SHADER_STAGE_COMPUTE_BIT; break; }
        case SHADER_TYPE_GEOMETRY: { shader.shaderStageCI.stage = VK_SHADER_STAGE_GEOMETRY_BIT; break; }
        case SHADER_TYPE_TESS_CTRL: { shader.shaderStageCI.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break; }
        case SHADER_TYPE_TESS_EVAL: { shader.shaderStageCI.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break; }
        default: { break; }
    }

    size_t spirvSize = 0;
    unsigned int* spirvCode = cren_load_file(path, &spirvSize);
    if (spirvCode == NULL) CREN_LOG(CREN_LOG_SEVERITY_ERROR, "SPIR-V code is NULL, therefore could not load file");

    VkShaderModuleCreateInfo moduleCI = { 0 };
    moduleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCI.pNext = NULL;
    moduleCI.flags = 0;
    moduleCI.codeSize = spirvSize;
    moduleCI.pCode = spirvCode;
    CREN_ASSERT(vkCreateShaderModule(device, &moduleCI, NULL, &shader.shaderStageCI.module) == VK_SUCCESS, "Failed to create shader module");
    
    free(spirvCode);
    return shader;
}

CREN_API void crenvk_pipeline_renderpass_release(VkDevice device, vkRenderpass* renderpass)
{
	vkDeviceWaitIdle(device);

	if (renderpass->renderPass != VK_NULL_HANDLE) vkDestroyRenderPass(device, renderpass->renderPass, NULL);
	if (renderpass->commandBuffers) vkFreeCommandBuffers(device, renderpass->commandPool, CREN_CONCURRENTLY_RENDERED_FRAMES, renderpass->commandBuffers);
	if (renderpass->commandPool != VK_NULL_HANDLE) vkDestroyCommandPool(device, renderpass->commandPool, NULL);

	for (unsigned int i = 0; i < renderpass->framebuffersCount; i++) {
		vkDestroyFramebuffer(device, renderpass->framebuffers[i], NULL);
	}
	free(renderpass->framebuffers);
	free(renderpass);
}

CREN_API VkResult crenvk_pipeline_create_quad(shashtable* pipelines, vkRenderpass* usedRenderpass, vkRenderpass* pickingRenderpass, VkDevice device, const char* rootPath)
{
	// default pipeline
	vkPipeline* defaultPipeline = (vkPipeline*)shashtable_lookup(pipelines, CREN_PIPELINE_QUAD_DEFAULT_NAME);
	if (defaultPipeline != NULL) crenvk_pipeline_destroy(device, defaultPipeline);

	char defaultVert[CREN_PATH_MAX_SIZE], defaultFrag[CREN_PATH_MAX_SIZE];
	cren_get_path("shaders/compiled/quad.vert.spv", rootPath, 0, defaultVert, sizeof(defaultVert));
	cren_get_path("shaders/compiled/quad.frag.spv", rootPath, 0, defaultFrag, sizeof(defaultFrag));

	vkPipelineCreateInfo ci = { 0 };
	ci.renderpass = usedRenderpass; // this will either be default or viewport renderpass
	ci.vertexShader = crenvk_pipeline_create_shader(device, "quad.vert", defaultVert, SHADER_TYPE_VERTEX);
	ci.fragmentShader = crenvk_pipeline_create_shader(device, "quad.frag", defaultFrag, SHADER_TYPE_FRAGMENT);
	ci.passingVertexData = false;
	ci.alphaBlending = true;

	// push constant
	ci.pushConstantsCount = 1;
	ci.pushConstants[0].offset = 0;
	ci.pushConstants[0].size = sizeof(vkBufferPushConstant);
	ci.pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	// bindings
	ci.bindingsCount = 3;
	// camera data
	ci.bindings[0].binding = 0;
	ci.bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ci.bindings[0].descriptorCount = 1;
	ci.bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	ci.bindings[0].pImmutableSamplers = NULL;
	// quad data
	ci.bindings[1].binding = 1;
	ci.bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ci.bindings[1].descriptorCount = 1;
	ci.bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	ci.bindings[1].pImmutableSamplers = NULL;
	// colormap
	ci.bindings[2].binding = 2;
	ci.bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ci.bindings[2].descriptorCount = 1;
	ci.bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	ci.bindings[2].pImmutableSamplers = NULL;

	defaultPipeline = (vkPipeline*)malloc(sizeof(vkPipeline));
	VkResult res = crenvk_pipeline_create(device, &ci, defaultPipeline);
	if (res != VK_SUCCESS || defaultPipeline == NULL) {
		CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create quad default pipeline");
		return res;
	}

	defaultPipeline->rasterizationState.cullMode = VK_CULL_MODE_NONE;

	res = crenvk_pipeline_build(device, defaultPipeline);
	if (res != VK_SUCCESS) {
		CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to build quad default pipeline");
		return res;
	}

	ctoolbox_result ctoolres = shashtable_insert(pipelines, CREN_PIPELINE_QUAD_DEFAULT_NAME, defaultPipeline);
	CREN_ASSERT(ctoolres == CTOOLBOX_SUCCESS, "Failed to insert quad default pipeline into pipeline's library");

	// picking pipeline
	vkPipeline* pickingPipeline = (vkPipeline*)shashtable_lookup(pipelines, CREN_PIPELINE_QUAD_PICKING_NAME);
	if (pickingPipeline != NULL) crenvk_pipeline_destroy(device, pickingPipeline);

	char pickingVert[CREN_PATH_MAX_SIZE], pickingFrag[CREN_PATH_MAX_SIZE];
	cren_get_path("shaders/compiled/quad_picking.vert.spv", rootPath, 0, pickingVert, sizeof(pickingVert));
	cren_get_path("shaders/compiled/quad_picking.frag.spv", rootPath, 0, pickingFrag, sizeof(pickingFrag));

	ci = (vkPipelineCreateInfo){ 0 };
	ci.renderpass = pickingRenderpass;
	ci.vertexShader = crenvk_pipeline_create_shader(device, "quad.vert", pickingVert, SHADER_TYPE_VERTEX);
	ci.fragmentShader = crenvk_pipeline_create_shader(device, "quad.frag", pickingFrag, SHADER_TYPE_FRAGMENT);
	ci.passingVertexData = false;
	ci.alphaBlending = false;

	// push constant
	ci.pushConstantsCount = 1;
	ci.pushConstants[0].offset = 0;
	ci.pushConstants[0].size = sizeof(vkBufferPushConstant);
	ci.pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	// bindings
	ci.bindingsCount = 3;
	// camera data
	ci.bindings[0].binding = 0;
	ci.bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ci.bindings[0].descriptorCount = 1;
	ci.bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	ci.bindings[0].pImmutableSamplers = NULL;
	// quad data
	ci.bindings[1].binding = 1;
	ci.bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ci.bindings[1].descriptorCount = 1;
	ci.bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	ci.bindings[1].pImmutableSamplers = NULL;
	// colormap
	ci.bindings[2].binding = 2;
	ci.bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ci.bindings[2].descriptorCount = 1;
	ci.bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	ci.bindings[2].pImmutableSamplers = NULL;

	pickingPipeline = (vkPipeline*)malloc(sizeof(vkPipeline));
	res = crenvk_pipeline_create(device, &ci, pickingPipeline);
	if (res != VK_SUCCESS || pickingPipeline == NULL) {
		CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create quad picking pipeline");
		return res;
	}

	pickingPipeline->rasterizationState.cullMode = VK_CULL_MODE_NONE;
	pickingPipeline->colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT; // id's are RED channel only

	res = crenvk_pipeline_build(device, pickingPipeline);
	if (res != VK_SUCCESS) {
		CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to build quad picking pipeline");
		return res;
	}

	ctoolres = shashtable_insert(pipelines, CREN_PIPELINE_QUAD_PICKING_NAME, pickingPipeline);
	CREN_ASSERT(ctoolres == CTOOLBOX_SUCCESS, "Failed to insert quad picking pipeline into pipeline's library");

	CREN_LOG(CREN_LOG_SEVERITY_TODO, "Create wireframe pipeline");
	return VK_SUCCESS;
}

#endif // CREN_BUILD_WITH_VULKAN
