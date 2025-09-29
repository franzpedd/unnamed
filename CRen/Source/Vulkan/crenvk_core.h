#ifndef CRENVK_CORE_INCLUDED
#define CRENVK_CORE_INCLUDED
#ifdef CREN_BUILD_WITH_VULKAN

#include "cren_defines.h"
#include "cren_types.h"
#include <vulkan/vulkan.h>

#ifdef __cplusplus 
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instance
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct vkInstance
{
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugger;
	bool validationsEnabled;
} vkInstance;

/// @brief creates and populates a vulkan instance
CREN_API void crenvk_instance_create(vkInstance* instance, const char* appName, unsigned int appVersion, CRen_Renderer api, bool validations);

/// @brief free used resources by the vulkan instance
CREN_API void crenvk_instance_destroy(vkInstance* instance);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Device
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief cren vulkan queues info
typedef struct vkQueueFamilyIndices {
    int graphicFamily;
    int presentFamily;
    int computeFamily;
    int graphicFound;
    int presentFound;
    int computeFound;
} vkQueueFamilyIndices;

/// @brief cren vulkan device
typedef struct vkDevice {
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;
} vkDevice;

/// @brief Creates and populates the cren vulkan device
CREN_API void crenvk_device_create(vkDevice* device, VkInstance instance, void* nativeWindow, void* optionalHandle, int validations);

/// @brief Releases all resources used by the cren vulkan device
CREN_API void crenvk_device_destroy(vkDevice* device, VkInstance instance);

/// @brief queries the queue indices for graphics, presentation and compute based on physical device and surface
CREN_API vkQueueFamilyIndices crenvk_device_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);

/// @brief creates a buffer on the gpu
CREN_API VkResult crenvk_device_create_buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data);

/// @brief creates and begins a single-use command buffer
CREN_API VkCommandBuffer crenvk_device_begin_commandbuffer_singletime(VkDevice device, VkCommandPool cmdPool);

/// @brief ends the single-used command buffer
CREN_API void crenvk_device_end_commandbuffer_singletime(VkDevice device, VkCommandPool cmdPool, VkCommandBuffer cmdBuffer, VkQueue queue);

/// @brief returns what type of memory is the correct one, given filters an properties
CREN_API unsigned int crenvk_device_find_memory_type(VkPhysicalDevice physicalDevice, unsigned int typeFilter, VkMemoryPropertyFlags properties);

/// @brief returns the most suitable format given the candidates and physical device properties
CREN_API VkFormat crenvk_device_find_suitable_format(VkPhysicalDevice physicalDevice, const VkFormat* candidates, unsigned int candidatesCount, VkImageTiling tiling, VkFormatFeatureFlags features);

/// @brief returns the most suitable depth format given the physical device properties
CREN_API VkFormat crenvk_device_find_depth_format(VkPhysicalDevice physicalDevice);

/// @brief creates an image on device and binds it with memory
CREN_API VkResult crenvk_device_create_image(unsigned int width, unsigned int height, unsigned int mipLevels, unsigned int arrayLayers, VkDevice device, VkPhysicalDevice physicalDevice, VkImage* image, VkDeviceMemory* memory, VkFormat format, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkImageCreateFlags flags);

/// @brief creates an image view for an image TODO: move this somewhere else?
CREN_API VkResult crenvk_device_create_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect, unsigned int mipLevels, unsigned int layerCount, VkImageViewType viewType, const VkComponentMapping* swizzle, VkImageView* outView);

/// @brief creates an image sampler
CREN_API VkResult crenvk_device_create_image_sampler(VkDevice device, VkPhysicalDevice physicalDevice, VkFilter min, VkFilter mag, VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w, float mipLevels, VkSampler* outSampler);

/// @brief creates an image descriptor set
CREN_API VkResult crenvk_device_create_image_descriptor_set(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkSampler sampler, VkImageView view, VkDescriptorSet* outDescriptor);

/// @brief creates the mipmaps for an image
CREN_API void crenvk_device_create_image_mipmaps(VkDevice device, VkQueue queue, VkCommandBuffer cmdBuffer, int width, int height, int mipLevels, VkImage image);

/// @brief modifies an image transition layout
CREN_API VkResult crenvk_device_image_transition_layout(VkDevice device, VkQueue queue, VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, unsigned int mipLevels, unsigned int layerCount);

/// @brief inserts a memory dependency on an image
CREN_API void crenvk_device_insert_image_memory_barrier(VkCommandBuffer cmdBuffer, VkImage image, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Swapchain
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief cren vulkan swapchain details
typedef struct {
    VkExtent2D extent;
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* surfaceFormats;
    VkPresentModeKHR* presentModes;
    uint32_t surfaceFormatCount;
    uint32_t presentModeCount;
} vkSwapchainDetails;

/// @brief cren vulkan swapchain
typedef struct {
    VkSurfaceFormatKHR swapchainFormat;
    VkPresentModeKHR swapchainPresentMode;
    VkExtent2D swapchainExtent;
    unsigned int swapchainImageCount;
    VkSwapchainKHR swapchain;
    VkImage* swapchainImages;
    VkImageView* swapchainImageViews;

    // used on sync
    unsigned int imageIndex;
    unsigned int currentFrame;
    unsigned int swapchainSyncCount;
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* finishedRenderingSemaphores;
    VkFence* framesInFlightFences;
} vkSwapchain;

/// @brief Creates the cren vulkan swapchain
CREN_API void crenvk_swapchain_create(vkSwapchain* swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t width, uint32_t height, bool vsync);

/// @brief Releases all used resources by the swapchain
CREN_API void crenvk_swapchain_destroy(vkSwapchain* swapchain, VkDevice device);


#ifdef __cplusplus 
}
#endif

#endif // CREN_BUILD_WITH_VULKAN
#endif // CRENVK_CORE_INCLUDED