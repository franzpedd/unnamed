#ifndef CREN_VULKAN_BUFFER_INCLUDED
#define CREN_VULKAN_BUFFER_INCLUDED
#ifdef CREN_BUILD_WITH_VULKAN

#include "cren_defines.h"
#include <ctoolbox/darray.h>
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

// creation
CREN_API vkBuffer* crenvk_buffer_create(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, uint32_t frameCount);
CREN_API void crenvk_buffer_destroy(VkDevice device, vkBuffer* buffer);

// memory mapping
CREN_API VkResult crenvk_buffer_map(VkDevice device, vkBuffer* buffer, uint32_t frameIndex);
CREN_API VkResult crenvk_buffer_unmap(VkDevice device, vkBuffer* buffer, uint32_t frameIndex);

// data operations
CREN_API VkResult crenvk_buffer_copy(vkBuffer* buffer, uint32_t frameIndex, const void* data, VkDeviceSize size, VkDeviceSize offset);
CREN_API VkResult crenvk_buffer_flush(VkDevice device, vkBuffer* buffer, uint32_t frameIndex, VkDeviceSize size, VkDeviceSize offset);

// gpu copying
CREN_API void crenvk_buffer_command_copy(VkCommandBuffer commandBuffer, vkBuffer* srcBuffer, uint32_t srcFrameIndex, vkBuffer* dstBuffer, uint32_t dstFrameIndex, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset);


/*
/// @brief cren general buffer
typedef struct vkBuffer 
{
	bool mapped;
	VkBuffer* buffers;
	VkDeviceMemory* memories;
	darray* mappedData;
} vkBuffer;



/// @brief creates a buffer, usually used for more precisde mapped buffers
CREN_API vkBuffer* crenvk_buffer_create(VkDevice device, VkPhysicalDevice physicalDevice, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags, VkDeviceSize size);

/// @brief releases all elements and destroys the buffer
CREN_API void crenvk_buffer_destroy(vkBuffer* buffer, VkDevice device);

/// @brief maps the gpu memory of a buffer with cpu address
CREN_API VkResult crenvk_buffer_map(vkBuffer* buffer, uint32_t index, VkDevice device);

/// @brief unmaps the gpu-cpu memory
CREN_API VkResult crenvk_buffer_unmap(vkBuffer* buffer, uint32_t index, VkDevice device);

/// @brief flush/invalidate functions for non-coherent memory
CREN_API void crenvk_buffer_flush(vkBuffer* buffer, VkDevice device, VkDeviceSize offset, VkDeviceSize size);

/// @brief copies data to a mapped buffer (assumes buffer is already mapped)
CREN_API VkResult crenvk_buffer_copy(vkBuffer* buffer, unsigned int index, const void* src, unsigned long long size);

/// @brief performs a buffer-to-buffer copying
CREN_API void crenvk_buffer_command_copy(VkCommandBuffer cmd, vkBuffer* srcBuffer, vkBuffer* dstBuffer, unsigned int srcIndex, unsigned int dstIndex, const VkBufferCopy* region);
*/

#ifdef __cplusplus 
}
#endif

#endif // CREN_BUILD_WITH_VULKAN
#endif // CREN_VULKAN_BUFFER_INCLUDED

