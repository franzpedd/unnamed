#ifndef CRENVK_RENDERPHASE
#define CRENVK_RENDERPHASE
#ifdef CREN_BUILD_WITH_VULKAN

#include "cren_defines.h"
#include "cren_types.h"
#include "cren_callbacks.h"

#include "crenvk_core.h"
#include "crenvk_pipeline.h"

/// @brief default renderphase
typedef struct vkDefaultRenderphase
{
    vkRenderpass* renderpass;
    vkPipeline* pipeline;

    VkDeviceSize defaultImageSize;
    VkImage colorImage;
    VkImage depthImage;
    VkDeviceMemory colorMemory;
    VkDeviceMemory depthMemory;
    VkImageView colorView;
    VkImageView depthView;
    VkFormat surfaceFormat;
    VkFormat depthFormat;

} vkDefaultRenderphase;

/// @brief picking renderphase, holding information about the object's id on the framebuffer
typedef struct vkPickingRenderphase
{
    vkRenderpass* renderpass;
    vkPipeline* pipeline;

    VkFormat surfaceFormat;
    VkFormat depthFormat;
    uint32_t imageSize;

    VkImage depthImage;
    VkDeviceMemory depthMemory;
    VkImageView depthView;
    VkImage* colorImage;
    VkDeviceMemory* colorMemory;
    VkImageView* colorView;
    uint32_t colorImageCount;
} vkPickingRenderphase;

/// @brief user-interface renderphase, very simple but "incomplete" since multiple choices of UI may be choosen, like ImGui, Nuklear and they might require more objects, this is enough for ImGui though
typedef struct vkUIRenderphase
{
    vkRenderpass* renderpass;
} vkUIRenderphase;

/// @brief optional viewport renderphase, uses it's own resources for managing the scene more appropriately, only drawing the scene on itself instead of the main renderphase
typedef struct vkViewportRenderphase
{
    vkRenderpass* renderpass;

    VkImage colorImage;
    VkDeviceMemory colorMemory;
    VkImageView colorView;

    VkImage depthImage;
    VkDeviceMemory depthMemory;
    VkImageView depthView;

    VkSampler sampler;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;
} vkViewportRenderphase;

#ifdef __cplusplus 
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Default
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief creates the main/default renderphase object
CREN_API VkResult crenvk_renderphase_default_create(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkFormat format, VkSampleCountFlagBits msaa, bool finalPhase, vkDefaultRenderphase* outPhase);

/// @brief releases all resources used by the main/default renderphase
CREN_API void crenvk_renderphase_default_destroy(vkDefaultRenderphase* renderphase, VkDevice device, bool destroyRenderpass, bool destroyPipeline);

/// @brief creates the framebuffers used by the main/default renderphase
CREN_API VkResult crenvk_renderphase_default_create_framebuffers(vkDefaultRenderphase* phase, VkDevice device, VkPhysicalDevice physicalDevice, VkImageView* views, uint32_t viewsCount, VkExtent2D extent, VkFormat colorFormat);

/// @brief recreates the default renderphase
CREN_API void crenvk_renderphase_default_recreate(vkDefaultRenderphase* phase, vkSwapchain* swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSampleCountFlagBits msaa, VkExtent2D extent, bool finalPhase, bool vsync);

/// @brief updates the renderphase
CREN_API void crenvk_renderphase_default_update(vkDefaultRenderphase* phase, void* context, void* backend, uint32_t currentFrame, uint32_t swapchainImageIndex, bool usingViewport, float timestep, CRenCallback_Render callback);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Picking
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief creates the picking renderphase
CREN_API VkResult crenvk_renderphase_picking_create(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, vkPickingRenderphase* outPhase);

/// @brief releases all resources used in the picking renderphase
CREN_API void crenvk_renderphase_picking_destroy(vkPickingRenderphase* phase, VkDevice device, bool destroyRenderpass, bool destroyPipeline);

/// @brief creates the picking framebuffers
CREN_API VkResult crenvk_renderphase_picking_create_framebuffers(vkPickingRenderphase* phase, VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkImageView* views, uint32_t viewsCount, VkExtent2D extent);

/// @brief recreates the picking renderphase
CREN_API void crenvk_renderphase_picking_recreate(vkPickingRenderphase* phase, vkSwapchain* swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, VkExtent2D extent);

/// @brief updates the picking renderphase
CREN_API void crenvk_renderphase_picking_update(vkPickingRenderphase* phase, void* context, void* backend, uint32_t currentFrame, uint32_t swapchainImageIndex, bool usingViewport, float timestep, CRenCallback_Render callback);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UI
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief creates the ui renderphase
CREN_API VkResult crenvk_renderphase_ui_create(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkFormat format, VkSampleCountFlagBits msaa, bool finalPhase, vkUIRenderphase* outPhase);

/// @brief releases all resources used in the ui renderphase
CREN_API void crenvk_renderphase_ui_destroy(vkUIRenderphase* phase, VkDevice device, bool destroyRenderpass);

/// @brief creates the ui framebuffers
CREN_API VkResult crenvk_renderphase_ui_framebuffers_create(vkUIRenderphase* phase, VkDevice device, VkExtent2D extent, VkImageView* swapchainViews, uint32_t swapchainViewsCount);

/// @brief recreates the ui renderphase
CREN_API void crenvk_renderphase_ui_recreate(vkUIRenderphase* phase, vkSwapchain* swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkExtent2D extent);

/// @brief updates the ui renderphase
CREN_API void crenvk_renderphase_ui_update(vkUIRenderphase* phase, void* context, void* backend, uint32_t currentFrame, uint32_t swapchainImageIndex, CRenCallback_DrawUIRawData callback);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Viewport
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief creates the viewport renderphase
CREN_API VkResult crenvk_renderphase_viewport_create(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkFormat format, VkSampleCountFlagBits msaa, vkViewportRenderphase* outPhase);

/// @brief releases all resources used in the viewport renderphase
CREN_API void crenvk_renderphase_viewport_destroy(vkViewportRenderphase* phase, VkDevice device, bool destroyRenderpass);

/// @brief creates the framebuffers used in the viewport renderphase
CREN_API VkResult crenvk_renderphase_viewport_create_framebuffers(vkViewportRenderphase* phase, VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkImageView* swapchainViews, uint32_t swapchainViewCount, VkExtent2D extent);

/// @brief recreates the viewport renderphase
CREN_API void crenvk_renderphase_viewport_recreate(vkViewportRenderphase* phase, vkSwapchain* swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, VkExtent2D extent);

/// @brief updates the viewport renderphase
CREN_API void crenvk_renderphase_viewport_update(vkViewportRenderphase* phase, void* context, void* backend, uint32_t currentFrame, uint32_t swapchainImageIndex, bool usingViewport, float timestep, CRenCallback_Render callback);

#ifdef __cplusplus 
}
#endif

#endif // CREN_BUILD_WITH_VULKAN
#endif // CRENVK_RENDERPHASE