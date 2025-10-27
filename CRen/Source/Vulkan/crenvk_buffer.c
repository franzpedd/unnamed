#include "Vulkan/crenvk_buffer.h"

#include "cren_error.h"
#include "Vulkan/crenvk_core.h"
#include <memm/memm.h>
#include <string.h>

#if defined (CREN_BUILD_WITH_VULKAN)

CREN_API vkBuffer* crenvk_buffer_create(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, uint32_t frameCount)
{
    if (size == 0 || frameCount == 0) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Invalid buffer size or frame count");    
        return NULL;
    }

    vkBuffer* buffer = (vkBuffer*)malloc(sizeof(vkBuffer));
    if (!buffer) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate buffer structure");
        return NULL;
    }

    buffer->size = size;
    buffer->usage = usage;
    buffer->memoryProperties = memoryProperties;
    buffer->frameCount = frameCount;

    // allocate arrays for per-frame resources
    buffer->buffers = (VkBuffer*)malloc(sizeof(VkBuffer) * frameCount);
    buffer->memories = (VkDeviceMemory*)malloc(sizeof(VkDeviceMemory) * frameCount);
    buffer->mappedPointers = (void**)malloc(sizeof(void*) * frameCount);
    buffer->isMapped = (bool*)malloc(sizeof(bool) * frameCount);

    if (!buffer->buffers || !buffer->memories || !buffer->mappedPointers || !buffer->isMapped) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate buffer arrays");
        crenvk_buffer_destroy(device, buffer);
        return NULL;
    }

    // initialize arrays
    memset(buffer->mappedPointers, 0, sizeof(void*) * frameCount);
    memset(buffer->isMapped, 0, sizeof(bool) * frameCount);

    // create each buffer
    for (uint32_t i = 0; i < frameCount; i++)
    {
        VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(device, &bufferInfo, NULL, &buffer->buffers[i]);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create buffer %u: %d", i, result);
            crenvk_buffer_destroy(device, buffer);
            return NULL;
        }

        // get memory requirements
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer->buffers[i], &memRequirements);

        // find memory type
        VkMemoryAllocateInfo allocInfo = { 0 };
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = crenvk_device_find_memory_type(physicalDevice,memRequirements.memoryTypeBits,memoryProperties);

        result = vkAllocateMemory(device, &allocInfo, NULL, &buffer->memories[i]);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate buffer memory %u: %d", i, result);
            crenvk_buffer_destroy(device, buffer);
            return NULL;
        }

        // bind memory
        result = vkBindBufferMemory(device, buffer->buffers[i], buffer->memories[i], 0);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to bind buffer memory %u: %d", i, result);
            crenvk_buffer_destroy(device, buffer);
            return NULL;
        }

        // auto-map if host visible
        if (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            result = crenvk_buffer_map(device, buffer, i);
            if (result != VK_SUCCESS) {
                CREN_LOG(CREN_LOG_SEVERITY_WARN, "Failed to auto-map buffer %u: %d", i, result);
            }
        }
    }

    return buffer;
}

CREN_API void crenvk_buffer_destroy(VkDevice device, vkBuffer* buffer)
{
    if (!buffer) return;

    if (buffer->buffers) {
        for (uint32_t i = 0; i < buffer->frameCount; i++) {
            if (buffer->buffers[i] != VK_NULL_HANDLE) {
                // unmap if mapped
                if (buffer->isMapped[i]) {
                    crenvk_buffer_unmap(device, buffer, i);
                }

                vkDestroyBuffer(device, buffer->buffers[i], NULL);
                buffer->buffers[i] = VK_NULL_HANDLE;
            }
        }
        free(buffer->buffers);
    }

    if (buffer->memories) {
        for (uint32_t i = 0; i < buffer->frameCount; i++) {
            if (buffer->memories[i] != VK_NULL_HANDLE) {
                vkFreeMemory(device, buffer->memories[i], NULL);
                buffer->memories[i] = VK_NULL_HANDLE;
            }
        }
        free(buffer->memories);
    }

    if (buffer->mappedPointers) free(buffer->mappedPointers);
    if (buffer->isMapped) free(buffer->isMapped);

    free(buffer);
}

CREN_API VkResult crenvk_buffer_map(VkDevice device, vkBuffer* buffer, uint32_t frameIndex)
{
    if (!buffer || frameIndex >= buffer->frameCount) return VK_ERROR_INITIALIZATION_FAILED;
    if (buffer->isMapped[frameIndex]) return VK_SUCCESS; // Already mapped

    if (!(buffer->memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Cannot map non-host-visible buffer");
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    VkResult result = vkMapMemory(device, buffer->memories[frameIndex], 0, buffer->size, 0, &buffer->mappedPointers[frameIndex]);
    if (result == VK_SUCCESS) buffer->isMapped[frameIndex] = true;
    else CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to map buffer: %d", result);

    return result;
}

CREN_API VkResult crenvk_buffer_unmap(VkDevice device, vkBuffer* buffer, uint32_t frameIndex)
{
    if (!buffer || frameIndex >= buffer->frameCount) return VK_ERROR_INITIALIZATION_FAILED;
    if (!buffer->isMapped[frameIndex]) return VK_SUCCESS; // not mapped

    vkUnmapMemory(device, buffer->memories[frameIndex]);
    buffer->mappedPointers[frameIndex] = NULL;
    buffer->isMapped[frameIndex] = false;

    return VK_SUCCESS;
}

CREN_API VkResult crenvk_buffer_copy(vkBuffer* buffer, uint32_t frameIndex, const void* data, VkDeviceSize size, VkDeviceSize offset)
{
    if (!buffer || !data || size == 0) return VK_ERROR_INITIALIZATION_FAILED;

    if (frameIndex >= buffer->frameCount) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Frame index %u out of bounds", frameIndex);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (offset + size > buffer->size) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Copy exceeds buffer size");
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    if (!buffer->isMapped[frameIndex]) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Buffer not mapped at frame %u", frameIndex);
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    memcpy((char*)buffer->mappedPointers[frameIndex] + offset, data, size);
    return VK_SUCCESS;
}

CREN_API VkResult crenvk_buffer_flush(VkDevice device, vkBuffer* buffer, uint32_t frameIndex, VkDeviceSize size, VkDeviceSize nonCoherentAtomSize, VkDeviceSize offset)
{
    if (!buffer || frameIndex >= buffer->frameCount) return VK_ERROR_INITIALIZATION_FAILED;

    if (!(buffer->memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        VkMappedMemoryRange memoryRange = { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE };
        memoryRange.memory = buffer->memories[frameIndex];

        VkDeviceSize atomSize = nonCoherentAtomSize;
        VkDeviceSize alignedOffset = offset & ~(atomSize - 1);
        VkDeviceSize end = offset + size;
        VkDeviceSize alignedEnd = (end + atomSize - 1) & ~(atomSize - 1);
        VkDeviceSize alignedSize = alignedEnd - alignedOffset;

        // clamp to buffer size
        if (alignedOffset + alignedSize > buffer->size) {
            alignedSize = buffer->size - alignedOffset;
        }

        memoryRange.offset = alignedOffset;
        memoryRange.size = alignedSize;

        return vkFlushMappedMemoryRanges(device, 1, &memoryRange);
    }

    return VK_SUCCESS;
}

CREN_API void crenvk_buffer_command_copy(VkCommandBuffer commandBuffer, vkBuffer* srcBuffer, uint32_t srcFrameIndex, vkBuffer* dstBuffer, uint32_t dstFrameIndex, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset)
{
    if (!srcBuffer || !dstBuffer || srcFrameIndex >= srcBuffer->frameCount || dstFrameIndex >= dstBuffer->frameCount) return;

    VkBufferCopy copyRegion = { 0 };
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = (size == VK_WHOLE_SIZE) ? srcBuffer->size - srcOffset : size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer->buffers[srcFrameIndex], dstBuffer->buffers[dstFrameIndex], 1, &copyRegion);
}

#endif // CREN_BUILD_WITH_VULKAN