#ifndef CRENVK_PRIMITIVES
#define CRENVK_PRIMITIVES
#ifdef CREN_BUILD_WITH_VULKAN

#include "cren_defines.h"
#include "cren_types.h"
#include "crenvk_context.h"
#include "crenvk_buffer.h"

#ifdef __cplusplus 
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture 2D
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief this structure holds all information about a vulkan 2d texture
typedef struct CRenVKTexture2D
{
	VkImage image;
	VkDeviceMemory memory;
	VkSampler sampler;
	VkImageView view;
	VkDescriptorSet uiDescriptor;
} CRenVKTexture2D;

/// @brief creates a 2d texture based on disk path
CREN_API VkResult crenvk_texture2d_create_from_path(CRenVulkanBackend* backend, const char* path, bool uiTexture, bool usingCustomViewport, CRenVKTexture2D** out_texture, int32_t* out_width, int32_t* out_height, int32_t* out_mipLevels);

/// @brief creates a 2d texture based on buffer data
CREN_API VkResult crenvk_texture2d_create_from_buffer(CRenVulkanBackend* backend, uint8_t* buffer, size_t bufferLen, int width, int height, bool uiTexture, bool usingCustomViewport, CRenVKTexture2D** out_texture, int32_t* out_mipLevels);

/// @brief releases all resources used by a 2d texture
CREN_API void crenvk_texture2d_destroy(CRenVulkanBackend* backend, CRenVKTexture2D* texture);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quad
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief this structure holds all necessary data for a vulkan quad
typedef struct CRenVKQuad
{
	vkQuadBufferParams params;
	vkBuffer* buffer;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSets[CREN_CONCURRENTLY_RENDERED_FRAMES];
} CRenVKQuad;

/// @brief creates a quad with vulkan backend
CREN_API VkResult crenvk_quad_create_from_path(CRenVulkanBackend* backend, const char* albedoPath, bool usingCustomViewport, CRenVKQuad** out_quad);

/// @brief releases all resources used by a quad
CREN_API void crenvk_quad_destroy(CRenVulkanBackend* backend, CRenVKQuad* quad);

/// @brief re-upload modified data to the gpu
CREN_API void crenvk_quad_update(CRenVulkanBackend* backend, CRenVKQuad* quad);

/// @brief renders the quad into the world
CREN_API void crenvk_quad_render(CRenVulkanBackend* backend, CRenVKQuad* quad, CRen_RenderStage stage, const fmat4 modelMatrix, uint32_t id, bool usingCustomViewport);

/// @breif re-updates the descriptorsets with new informations since last update
CREN_API void crenvk_quad_update_descriptors(CRenVulkanBackend* backend, CRenVKQuad* quad, CRenVKTexture2D* albedoTexture);

#ifdef __cplusplus 
}
#endif

#endif // CREN_BUILD_WITH_VULKAN
#endif // CRENVK_PRIMITIVES