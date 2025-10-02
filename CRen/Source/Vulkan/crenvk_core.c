#include "crenvk_core.h"
#include "cren_error.h"
#include "cren_platform.h"

#include <memm/memm.h>
#include <ctoolbox/darray.h>
#include <vecmath/vecmath.h>
#include <string.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instance
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief vulkan validation messages returns back to this function
static VKAPI_ATTR VkBool32 VKAPI_CALL internal_crenvk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback, void* userData)
{
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "%s\n", callback->pMessage);
        return VK_FALSE;
    }

    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        CREN_LOG(CREN_LOG_SEVERITY_WARN, "%s\n", callback->pMessage);
        return VK_FALSE;
    }

    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        CREN_LOG(CREN_LOG_SEVERITY_INFO, "%s\n", callback->pMessage);
        return VK_FALSE;
    }

    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        CREN_LOG(CREN_LOG_SEVERITY_TRACE, "%s\n", callback->pMessage);
        return VK_FALSE;
    }
    return VK_TRUE;
}

void crenvk_instance_create(CRenContext* context, vkInstance* instance, const char* appName, unsigned int appVersion, CRen_Renderer api, bool validations, CRenCallback_GetVulkanRequiredInstanceExtensions callback)
{
    if (!instance) return;
    
    uint32_t count = 0;
    const char* const* extensions = callback(context, &count);
    instance->instance = VK_NULL_HANDLE;
    instance->debugger = VK_NULL_HANDLE;
    instance->validationsEnabled = validations;

    CREN_LOG(CREN_LOG_SEVERITY_TRACE, "Requesting %u instance extensions:", count);
    for (uint32_t i = 0; i < count; i++) {
        CREN_LOG(CREN_LOG_SEVERITY_TRACE, "  [%u] %s", i, extensions[i]);
    }

    // application info - fully initialized
    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = appVersion;
    appInfo.pEngineName = "CRen";
    appInfo.engineVersion = appVersion;
    appInfo.apiVersion = crenvk_encodeversion(api);
    appInfo.pNext = NULL;

    // instance create info - fully initialized
    VkInstanceCreateInfo instanceCI = {0};
    instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCI.pApplicationInfo = &appInfo;
    instanceCI.enabledExtensionCount = count;
    instanceCI.ppEnabledExtensionNames = extensions;
    instanceCI.flags = 0;
    instanceCI.pNext = NULL;
    instanceCI.enabledLayerCount = 0;
    instanceCI.ppEnabledLayerNames = NULL;
    
    CRen_Platform platform = cren_detect_platform();
    if (platform == CREN_PLATFORM_IOS || platform == CREN_PLATFORM_MACOS) instanceCI.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    // debug utils - persistent through function scope
    VkDebugUtilsMessengerCreateInfoEXT debugUtilsCI = {0};
    
    if (instance->validationsEnabled) {
        const char* validationList = "VK_LAYER_KHRONOS_validation";
        instanceCI.enabledLayerCount = 1U;
        instanceCI.ppEnabledLayerNames = &validationList;

        debugUtilsCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilsCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugUtilsCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugUtilsCI.pfnUserCallback = internal_crenvk_debug_callback;
        debugUtilsCI.pUserData = NULL;

        instanceCI.pNext = &debugUtilsCI;
    }

    // create instance
    VkResult result = vkCreateInstance(&instanceCI, NULL, &instance->instance);
    if (result != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create Vulkan Instance: %d", result);
        return;
    }

    //create debug messenger
    if (instance->validationsEnabled) {
        PFN_vkCreateDebugUtilsMessengerEXT createDebugFunc = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance->instance, "vkCreateDebugUtilsMessengerEXT");
        
        if (createDebugFunc) {
            result = createDebugFunc(instance->instance, &debugUtilsCI, NULL, &instance->debugger);
            if (result != VK_SUCCESS) {
                CREN_LOG(CREN_LOG_SEVERITY_WARN, "Failed to create Vulkan Debug Messenger: %d", result);
                instance->debugger = VK_NULL_HANDLE;
            }
        } else {
            CREN_LOG(CREN_LOG_SEVERITY_WARN, "vkCreateDebugUtilsMessengerEXT not available");
        }
    }
}

void crenvk_instance_destroy(vkInstance* instance)
{
    if (!instance) return;

    if (instance->validationsEnabled) {
        PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance->instance, "vkDestroyDebugUtilsMessengerEXT");
        if (instance->instance != VK_NULL_HANDLE || instance->debugger != VK_NULL_HANDLE) {
            func(instance->instance, instance->debugger, NULL);
        }
    }

    if (instance->instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance->instance, NULL);
    }
}

CREN_API unsigned int crenvk_encodeversion(CRen_Renderer api)
{
    unsigned int variant, major, minor, patch;
    switch (api)
    {
        case CREN_RENDERER_API_VULKAN_1_0: return VK_MAKE_API_VERSION(0, 1, 0, 0);
        case CREN_RENDERER_API_VULKAN_1_1: return VK_MAKE_API_VERSION(0, 1, 1, 0);
        case CREN_RENDERER_API_VULKAN_1_2: return VK_MAKE_API_VERSION(0, 1, 2, 0);
        case CREN_RENDERER_API_VULKAN_1_3: return VK_MAKE_API_VERSION(0, 1, 3, 0);
    }
    return VK_MAKE_API_VERSION(0, 1, 0, 0); // default 1.0.0
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Device
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief checks if a given physical device supports the required extensions
static int internal_crenvk_check_device_extension_support(VkPhysicalDevice device, const char** required_extensions, unsigned int extension_count)
{
    unsigned int available_extension_count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &available_extension_count, NULL);

    VkExtensionProperties* available_extensions = (VkExtensionProperties*)malloc(available_extension_count * sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(device, NULL, &available_extension_count, available_extensions);

    for (unsigned int i = 0; i < extension_count; i++) {
        int extension_found = 0;
        for (unsigned int j = 0; j < available_extension_count; j++) {
            if (strcmp(required_extensions[i], available_extensions[j].extensionName) == 0) {
                extension_found = 1;
                break;
            }
        }
        if (!extension_found) {
            free(available_extensions);
            return 0;
        }
    }

    free(available_extensions);
    return 1;
}

/// @brief chooses the most suitable GPU available, not supporting multiple gpus
static VkPhysicalDevice internal_crenvk_choose_physical_device(VkInstance instance, VkSurfaceKHR surface)
{
    // selecting most suitable physical device
    unsigned int gpus = 0;
    vkEnumeratePhysicalDevices(instance, &gpus, NULL);

    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(gpus * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &gpus, devices);

    VkPhysicalDevice choosenOne = VK_NULL_HANDLE;
    const char* requiredExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    const unsigned int requiredExtensionsCount = 1;
    VkDeviceSize bestScore = 0;

    for (unsigned int i = 0; i < gpus; i++) {

        // checking support
        VkPhysicalDeviceProperties device_props;
        VkPhysicalDeviceFeatures device_features;
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceProperties(devices[i], &device_props);
        vkGetPhysicalDeviceFeatures(devices[i], &device_features);
        vkGetPhysicalDeviceMemoryProperties(devices[i], &mem_props);
        vkQueueFamilyIndices indices = crenvk_device_find_queue_families(devices[i], surface);
        if (!indices.graphicFound || !indices.presentFound || !indices.computeFound) continue;
        if (!internal_crenvk_check_device_extension_support(devices[i], requiredExtensions, requiredExtensionsCount)) continue;

        // scoring
        VkDeviceSize currentScore = 0;
        if (device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) currentScore += 1000;  // discrete gpu
        currentScore += device_props.limits.maxImageDimension2D;                                    // max texture size
        for (unsigned int j = 0; j < mem_props.memoryHeapCount; j++) {                              // prefer devices with dedicated VRAM
            if (mem_props.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                currentScore += mem_props.memoryHeaps[j].size / (VkDeviceSize)(1024 * 1024);        // mb
            }
        }

        if (currentScore > bestScore) {
            bestScore = currentScore;
            choosenOne = devices[i];
        }
    }

    free(devices);
    return choosenOne;
}

/// @brief creates the logical device
static void internal_crenvk_create_logical_device(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice* device, VkQueue* graphicsQueue, VkQueue* presentQueue, VkQueue* computeQueue, int* graphicsIndex, int* presentIndex, int* computeIndex, bool validations)
{
    const char* validationLayers[] = { "VK_LAYER_KHRONOS_validation" }; // must be the same as instance, wich it is
    unsigned int validationLayerCount = 1;

    // find unique queue families
    vkQueueFamilyIndices indices = crenvk_device_find_queue_families(physicalDevice, surface);
    unsigned int queueFamilyIndices[3]; // store unique queue family indices
    unsigned int queueCount = 0;
    float queuePriority = 1.0f;

    if (indices.graphicFamily != -1) queueFamilyIndices[queueCount++] = indices.graphicFamily;
    if (indices.presentFamily != -1 && indices.presentFamily != indices.graphicFamily)  queueFamilyIndices[queueCount++] = indices.presentFamily;
    if (indices.computeFamily != -1 && indices.computeFamily != indices.graphicFamily && indices.computeFamily != indices.presentFamily) queueFamilyIndices[queueCount++] = indices.computeFamily;
    
    // create queue create info for each unique queue family
    VkDeviceQueueCreateInfo* queueCreateInfos = (VkDeviceQueueCreateInfo*)malloc(sizeof(VkDeviceQueueCreateInfo) * queueCount);
    for (unsigned int i = 0; i < queueCount; i++) {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].pNext = NULL;
        queueCreateInfos[i].queueFamilyIndex = queueFamilyIndices[i];
        queueCreateInfos[i].queueCount = 1;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
        queueCreateInfos[i].flags = 0;
    }

    // handle apple extension
    #if defined(__APPLE__) && defined(__MACH__) && (VK_HEADER_VERSION >= 216)
    const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME };
    unsigned int extensionCount = 2;
    #else
    const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    unsigned int extensionCount = 1;
    #endif

    // required features
    VkPhysicalDeviceFeatures deviceFeatures = { 0 };
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    // device create info
    VkDeviceCreateInfo deviceCI = { 0 };
    deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCI.pNext = NULL;
    deviceCI.flags = 0;
    deviceCI.queueCreateInfoCount = queueCount;
    deviceCI.pQueueCreateInfos = queueCreateInfos;
    deviceCI.enabledExtensionCount = extensionCount;
    deviceCI.ppEnabledExtensionNames = extensions;
    deviceCI.pEnabledFeatures = &deviceFeatures;

    // validation layers
    if (validations && validationLayerCount > 0) {
        deviceCI.enabledLayerCount = validationLayerCount;
        deviceCI.ppEnabledLayerNames = validationLayers;
    }
    else {
        deviceCI.enabledLayerCount = 0;
        deviceCI.ppEnabledLayerNames = NULL;
    }
    
    // create the device
    CREN_ASSERT(vkCreateDevice(physicalDevice, &deviceCI, NULL, device) == VK_SUCCESS, "Failed to create vulkan logical device");

    // retrieve queues
    vkGetDeviceQueue(*device, indices.graphicFamily, 0, graphicsQueue);
    vkGetDeviceQueue(*device, indices.presentFamily, 0, presentQueue);
    vkGetDeviceQueue(*device, indices.computeFamily, 0, computeQueue);
    *graphicsIndex = indices.graphicFamily;
    *presentIndex = indices.presentFamily;
    *computeIndex = indices.computeFamily;

    free(queueCreateInfos);
}

void crenvk_device_create(vkDevice* device, VkInstance instance, bool validations)
{
    device->physicalDevice = internal_crenvk_choose_physical_device(instance, device->surface);
    CREN_ASSERT(device->physicalDevice != NULL, "An unfit physical device was choosen, please report about it");

    vkGetPhysicalDeviceMemoryProperties(device->physicalDevice, &device->physicalDeviceMemoryProperties);
    vkGetPhysicalDeviceProperties(device->physicalDevice, &device->physicalDeviceProperties);
    vkGetPhysicalDeviceFeatures(device->physicalDevice, &device->physicalDeviceFeatures);
    
    // create logical device
    internal_crenvk_create_logical_device(device->physicalDevice, device->surface, &device->device, &device->graphicsQueue, &device->presentQueue, &device->computeQueue, &device->graphicsQueueIndex, &device->presentQueueIndex, &device->computeQueueIndex, validations);
}

void crenvk_device_destroy(vkDevice* device, VkInstance instance)
{
    if (instance == VK_NULL_HANDLE || !device) return;

    if (device->device) vkDestroyDevice(device->device, NULL);
    if (device->surface) vkDestroySurfaceKHR(instance, device->surface, NULL);
}

vkQueueFamilyIndices crenvk_device_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    vkQueueFamilyIndices indices = {0};
    indices.graphicFamily = UINT32_MAX;
    indices.presentFamily = UINT32_MAX;
    indices.computeFamily = UINT32_MAX;

    unsigned int queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*)malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    for (unsigned int i = 0; i < queue_family_count; i++) {
        // check for graphics support
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicFamily = i;
            indices.graphicFound = 1;
        }

        // check for compute support
        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.computeFamily = i;
            indices.computeFound = 1;
        }

        // check for presentation support
        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (present_support) {
            indices.presentFamily = i;
            indices.presentFound = 1;
        }

        if (indices.graphicFamily && indices.presentFound && indices.computeFound) break;
    }

    free(queue_families);
    return indices;
}

VkResult crenvk_device_create_buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data)
{
    VkResult res = VK_SUCCESS;

    VkBufferCreateInfo bufferCI = { 0 };
    bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCI.size = size;
    bufferCI.usage = usage;
    bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    res = vkCreateBuffer(device, &bufferCI, NULL, buffer);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create buffer on GPU");
        return res;
    }

    VkMemoryRequirements memRequirements = { 0 };
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    // allocate memory for the buffer and bind it
    VkMemoryAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = crenvk_device_find_memory_type(physicalDevice, memRequirements.memoryTypeBits, properties);

    res = vkAllocateMemory(device, &allocInfo, NULL, memory);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate memory for GPU buffer");
        vkDestroyBuffer(device, *buffer, NULL);
        return res;
    }

    res = vkBindBufferMemory(device, *buffer, *memory, 0);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to bind GPU memory with buffer");
        vkDestroyBuffer(device, *buffer, NULL);
        vkFreeMemory(device, *memory, NULL);
        return res;
    }

    // map data if passing it
    if (data != NULL) {
        void* mapped;
        res = vkMapMemory(device, *memory, 0, size, 0, &mapped);
        if (res == VK_SUCCESS) {
            memcpy(mapped, data, size);
            vkUnmapMemory(device, *memory);
        }

        else {
            CREN_LOG(CREN_LOG_SEVERITY_WARN, "Failed to map memory for data upload (VkResult: %d)", res);
        }
    }
    return VK_SUCCESS;
}

VkCommandBuffer crenvk_device_begin_commandbuffer_singletime(VkDevice device, VkCommandPool cmdPool)
{
    VkCommandBufferAllocateInfo cmdBufferAllocInfo = { 0 };
    cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferAllocInfo.commandPool = cmdPool;
    cmdBufferAllocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &cmdBufferAllocInfo, &commandBuffer);

    VkCommandBufferBeginInfo cmdBufferBeginInfo = { 0 };
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkResult res = vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to begin command buffer");
    }

    return commandBuffer;
}

void crenvk_device_end_commandbuffer_singletime(VkDevice device, VkCommandPool cmdPool, VkCommandBuffer cmdBuffer, VkQueue queue)
{
    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    VkResult res = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to submit command buffer to queue");
    }

    res = vkQueueWaitIdle(queue);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to await queue response from sent command buffer");
    }

    vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuffer);
}

unsigned int crenvk_device_find_memory_type(VkPhysicalDevice physicalDevice, unsigned int typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to find suitable memory type");
    return UINT32_MAX;
}

VkFormat crenvk_device_find_suitable_format(VkPhysicalDevice physicalDevice, const VkFormat* candidates, unsigned int candidatesCount, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    VkFormat resFormat = VK_FORMAT_UNDEFINED;
    for (unsigned int i = 0; i < candidatesCount; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i], &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) resFormat = candidates[i];
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) resFormat = candidates[i];
    }

    CREN_ASSERT(resFormat != VK_FORMAT_UNDEFINED, "Failed to find suitable VkFormat");
    return resFormat;
}

VkFormat crenvk_device_find_depth_format(VkPhysicalDevice physicalDevice)
{
    const VkFormat candidates[] = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT };
    VkFormat format = crenvk_device_find_suitable_format(physicalDevice, candidates, 3, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    return format;
}

VkResult crenvk_device_create_image(unsigned int width, unsigned int height, unsigned int mipLevels, unsigned int arrayLayers, VkDevice device, VkPhysicalDevice physicalDevice, VkImage* image, VkDeviceMemory* memory, VkFormat format, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkImageCreateFlags flags)
{
    VkResult res = VK_SUCCESS;

    // specify and create image
    VkImageCreateInfo imageCI = { 0 };
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.pNext = NULL;
    imageCI.flags = flags;
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.extent.width = width;
    imageCI.extent.height = height;
    imageCI.extent.depth = 1;
    imageCI.mipLevels = mipLevels;
    imageCI.arrayLayers = arrayLayers;
    imageCI.format = format;
    imageCI.tiling = tiling;
    imageCI.usage = usage;
    imageCI.samples = samples;
    imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    res = vkCreateImage(device, &imageCI, NULL, image);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create device image (%d)", res);
        return res;
    }

    // query memory requirements and allocate it
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = crenvk_device_find_memory_type(physicalDevice, memRequirements.memoryTypeBits, memoryProperties);

    res = vkAllocateMemory(device, &allocInfo, NULL, memory);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate memory for the device image (%d)", res);
        return res;
    }

    res = vkBindImageMemory(device, *image, *memory, 0);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to bind memory with device image (%d)", res);
        return res;
    }

    return VK_SUCCESS;
}

VkResult crenvk_device_create_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect, unsigned int mipLevels, unsigned int layerCount, VkImageViewType viewType, const VkComponentMapping* swizzle, VkImageView* outView)
{
    if (mipLevels == 0 || layerCount == 0) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Invalid mipLevels or layerCount (must be >= 1)");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkImageViewCreateInfo imageViewCI = { 0 };
    imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCI.image = image;
    imageViewCI.viewType = viewType;
    imageViewCI.format = format;
    imageViewCI.subresourceRange.aspectMask = aspect;
    imageViewCI.subresourceRange.baseMipLevel = 0;
    imageViewCI.subresourceRange.levelCount = mipLevels;
    imageViewCI.subresourceRange.baseArrayLayer = 0;
    imageViewCI.subresourceRange.layerCount = layerCount;
    imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // using a different swizzling
    if (swizzle) {
        imageViewCI.components = *swizzle;
    }

    VkResult res = vkCreateImageView(device, &imageViewCI, NULL, outView);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create image view (VkResult: %d)", res);
        return res;
    }

    return VK_SUCCESS;
}

VkResult crenvk_device_create_image_sampler(VkDevice device, VkPhysicalDevice physicalDevice, VkFilter min, VkFilter mag, VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w, float mipLevels, VkSampler* outSampler)
{
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice, &props);

    VkSamplerCreateInfo samplerCI = { 0 };
    samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCI.magFilter = mag;
    samplerCI.minFilter = min;
    samplerCI.addressModeU = u;
    samplerCI.addressModeV = v;
    samplerCI.addressModeW = w;
    samplerCI.anisotropyEnable = VK_TRUE;
    samplerCI.maxAnisotropy = props.limits.maxSamplerAnisotropy;
    samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCI.unnormalizedCoordinates = VK_FALSE;
    samplerCI.compareEnable = VK_FALSE;
    samplerCI.maxLod = mipLevels;
    samplerCI.minLod = 0.0f;
    samplerCI.mipLodBias = 0.0f;
    samplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkResult res = vkCreateSampler(device, &samplerCI, NULL, outSampler);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create image sampler");
        return res;
    }

    return VK_SUCCESS;
}

VkResult crenvk_device_create_image_descriptor_set(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkSampler sampler, VkImageView view, VkDescriptorSet* outDescriptor)
{
    // create descriptor set
    VkDescriptorSetAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    VkResult res = vkAllocateDescriptorSets(device, &allocInfo, outDescriptor);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate descriptor set");
        return res;
    }

    // update descriptor set
    VkDescriptorImageInfo descImage[1] = { 0 };
    descImage[0].sampler = sampler;
    descImage[0].imageView = view;
    descImage[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writeDesc[1] = { 0 };
    writeDesc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDesc[0].dstSet = *outDescriptor;
    writeDesc[0].descriptorCount = 1;
    writeDesc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDesc[0].pImageInfo = descImage;
    vkUpdateDescriptorSets(device, 1, writeDesc, 0, NULL);

    return VK_SUCCESS;
}

void crenvk_device_create_image_mipmaps(VkDevice device, VkQueue queue, VkCommandBuffer cmdBuffer, int width, int height, int mipLevels, VkImage image)
{
    if (mipLevels <= 1) return;

    VkImageMemoryBarrier barrier = { 0 };
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = width;
    int32_t mipHeight = height;

    for (int32_t i = 1; i < mipLevels; i++) {

        // transition previous mip level to transfer source
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

        // blit from previous level to current level
        VkImageBlit blit = { 0 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.srcOffsets[1].x = mipWidth;
        blit.srcOffsets[1].y = mipHeight;
        blit.srcOffsets[1].z = 1;
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        blit.dstOffsets[1].x = mipWidth > 1 ? mipWidth / 2 : 1;
        blit.dstOffsets[1].y = mipHeight > 1 ? mipHeight / 2 : 1;
        blit.dstOffsets[1].z = 1;
        vkCmdBlitImage(cmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        // transition previous level to shader read
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    // transition last mip level
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
}

VkResult crenvk_device_image_transition_layout(VkDevice device, VkQueue queue, VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, unsigned int mipLevels, unsigned int layerCount)
{
    VkImageMemoryBarrier barrier = { 0 };
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layerCount;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0; // no synchronization is needed for VK_IMAGE_LAYOUT_UNDEFINED
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // set destination access mask for writing to a color attachment

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // writes to the color attachment
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;          // reads for transfer operations

        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // writing to color attachment
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;           // ready for transfer
    }

    else
    {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "The transition of layout OLD:%d NEW:%d is not supported (See VkImageLayout)", oldLayout, newLayout);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    vkCmdPipelineBarrier(cmdBuffer, sourceStage, destinationStage, 0, 0, NULL, 0, NULL, 1, &barrier);

    return VK_SUCCESS;
}

void crenvk_device_insert_image_memory_barrier(VkCommandBuffer cmdBuffer, VkImage image, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange)
{
    VkImageMemoryBarrier imageMemoryBarrier = { 0 };
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.pNext = 0;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.srcAccessMask = srcAccessFlags;
    imageMemoryBarrier.dstAccessMask = dstAccessFlags;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    vkCmdPipelineBarrier(cmdBuffer, srcStageMask, dstStageMask, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Swapchain
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief queries information for the swapchain, like available surface formats and etc.
static vkSwapchainDetails internal_crenvk_query_swapchain_details(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    vkSwapchainDetails details = { 0 };   
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &details.surfaceFormatCount, NULL);
    
    if (details.surfaceFormatCount != 0) {
        details.surfaceFormats = (VkSurfaceFormatKHR*)malloc(details.surfaceFormatCount * sizeof(VkSurfaceFormatKHR));

        if (details.surfaceFormats) {
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &details.surfaceFormatCount, details.surfaceFormats);
        }
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &details.presentModeCount, NULL);

    if (details.presentModeCount != 0) {
        details.presentModes = (VkPresentModeKHR*)malloc(details.presentModeCount * sizeof(VkPresentModeKHR));
        if (details.presentModes) {
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &details.presentModeCount, details.presentModes);
        }
    }

    return details;
}

/// @brief chooses the swapchain surface format, opting with the most widely supported (VK_FORMAT_B8G8R8A8_UNORM & VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
static VkSurfaceFormatKHR internal_crenvk_choose_swapchain_surface_format(VkSurfaceFormatKHR* formats, unsigned int quantity)
{
    for (unsigned int i = 0; i < quantity; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return formats[i];
    }

    return formats[0];
}

/// @brief chooses the most suitable presentation mode for the swapchain
static VkPresentModeKHR internal_crenvk_choose_swapchain_present_mode(VkPresentModeKHR* modes, uint32_t quantity, bool vsync)
{
    // handle edge cases and vsync request
    if (modes == NULL || quantity == 0 || vsync)  return VK_PRESENT_MODE_FIFO_KHR; // fallback to FIFO

    // search for the best non-VSync mode
    int immediateModeAvailable = 0;
    for (unsigned int i = 0; i < quantity; i++) {
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)  return VK_PRESENT_MODE_MAILBOX_KHR; // prefer MAILBOX (multiple buffering) if available
        if (modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) immediateModeAvailable = 1; // mark IMMEDIATE mode as available for fallback
    }

    // fallback to IMMEDIATE if available
    if (immediateModeAvailable) return VK_PRESENT_MODE_IMMEDIATE_KHR;

    // fallback to FIFO (always supported)
    return VK_PRESENT_MODE_FIFO_KHR;
}

/// @brief clamps the swapchain extent between the surface capabilities and returns it as the swapchain extent
static VkExtent2D internal_crenvk_choose_swapchain_extent(const VkSurfaceCapabilitiesKHR* capabilities, unsigned int width, unsigned int height)
{
    if (capabilities->currentExtent.width != UINT32_MAX) return capabilities->currentExtent;

    VkExtent2D actualExtent = { width, height };
    actualExtent.width = (uint32_t)f_clamp((const uint32_t)actualExtent.width, (const uint32_t)capabilities->minImageExtent.width, (const uint32_t)capabilities->maxImageExtent.width);
    actualExtent.height = (uint32_t)f_clamp((const uint32_t)actualExtent.height, (const uint32_t)capabilities->minImageExtent.height, (const uint32_t)capabilities->maxImageExtent.height);

    return actualExtent;
}

void crenvk_swapchain_create(vkSwapchain* swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t width, uint32_t height, bool vsync)
{
    vkSwapchainDetails details = internal_crenvk_query_swapchain_details(physicalDevice, surface);
    swapchain->swapchainFormat = internal_crenvk_choose_swapchain_surface_format(details.surfaceFormats, details.surfaceFormatCount);
    swapchain->swapchainPresentMode = internal_crenvk_choose_swapchain_present_mode(details.presentModes, details.presentModeCount, vsync);
    swapchain->swapchainExtent = internal_crenvk_choose_swapchain_extent(&details.capabilities, width, height);

    // images in the swapchain
    swapchain->swapchainImageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && swapchain->swapchainImageCount > details.capabilities.maxImageCount) swapchain->swapchainImageCount = details.capabilities.maxImageCount;

    // create swapchain
    vkQueueFamilyIndices indices = crenvk_device_find_queue_families(physicalDevice, surface);
    int queueFamilyIndices[] = { indices.graphicFamily, indices.presentFamily, indices.computeFamily };
    VkSwapchainCreateInfoKHR swapchainCI = { 0 };
    swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCI.pNext = NULL;
    swapchainCI.flags = 0;
    swapchainCI.surface = surface;
    swapchainCI.minImageCount = swapchain->swapchainImageCount;
    swapchainCI.imageFormat = swapchain->swapchainFormat.format;
    swapchainCI.imageColorSpace = swapchain->swapchainFormat.colorSpace;
    swapchainCI.imageExtent = swapchain->swapchainExtent;
    swapchainCI.imageArrayLayers = 1;
    swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // allow copying swapchain images
    swapchainCI.preTransform = details.capabilities.currentTransform;
    swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCI.presentMode = swapchain->swapchainPresentMode;
    swapchainCI.clipped = VK_TRUE;

    if (indices.graphicFamily != indices.presentFamily) {
        swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCI.queueFamilyIndexCount = 2;
        swapchainCI.pQueueFamilyIndices = (unsigned int*)queueFamilyIndices;
    }

    else {
        swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    CREN_ASSERT(vkCreateSwapchainKHR(device, &swapchainCI, NULL, &swapchain->swapchain) == VK_SUCCESS, "Failed to create swapchain");

    vkGetSwapchainImagesKHR(device, swapchain->swapchain, &swapchain->swapchainImageCount, NULL);
    swapchain->swapchainImages = (VkImage*)malloc(swapchain->swapchainImageCount * sizeof(VkImage));
    swapchain->swapchainImageViews = (VkImageView*)malloc(sizeof(VkImageView) * swapchain->swapchainImageCount);
    vkGetSwapchainImagesKHR(device, swapchain->swapchain, &swapchain->swapchainImageCount, swapchain->swapchainImages);

    // create image views
    for (unsigned int i = 0; i < swapchain->swapchainImageCount; i++) {
        crenvk_device_create_image_view(device, swapchain->swapchainImages[i], swapchain->swapchainFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, VK_IMAGE_VIEW_TYPE_2D, NULL, &swapchain->swapchainImageViews[i]);
    }

    // syncronization objects
    VkSemaphoreCreateInfo semaphoreCI = { 0 };
    semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCI.pNext = NULL;
    semaphoreCI.flags = 0;

    VkFenceCreateInfo fenceCI = { 0 };
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.pNext = NULL;
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    swapchain->imageAvailableSemaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * swapchain->swapchainImageCount);
    swapchain->finishedRenderingSemaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * swapchain->swapchainImageCount);
    swapchain->framesInFlightFences = (VkFence*)malloc(sizeof(VkFence) * swapchain->swapchainImageCount);

    swapchain->swapchainSyncCount = swapchain->swapchainImageCount;
    for (size_t i = 0; i < swapchain->swapchainSyncCount; i++) {
        CREN_ASSERT(vkCreateSemaphore(device, &semaphoreCI, NULL, &swapchain->imageAvailableSemaphores[i]) == VK_SUCCESS, "Failed to create image available semaphore");
        CREN_ASSERT(vkCreateSemaphore(device, &semaphoreCI, NULL, &swapchain->finishedRenderingSemaphores[i]) == VK_SUCCESS, "Failed to create rendering finished semaphore");
        CREN_ASSERT(vkCreateFence(device, &fenceCI, NULL, &swapchain->framesInFlightFences[i]) == VK_SUCCESS, "Failed to create syncronizer fence");
    }

    // free details
    free(details.presentModes);
    free(details.surfaceFormats);
}

void crenvk_swapchain_destroy(vkSwapchain* swapchain, VkDevice device)
{
    for (unsigned int i = 0; i < swapchain->swapchainSyncCount; i++) {
        if (swapchain->imageAvailableSemaphores[i]) vkDestroySemaphore(device, swapchain->imageAvailableSemaphores[i], NULL);
        if (swapchain->finishedRenderingSemaphores[i]) vkDestroySemaphore(device, swapchain->finishedRenderingSemaphores[i], NULL);
        if (swapchain->framesInFlightFences[i]) vkDestroyFence(device, swapchain->framesInFlightFences[i], NULL);
    }
    free(swapchain->imageAvailableSemaphores);
    free(swapchain->finishedRenderingSemaphores);
    free(swapchain->framesInFlightFences);

    for (unsigned int i = 0; i < swapchain->swapchainImageCount; i++) vkDestroyImageView(device, swapchain->swapchainImageViews[i], NULL);

    free(swapchain->swapchainImageViews);
    free(swapchain->swapchainImages); // swapchain images are vkDestroyed internally

    vkDestroySwapchainKHR(device, swapchain->swapchain, NULL);
}