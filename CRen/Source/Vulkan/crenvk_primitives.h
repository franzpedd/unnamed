#ifndef CRENVK_PRIMITIVES
#define CRENVK_PRIMITIVES
#ifdef CREN_BUILD_WITH_VULKAN

#include "cren_defines.h"
#include "cren_types.h"
#include "crenvk_buffer.h"

/// @brief this structure holds all information about a vulkan 2d texture
typedef struct vkTexture2D
{
	VkImage image;
	VkDeviceMemory memory;
	VkSampler sampler;
	VkImageView view;
	VkDescriptorSet uiDescriptor;
	char path[CREN_PATH_MAX_SIZE];
	int32_t width;
	int32_t height;
	int32_t mipLevels;
} vkTexture2D;

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief creates a 2d texture based on disk path
CREN_API VkResult crenvk_create_texture2d_from_path(CRenContext* context, vkTexture2D* out_tex, const char* path, bool uiTexture);


#ifdef __cplusplus 
}
#endif

#endif // CREN_BUILD_WITH_VULKAN
#endif // CRENVK_PRIMITIVES