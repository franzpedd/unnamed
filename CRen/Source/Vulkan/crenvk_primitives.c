#include "crenvk_primitives.h"

#include "cren_context.h"
#include "cren_error.h"
#include "cren_platform.h"
#include "Vulkan/crenvk_core.h"
#include "Vulkan/crenvk_context.h"
#include <memm/memm.h>
#include <string.h>
#include <math.h>

CREN_API VkResult crenvk_create_texture2d_from_path(CRenContext* context, vkTexture2D* out_tex, const char* path, bool uiTexture)
{
    // asserts for compiling error time catch
    CREN_ASSERT(context != NULL, "Creating 2D Texture when Context is NULL");
    CREN_ASSERT(path != NULL, "Creating 2D Texture when disk path is NULL/Invalid (%s)", path);
    CREN_ASSERT(out_tex == NULL, "Creating 2D Texture when param out_tex is already initialzed");

    CRenVulkanBackend* backend = (CRenVulkanBackend*)cren_get_vulkan_backend(context);
    CREN_ASSERT(backend != NULL, "Vulkan backend is NULL when creating a Vulkan 2D Texture");

    // memory initialization and image data
    out_tex = (vkTexture2D*)malloc(sizeof(vkTexture2D));
    if (!out_tex) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate memory for Vulkan 2D Texture %s", path);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    strncpy(out_tex, path, sizeof(out_tex->path)- 1);
    
    int channels = 0;
    const int desiredChannels = 4;
    unsigned char* pixels = cren_stbimage_load_from_file(out_tex->path, desiredChannels, &out_tex->width, &out_tex->height, &channels);
    if (!pixels || out_tex->width == 0 || out_tex->height == 0) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to load image %s when creating Vulkan 2D Texture", path);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // staging buffer
    VkDeviceSize imageSize = (VkDeviceSize)(out_tex->width * out_tex->height * desiredChannels);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    VkResult res = crenvk_device_create_buffer(backend->device.device, backend->device.physicalDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, imageSize, &stagingBuffer, &stagingMemory, NULL);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Invalid create gpu Staging Buffer");
        return res;
    }

    // copy pixel data to staging buffer
    void* data;
    vkMapMemory(backend->device.device, stagingMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, imageSize);
    vkUnmapMemory(backend->device.device, stagingMemory);
    cren_stbimage_destroy(pixels);

    // determine renderpass, format and mip-level
    const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkRenderpass* renderpass = cren_using_custom_viewport(context) ? backend->viewportRenderphase->renderpass : backend->defaultRenderphase->renderpass;
    
    out_tex->mipLevels = uiTexture ? 1 : (uint32_t)floor(log2(f_max((float)out_tex->width, (float)out_tex->height))) + 1;

    res = crenvk_device_create_image(out_tex->width, out_tex->height, out_tex->mipLevels, 1, backend->device.device, backend->device.physicalDevice, &out_tex->image, &out_tex->memory, format, uiTexture ? VK_SAMPLE_COUNT_1_BIT : renderpass->msaa, VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Invalid create vulkan image for texture");
        return res;
    }

    // transition image layouts and copy data
    VkCommandBuffer cmd = crenvk_device_begin_commandbuffer_singletime(backend->device.device, renderpass->commandPool);

    // Transition to TRANSFER_DST
    VkImageMemoryBarrier barrier = { 0 };
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = out_tex->image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = out_tex->mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

    // Copy buffer to image
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
    region.imageExtent.width = out_tex->width;
    region.imageExtent.height = out_tex->height;
    region.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(cmd, stagingBuffer, out_tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // generate mipmaps if needed
    if (out_tex->mipLevels > 1) {
        crenvk_device_create_image_mipmaps(backend->device.device, backend->device.graphicsQueue, cmd, out_tex->width, out_tex->height, out_tex->mipLevels, out_tex->image);
    }

    else {
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
    }

    // end command buffer
    crenvk_device_end_commandbuffer_singletime(backend->device.device, renderpass->commandPool, cmd, backend->device.graphicsQueue);

    // cleanup staging resources
    vkDestroyBuffer(backend->device.device, stagingBuffer, NULL);
    vkFreeMemory(backend->device.device, stagingMemory, NULL);

    // create image view
    res = crenvk_device_create_image_view(backend->device.device, out_tex->image, format, VK_IMAGE_ASPECT_COLOR_BIT, out_tex->mipLevels, 1, VK_IMAGE_VIEW_TYPE_2D, NULL, &out_tex->view);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Invalid create vulkan image view for texture");
        return res;
    }

    // create sampler
    res = crenvk_device_create_image_sampler(backend->device.device, backend->device.physicalDevice, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, (float)out_tex->mipLevels, &out_tex->sampler);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Invalid create vulkan image sampler for texture");
        return res;
    }

    // create descriptor set
    res = crenvk_device_create_image_descriptor_set
    (
        backend->device.device,
        backend->uiRenderphase->desc,
        backend->uiRenderphase->descSetLayout,
        out_tex->sampler,
        out_tex->view,
        &out_tex->uiDescriptor
    );
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Invalid create vulkan image descriptor set for texture");
        return res;
    }

    return VK_SUCCESS;
}
