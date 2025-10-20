#include "crenvk_primitives.h"

#include "cren_context.h"
#include "cren_error.h"
#include "cren_platform.h"
#include "crenvk_buffer.h"

#include <memm/memm.h>
#include <string.h>

#if defined (CREN_BUILD_WITH_VULKAN)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CREN_API VkResult crenvk_texture2d_create_from_path(CRenVulkanBackend* backend, const char* path, bool uiTexture, bool usingCustomViewport, CRenVKTexture2D** out_texture, int32_t* out_width, int32_t* out_height, int32_t* out_mipLevels)
{
    // initial error checking
    CREN_ASSERT(backend != NULL, "Vulkan Backend is NULL");

    if (path == NULL) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Path is NULL");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (*out_texture != NULL) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Output Texture is already initialized");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // allocate and initialize texture structure
    CRenVKTexture2D* texture = (CRenVKTexture2D*)malloc(sizeof(CRenVKTexture2D));
    if (!texture) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate memory for texture: %s", path);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    memset(texture, 0, sizeof(CRenVKTexture2D));
    texture->image = VK_NULL_HANDLE;
    texture->memory = VK_NULL_HANDLE;
    texture->view = VK_NULL_HANDLE;
    texture->sampler = VK_NULL_HANDLE;
    texture->uiDescriptor = VK_NULL_HANDLE;
    //
    VkResult result = VK_SUCCESS;
    uint8_t* pixels = NULL;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    VkCommandBuffer cmd = VK_NULL_HANDLE;

    // structured error handling
    do {
        // load image data
        int channels = 0;
        const int desiredChannels = 4;
        pixels = cren_stbimage_load_from_file(path, desiredChannels, out_width, out_height, &channels);
        if (!pixels || *out_width == 0 || *out_height == 0) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to load image: %s", path);
            result = VK_ERROR_INITIALIZATION_FAILED;
            break;
        }

        // staging buffer
        VkDeviceSize imageSize = (VkDeviceSize)(*out_width * *out_height * desiredChannels);
        result = crenvk_device_create_buffer(backend->device.device, backend->device.physicalDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, imageSize, &stagingBuffer, &stagingMemory, NULL);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create staging buffer for: %s", path);
            break;
        }

        void* data;
        result = vkMapMemory(backend->device.device, stagingMemory, 0, imageSize, 0, &data);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to map staging memory for: %s", path);
            break;
        }
        memcpy(data, pixels, imageSize);
        vkUnmapMemory(backend->device.device, stagingMemory);

        *out_mipLevels = uiTexture ? 1 : i_floor(f_log2(f_max((float)*out_width, (float)*out_height))) + 1;
        const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
        const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        vkRenderpass* renderpass = usingCustomViewport ? backend->viewportRenderphase->renderpass : backend->defaultRenderphase->renderpass;

        // create Vulkan image
        result = crenvk_device_create_image(*out_width, *out_height, *out_mipLevels, 1, backend->device.device, backend->device.physicalDevice, &texture->image, &texture->memory, format, uiTexture ? VK_SAMPLE_COUNT_1_BIT : renderpass->msaa, VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create Vulkan image for: %s", path);
            break;
        }

        // transfer operations
        cmd = crenvk_device_begin_commandbuffer_singletime(backend->device.device, renderpass->commandPool);
        if (!cmd) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to begin command buffer for: %s", path);
            result = VK_ERROR_INITIALIZATION_FAILED;
            break;
        }

        // transition image to TRANSFER_DST_OPTIMAL
        VkImageMemoryBarrier barrier = { 0 };
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = texture->image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = *out_mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

        // copy buffer to image
        VkBufferImageCopy region = { 0 };
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset.x = 0;
        region.imageOffset.y = 0;
        region.imageOffset.z = 0;
        region.imageExtent.width = *out_width;
        region.imageExtent.height = *out_height;
        region.imageExtent.depth = 1;
        vkCmdCopyBufferToImage(cmd, stagingBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        // generate mipmaps or transition to final layout
        if (*out_mipLevels > 1) {
            crenvk_device_create_image_mipmaps(backend->device.device, backend->device.graphicsQueue, cmd, *out_width, *out_height, *out_mipLevels, texture->image);
        }
        else {
            // transition to SHADER_READ_ONLY_OPTIMAL for non-mipmapped textures
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
        }

        // end and submit command buffer
        result = crenvk_device_end_commandbuffer_singletime(backend->device.device, renderpass->commandPool, cmd, backend->device.graphicsQueue);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to submit command buffer for: %s", path);
            break;
        }

        // create image view
        result = crenvk_device_create_image_view(backend->device.device, texture->image, format, VK_IMAGE_ASPECT_COLOR_BIT, *out_mipLevels, 1, VK_IMAGE_VIEW_TYPE_2D, NULL, &texture->view);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create image view for: %s", path);
            break;
        }

        // create sampler
        result = crenvk_device_create_image_sampler(backend->device.device, backend->device.physicalDevice, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, (float)*out_mipLevels, &texture->sampler);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create sampler for: %s", path);
            break;
        }

        // create descriptor set
        result = crenvk_device_create_image_descriptor_set(backend->device.device, backend->uiRenderphase->descPool, backend->uiRenderphase->descSetLayout, texture->sampler, texture->view, &texture->uiDescriptor);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create descriptor set for: %s", path);
            break;
        }

        // success - assign output parameter
        *out_texture = texture;
        texture = NULL; // prevent cleanup of the texture resources

    } while (0);

    // cleanup
    if (pixels) cren_stbimage_destroy(pixels);
    if (stagingBuffer != VK_NULL_HANDLE) vkDestroyBuffer(backend->device.device, stagingBuffer, NULL);
    if (stagingMemory != VK_NULL_HANDLE) vkFreeMemory(backend->device.device, stagingMemory, NULL);
    if (texture) {
        crenvk_texture2d_destroy(backend, texture);
        free(texture);
    }

    return result;
}

CREN_API VkResult crenvk_texture2d_create_from_buffer(CRenVulkanBackend* backend, uint8_t* buffer, size_t bufferLen, int width, int height, bool uiTexture, bool usingCustomViewport, CRenVKTexture2D** out_texture, int32_t* out_mipLevels)
{
    // input validation
    // initial error checking
    CREN_ASSERT(backend != NULL, "Vulkan Backend is NULL");

    if (buffer == NULL || bufferLen <= 0 || width <= 0 || height <= 0) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Buffer data for texture is incorrect");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (*out_texture != NULL) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Output Texture is already initialized");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // allocate and initialize texture structure
    CRenVKTexture2D* texture = (CRenVKTexture2D*)malloc(sizeof(CRenVKTexture2D));
    if (!texture) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate memory for texture from buffer");
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    memset(texture, 0, sizeof(CRenVKTexture2D));

    // initialize with safe defaults
    texture->image = VK_NULL_HANDLE;
    texture->memory = VK_NULL_HANDLE;
    texture->view = VK_NULL_HANDLE;
    texture->sampler = VK_NULL_HANDLE;
    texture->uiDescriptor = VK_NULL_HANDLE;
    //
    VkResult result = VK_SUCCESS;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    VkCommandBuffer cmd = VK_NULL_HANDLE;

    // structured error handling
    do {
        // staging buffer
        VkDeviceSize imageSize = (VkDeviceSize)bufferLen;
        result = crenvk_device_create_buffer(backend->device.device, backend->device.physicalDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, imageSize, &stagingBuffer, &stagingMemory, NULL);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create staging buffer for texture from buffer");
            break;
        }

        void* data;
        result = vkMapMemory(backend->device.device, stagingMemory, 0, imageSize, 0, &data);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to map staging memory for texture from buffer");
            break;
        }
        memcpy(data, buffer, imageSize);
        vkUnmapMemory(backend->device.device, stagingMemory);

        // calculate mipLevels, determine renderpass and texture properties
        *out_mipLevels = uiTexture ? 1 : i_floor(f_log2(f_max((float)width, (float)height))) + 1;
        const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
        const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        vkRenderpass* renderpass = usingCustomViewport ? backend->viewportRenderphase->renderpass : backend->defaultRenderphase->renderpass;

        // create vulkan image
        result = crenvk_device_create_image(width, height, *out_mipLevels, 1, backend->device.device, backend->device.physicalDevice, &texture->image, &texture->memory, format, uiTexture ? VK_SAMPLE_COUNT_1_BIT : renderpass->msaa, VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create Vulkan image for texture from buffer");
            break;
        }

        // begin command buffer for transfer operations
        cmd = crenvk_device_begin_commandbuffer_singletime(backend->device.device, renderpass->commandPool);
        if (!cmd) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to begin command buffer for texture from buffer");
            result = VK_ERROR_INITIALIZATION_FAILED;
            break;
        }

        // transition image to TRANSFER_DST_OPTIMAL
        VkImageMemoryBarrier barrier = { 0 };
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = texture->image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = *out_mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

        // copy buffer to image
        VkBufferImageCopy region = { 0 };
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset.x = 0;
        region.imageOffset.y = 0;
        region.imageOffset.z = 0;
        region.imageExtent.width = width;
        region.imageExtent.height = height;
        region.imageExtent.depth = 1;
        vkCmdCopyBufferToImage(cmd, stagingBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        // generate mipmaps or transition to final layout
        if (*out_mipLevels > 1) {
            crenvk_device_create_image_mipmaps(backend->device.device, backend->device.graphicsQueue, cmd, width, height, *out_mipLevels, texture->image);
        }
        else {
            // transition to SHADER_READ_ONLY_OPTIMAL for non-mipmapped textures
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
        }

        // end and submit command buffer
        result = crenvk_device_end_commandbuffer_singletime(backend->device.device, renderpass->commandPool, cmd, backend->device.graphicsQueue);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to submit command buffer for texture from buffer");
            break;
        }

        // create image view
        result = crenvk_device_create_image_view(backend->device.device, texture->image, format, VK_IMAGE_ASPECT_COLOR_BIT, *out_mipLevels, 1, VK_IMAGE_VIEW_TYPE_2D, NULL, &texture->view);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create image view for texture from buffer");
            break;
        }

        // create sampler
        result = crenvk_device_create_image_sampler(backend->device.device, backend->device.physicalDevice, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, (float)*out_mipLevels, &texture->sampler);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create sampler for texture from buffer");
            break;
        }

        // create descriptor set
        result = crenvk_device_create_image_descriptor_set(backend->device.device, backend->uiRenderphase->descPool, backend->uiRenderphase->descSetLayout, texture->sampler, texture->view, &texture->uiDescriptor);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create descriptor set for texture from buffer");
            break;
        }

        // success - assign output parameter
        *out_texture = texture;
        texture = NULL; // prevent cleanup of the texture resources

    } while (0);

    // cleanup
    if (stagingBuffer != VK_NULL_HANDLE) vkDestroyBuffer(backend->device.device, stagingBuffer, NULL);
    if (stagingMemory != VK_NULL_HANDLE) vkFreeMemory(backend->device.device, stagingMemory, NULL);
    if (texture) {
        crenvk_texture2d_destroy(backend, texture);
        free(texture);
    }

    return result;
}


CREN_API void crenvk_texture2d_destroy(CRenVulkanBackend* backend, CRenVKTexture2D* texture)
{
    CREN_ASSERT(backend != NULL, "Vulkan Backend is NULL");
    CREN_ASSERT(texture != NULL, "Vulkan Texture is NULL");

    if (texture->sampler != VK_NULL_HANDLE) vkDestroySampler(backend->device.device, texture->sampler, NULL);
    if (texture->view != VK_NULL_HANDLE) vkDestroyImageView(backend->device.device, texture->view, NULL);
    if (texture->image != VK_NULL_HANDLE) vkDestroyImage(backend->device.device, texture->image, NULL);
    if (texture->memory != VK_NULL_HANDLE) vkFreeMemory(backend->device.device, texture->memory, NULL);

    free(texture);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quad
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CREN_API VkResult crenvk_quad_create_from_path(CRenVulkanBackend* backend, const char* albedoPath, bool usingCustomViewport, CRenVKQuad** out_quad)
{
    // input validation
    CREN_ASSERT(backend != NULL, "Vulkan Backend is NULL");

    if (albedoPath == NULL) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Albedo path for quad is NULL");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (*out_quad != NULL) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Output quad is already initialized");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // allocate and initialize quad structure
    CRenVKQuad* quad = (CRenVKQuad*)malloc(sizeof(CRenVKQuad));
    if (!quad) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate memory for quad: %s", albedoPath);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    memset(quad, 0, sizeof(CRenVKQuad));
    for (uint32_t i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
        quad->descriptorSets[i] = VK_NULL_HANDLE;
    }
    quad->buffer = NULL;
    quad->descriptorPool = VK_NULL_HANDLE;
    quad->params.billboard = 1;
    quad->params.uv_rotation = 0.0f;
    quad->params.lockAxis.xy.x = 0.0f;
    quad->params.lockAxis.xy.y = 0.0f;
    quad->params.uv_offset.xy.x = 0.0f;
    quad->params.uv_offset.xy.y = 0.0f;
    quad->params.uv_scale.xy.x = 1.0f;
    quad->params.uv_scale.xy.y = 1.0f;
    //
    VkResult result = VK_SUCCESS;
    vkBuffer* staging = NULL;
    VkCommandBuffer cmd = VK_NULL_HANDLE;

    // structured error handling
    do {
        // create GPU buffer for quad parameters
        quad->buffer = crenvk_buffer_create(backend->device.device, backend->device.physicalDevice, sizeof(vkQuadBufferParams), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, CREN_CONCURRENTLY_RENDERED_FRAMES);
        if (!quad->buffer) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create GPU buffer for quad: %s", albedoPath);
            result = VK_ERROR_INITIALIZATION_FAILED;
            break;
        }

        // create staging buffer
        staging = crenvk_buffer_create(backend->device.device, backend->device.physicalDevice, sizeof(vkQuadBufferParams), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, CREN_CONCURRENTLY_RENDERED_FRAMES);
        if (!staging) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create staging buffer for quad: %s", albedoPath);
            result = VK_ERROR_INITIALIZATION_FAILED;
            break;
        }

        // map, copy, and unmap staging buffer
        for (uint32_t i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
            result = crenvk_buffer_map(backend->device.device, staging, i);
            if (result != VK_SUCCESS) {
                CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to map staging buffer for quad: %s", albedoPath);
                break;
            }

            crenvk_buffer_copy(staging, i, &quad->params, sizeof(vkQuadBufferParams), 0);
            crenvk_buffer_unmap(backend->device.device, staging, i);
        }

        // copy staging to GPU buffer
        vkRenderpass* renderpass = usingCustomViewport ? backend->viewportRenderphase->renderpass : backend->defaultRenderphase->renderpass;
        cmd = crenvk_device_begin_commandbuffer_singletime(backend->device.device, renderpass->commandPool);
        if (!cmd) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to begin command buffer for quad: %s", albedoPath);
            result = VK_ERROR_INITIALIZATION_FAILED;
            break;
        }

        for (uint32_t i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
            crenvk_buffer_command_copy(cmd, staging, i, quad->buffer, i, sizeof(vkQuadBufferParams), 0, 0);
        }

        result = crenvk_device_end_commandbuffer_singletime(backend->device.device, renderpass->commandPool, cmd, backend->device.graphicsQueue);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to submit command buffer for quad: %s", albedoPath);
            break;
        }

        // create descriptor pool
        VkDescriptorPoolSize poolSizes[3] = { 0 };
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[1].descriptorCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
        poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[2].descriptorCount = CREN_CONCURRENTLY_RENDERED_FRAMES;

        VkDescriptorPoolCreateInfo descriptorPoolCI = { 0 };
        descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCI.poolSizeCount = (uint32_t)CREN_STATIC_ARRAY_SIZE(poolSizes);
        descriptorPoolCI.pPoolSizes = poolSizes;
        descriptorPoolCI.maxSets = CREN_CONCURRENTLY_RENDERED_FRAMES;

        result = vkCreateDescriptorPool(backend->device.device, &descriptorPoolCI, NULL, &quad->descriptorPool);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create descriptor pool for quad: %s", albedoPath);
            break;
        }

        // get pipeline and allocate descriptor sets
        vkPipeline* pipeline = (vkPipeline*)shashtable_lookup(backend->pipelinesLib, CREN_PIPELINE_QUAD_DEFAULT_NAME);
        if (!pipeline) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to find quad pipeline: %s", CREN_PIPELINE_QUAD_DEFAULT_NAME);
            result = VK_ERROR_INITIALIZATION_FAILED;
            break;
        }

        VkDescriptorSetLayout layouts[CREN_CONCURRENTLY_RENDERED_FRAMES];
        for (uint32_t i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
            layouts[i] = pipeline->descriptorSetLayout;
        }

        VkDescriptorSetAllocateInfo descSetAllocInfo = { 0 };
        descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descSetAllocInfo.descriptorPool = quad->descriptorPool;
        descSetAllocInfo.descriptorSetCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
        descSetAllocInfo.pSetLayouts = layouts;

        result = vkAllocateDescriptorSets(backend->device.device, &descSetAllocInfo, quad->descriptorSets);
        if (result != VK_SUCCESS) {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate descriptor sets for quad: %s", albedoPath);
            break;
        }

        // success - assign output parameter
        *out_quad = quad;
        quad = NULL; // prevent cleanup

    } while (0);

    // cleanup on failure
    if (staging) crenvk_buffer_destroy(backend->device.device, staging);
    if (quad) {
        crenvk_quad_destroy(backend, quad);
        free(quad);
    }

    return result;
}

CREN_API void crenvk_quad_destroy(CRenVulkanBackend* backend, CRenVKQuad* quad)
{
    if (!quad || !backend) return;
    if (quad->buffer) crenvk_buffer_destroy(backend->device.device, quad->buffer);
    if (quad->descriptorPool != VK_NULL_HANDLE) vkDestroyDescriptorPool(backend->device.device, quad->descriptorPool, NULL);

    free(quad);
}

CREN_API void crenvk_quad_update(CRenVulkanBackend* backend, CRenVKQuad* quad)
{
    if (!backend || !quad) return;

    if (quad->buffer) {
        for(uint32_t i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
            void* where = quad->buffer->mappedPointers[i];

            if (where) {
                memcpy(where, &quad->buffer, sizeof(vkQuadBufferParams));
            }
        }
    }
}

CREN_API void crenvk_quad_render(CRenVulkanBackend* backend, CRenVKQuad* quad, CRen_RenderStage stage, const fmat4 modelMatrix, uint32_t id, bool usingCustomViewport)
{
    const VkDeviceSize offsets[] = { 0 };
    vkPipeline* crenPipe = NULL;
    VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    uint32_t currentFrame = backend->swapchain.currentFrame;

    switch (stage)
    {
        case CREN_RENDER_STAGE_DEFAULT:
        {
            crenPipe = (vkPipeline*)shashtable_lookup(backend->pipelinesLib, CREN_PIPELINE_QUAD_DEFAULT_NAME);
            cmdBuffer = usingCustomViewport ? backend->viewportRenderphase->renderpass->commandBuffers[currentFrame] : backend->defaultRenderphase->renderpass->commandBuffers[currentFrame];
            break;
        }

        case CREN_RENDER_STAGE_PICKING:
        {
            crenPipe = (vkPipeline*)shashtable_lookup(backend->pipelinesLib, CREN_PIPELINE_QUAD_PICKING_NAME);
            cmdBuffer = backend->pickingRenderphase->renderpass->commandBuffers[currentFrame];
            break;
        }

        default:
        {
            CREN_LOG(CREN_LOG_SEVERITY_ERROR, "The render stage %d is invalid", stage);
            return;
        }
    }

    pipelineLayout = crenPipe->layout;

    vkBufferPushConstant constants = { 0 };
    constants.id = id;
    constants.model = modelMatrix;
    vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vkBufferPushConstant), &constants);

    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &quad->descriptorSets[currentFrame], 0, NULL);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, crenPipe->pipeline);
    vkCmdDraw(cmdBuffer, 6, 1, 0, 0);
}

CREN_API void crenvk_quad_update_descriptors(CRenVulkanBackend* backend, CRenVKQuad* quad, CRenVKTexture2D* albedoTexture)
{
    for (uint32_t i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++)
    {
        // 0: camera data
        vkBuffer* cameraBuffer = (vkBuffer*)shashtable_lookup(backend->buffersLib, "Camera");
        VkDescriptorBufferInfo camInfo = { 0 };
        camInfo.buffer = cameraBuffer->buffers[i];
        camInfo.offset = 0;
        camInfo.range = sizeof(vkBufferCamera);

        VkWriteDescriptorSet camDesc = { 0 };
        camDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        camDesc.dstSet = quad->descriptorSets[i];
        camDesc.dstBinding = 0;
        camDesc.dstArrayElement = 0;
        camDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        camDesc.descriptorCount = 1;
        camDesc.pBufferInfo = &camInfo;
        vkUpdateDescriptorSets(backend->device.device, 1, &camDesc, 0, NULL);

        // 1: quad data
        VkDescriptorBufferInfo quadInfo = { 0 };
        quadInfo.buffer = quad->buffer->buffers[i];
        quadInfo.offset = 0;
        quadInfo.range = sizeof(vkQuadBufferParams);

        VkWriteDescriptorSet quadDesc = { 0 };
        quadDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        quadDesc.dstSet = quad->descriptorSets[i];
        quadDesc.dstBinding = 1;
        quadDesc.dstArrayElement = 0;
        quadDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        quadDesc.descriptorCount = 1;
        quadDesc.pBufferInfo = &quadInfo;
        vkUpdateDescriptorSets(backend->device.device, 1, &quadDesc, 0, NULL);

        // 2: color map
        VkDescriptorImageInfo colorMapInfo = { 0 };
        colorMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        colorMapInfo.imageView = albedoTexture->view;
        colorMapInfo.sampler = albedoTexture->sampler;

        VkWriteDescriptorSet desc = { 0 };
        desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc.dstSet = quad->descriptorSets[i];
        desc.dstBinding = 2;
        desc.dstArrayElement = 0;
        desc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        desc.descriptorCount = 1;
        desc.pImageInfo = &colorMapInfo;
        vkUpdateDescriptorSets(backend->device.device, 1, &desc, 0, NULL);
    }

    // update the mapped data
    crenvk_quad_update(backend, quad);
}

#endif // CREN_BUILD_WITH_VULKAN