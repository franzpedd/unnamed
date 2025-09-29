#include "Vulkan/crenvk_buffer.h"

#include "cren_error.h"
#include "Vulkan/crenvk_core.h"
#include <memm/memm.h>
#include <string.h>

CREN_API vkBuffer* crenvk_buffer_create(VkDevice device, VkPhysicalDevice physicalDevice, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags, VkDeviceSize size)
{
    vkBuffer* buffer = (vkBuffer*)malloc(sizeof(vkBuffer));
    if (!buffer) return NULL;

    buffer->mapped = false;

    // allocate buffers
    buffer->buffers = (VkBuffer*)malloc(sizeof(VkBuffer) * CREN_CONCURRENTLY_RENDERED_FRAMES);
    if (!buffer->buffers) {
        CREN_LOG(CRenLogSeverity_Error, "Failed to allocate memory for Vulkan buffers");
        free(buffer);
        return NULL;
    }

    // allocate memories
    buffer->memories = (VkDeviceMemory*)malloc(sizeof(VkDeviceMemory) * CREN_CONCURRENTLY_RENDERED_FRAMES);
    if (!buffer->memories) {
        CREN_LOG(CRenLogSeverity_Error, "Failed to allocate memory for Vulkan buffer memories");
        free(buffer->buffers);
        free(buffer);
        return NULL;
    }

    // initialize mappedData array (all entries NULL by default)
    buffer->mappedData = darray_init(sizeof(void*), CREN_CONCURRENTLY_RENDERED_FRAMES);
    if (!buffer->mappedData) {
        CREN_LOG(CRenLogSeverity_Error, "Failed to create mapped data array");
        free(buffer->memories);
        free(buffer->buffers);
        free(buffer);
        return NULL;
    }

    // create buffers and allocate memory but don't map yet
    for (uint32_t i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
        VkResult res = crenvk_device_create_buffer(device, physicalDevice, usageFlags, memoryFlags, size, &buffer->buffers[i], &buffer->memories[i], NULL);
        if (res != VK_SUCCESS) {
            CREN_LOG(CRenLogSeverity_Error, "Failed to create device buffer (Error: %d)", res);
            crenvk_buffer_destroy(buffer, device);
            return NULL;
        }

        // push NULL to mappedData (will be filled later when mapped explicitly)
        void* nullPtr = NULL;
        if (darray_push_back(buffer->mappedData, &nullPtr) != CTOOLBOX_SUCCESS) {
            crenvk_buffer_destroy(buffer, device);
            return NULL;
        }
    }

    return buffer;
}

CREN_API void crenvk_buffer_destroy(vkBuffer* buffer, VkDevice device)
{
    if (!buffer || device == VK_NULL_HANDLE) return;

    for (unsigned int i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
        //void** dataPtr = (void**)crenarray_at(buffer->mappedData, i);
        void** dataPtr = NULL;
        darray_get(buffer->mappedData, i, &dataPtr);
        if (dataPtr && *dataPtr && buffer->mapped) {
            vkUnmapMemory(device, buffer->memories[i]);
            *dataPtr = NULL;
        }

        if (buffer->buffers && buffer->buffers[i] != VK_NULL_HANDLE) {
            vkDestroyBuffer(device, buffer->buffers[i], NULL);
            buffer->buffers[i] = VK_NULL_HANDLE;
        }

        if (buffer->memories && buffer->memories[i] != VK_NULL_HANDLE) {
            vkFreeMemory(device, buffer->memories[i], NULL);
            buffer->memories[i] = VK_NULL_HANDLE;
        }
    }

    if (buffer->buffers) {
        free(buffer->buffers);
        buffer->buffers = NULL;
    }

    if (buffer->memories) {
        free(buffer->memories);
        buffer->memories = NULL;
    }

    if (buffer->mappedData) {
        free(buffer->mappedData);
        buffer->mappedData = NULL;
    }

    free(buffer);
}

VkResult crenvk_buffer_map(vkBuffer* buffer, VkDevice device)
{
    if (!buffer || !device) return VK_ERROR_INITIALIZATION_FAILED;

    for (unsigned int i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
        //void** dataPtr = crenarray_at(buffer->mappedData, i);
        void** dataPtr = NULL;
        darray_get(buffer->mappedData, i, &dataPtr);

        if (!dataPtr) continue; // the mapped data is null, either the buffer was mapped only in the first iteration or an error occured
        if (*dataPtr) continue;  // skip if already mapped

        VkResult res = vkMapMemory(device, buffer->memories[i], 0, VK_WHOLE_SIZE, 0, dataPtr);
        if (res != VK_SUCCESS) {
            CREN_LOG(CRenLogSeverity_Error, "vkMapMemory failed: %d", res);
            return res;
        }
    }
    buffer->mapped = true;
    return VK_SUCCESS;
}

void crenvk_buffer_unmap(vkBuffer* buffer, VkDevice device)
{
    if (buffer == NULL || device == VK_NULL_HANDLE) return;

    for (unsigned int i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
        //void** dataPtr = (void**)crenarray_at(buffer->mappedData, i);
        void** dataPtr = NULL;
        darray_get(buffer->mappedData, i, &dataPtr);

        if (!dataPtr || !(*dataPtr)) continue;

        vkUnmapMemory(device, buffer->memories[i]);
        *dataPtr = NULL;
    }
    buffer->mapped = false;
}

void crenvk_buffer_flush(vkBuffer* buffer, VkDevice device, VkDeviceSize offset, VkDeviceSize size)
{
    VkMappedMemoryRange range = { 0 };
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.memory = buffer->memories[0];
    range.offset = offset;
    range.size = size;

    vkFlushMappedMemoryRanges(device, 1, &range);
}

VkResult crenvk_buffer_copy(vkBuffer* buffer, unsigned int index, const void* src, unsigned long long size)
{
    if (!buffer || !src || size == 0) {
        CREN_LOG(CRenLogSeverity_Error, "Invalid buffer, source, or size");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    // check array bounds
    if (index >= darray_size(buffer->mappedData)) {
        CREN_LOG(CRenLogSeverity_Error, "Index %u out of bounds (size=%llu)", index, darray_size(buffer->mappedData));
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // get the mapped pointer
    //void* dst = buffer->mappedData->data[index];
    void** dst = NULL;
    darray_get(buffer->mappedData, index, &dst);

    if (!dst) {
        CREN_LOG(CRenLogSeverity_Error, "Buffer not mapped at index %u", index);
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    // copy
    memcpy(dst, src, size);
    return VK_SUCCESS;
}

void crenvk_buffer_command_copy(VkCommandBuffer cmd, vkBuffer* srcBuffer, vkBuffer* dstBuffer, unsigned int srcIndex, unsigned int dstIndex, const VkBufferCopy* region)
{
    if (!cmd || !srcBuffer || !dstBuffer) {
        CREN_LOG(CRenLogSeverity_Error, "Invalid command buffer or buffers");
        return;
    }

    vkCmdCopyBuffer(cmd, srcBuffer->buffers[srcIndex], dstBuffer->buffers[dstIndex], 1, region);
}
