#ifndef CREN_VULKAN_BUFFER_INCLUDED
#define CREN_VULKAN_BUFFER_INCLUDED
#ifdef CREN_BUILD_WITH_VULKAN

#include "cren_defines.h"
#include <vecmath/vecmath.h>
#include <vulkan/vulkan.h>

/// @brief cren push-constant buffer
typedef struct vkBufferPushConstant 
{
	align_as(4) uint32_t id;
	align_as(16) fmat4 model;
} vkBufferPushConstant;

/// @brief cren camera buffer
typedef struct vkBufferCamera 
{
	align_as(16) fmat4 view;
	align_as(16) fmat4 viewInverse;
	align_as(16) fmat4 proj;
} vkBufferCamera;

/// @brief cren quad buffer
typedef struct vkQuadBufferParams
{
	align_as(4) uint32_t billboard;	    // hints to always faces the camera
	align_as(4) float uv_rotation;		// rotates the uv/texture
	align_as(8) float2 lockAxis;	    // controls wich axis to lock
	align_as(8) float2 uv_offset;	    // used to offset the uv/texture
	align_as(8) float2 uv_scale;	    // used to scale the uv/texture
} vkQuadBufferParams;

/// @brief cren general buffer
typedef struct vkBuffer
{
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags memoryProperties;

    // per-frame resources
    VkBuffer* buffers;
    VkDeviceMemory* memories;
    void** mappedPointers;
    bool* isMapped;

    uint32_t frameCount;
} vkBuffer;

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief creates the buffer object
CREN_API vkBuffer* crenvk_buffer_create(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, uint32_t frameCount);

/// @brief destroys the buffer object
CREN_API void crenvk_buffer_destroy(VkDevice device, vkBuffer* buffer);

/// @brief maps gpu the buffer to a cpu-visible buffer
CREN_API VkResult crenvk_buffer_map(VkDevice device, vkBuffer* buffer, uint32_t frameIndex);

/// @brief unmaps gpu the buffer
CREN_API VkResult crenvk_buffer_unmap(VkDevice device, vkBuffer* buffer, uint32_t frameIndex);

/// @brief copies a buffer within gpu memory to cpu memory
CREN_API VkResult crenvk_buffer_copy(vkBuffer* buffer, uint32_t frameIndex, const void* data, VkDeviceSize size, VkDeviceSize offset);

/// @brief flushes a buffer
CREN_API VkResult crenvk_buffer_flush(VkDevice device, vkBuffer* buffer, uint32_t frameIndex, VkDeviceSize size, VkDeviceSize offset);

/// @brief copies a gpu buffer within gpu
CREN_API void crenvk_buffer_command_copy(VkCommandBuffer commandBuffer, vkBuffer* srcBuffer, uint32_t srcFrameIndex, vkBuffer* dstBuffer, uint32_t dstFrameIndex, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset);

#ifdef __cplusplus 
}
#endif

#endif // CREN_BUILD_WITH_VULKAN
#endif // CREN_VULKAN_BUFFER_INCLUDED

