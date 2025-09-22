#include "cren_platform.h"

#include <stdio.h>
#include <string.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include <stb/stb_image.h>

CRen_Platform cren_detect_platform()
{
    #if defined(_WIN32)
        return CREN_PLATFORM_WINDOWS;
    #elif defined(__APPLE__) && defined(__MACH__)
        #if TARGET_OS_IPHONE
            return CREN_PLATFORM_IOS;
        #elif TARGET_OS_OSX
            return CREN_PLATFORM_MACOS;
        #else
            return CREN_PLATFORM_UNKNOWN_APPLE;
    #endif
    #elif defined(__ANDROID__)
        return CREN_PLATFORM_ANDROID;
    #elif defined(__linux__)
        if (getenv("WAYLAND_DISPLAY")) return CREN_PLATFORM_WAYLAND;
        else if (getenv("DISPLAY")) return CREN_PLATFORM_X11;
    #else
        return CREN_PLATFORM_UNKNOWN;
    #endif
}

bool cren_surface_create(void* instance, void* surface, void* nativeWindow)
{
    #ifdef CREN_BUILD_WITH_VULKAN
    VkResult result = VK_ERROR_EXTENSION_NOT_PRESENT;
    VkInstance vkInstance = (VkInstance)instance;
    VkSurfaceKHR* vkSurface = (VkSurfaceKHR*)surface;

    #ifdef PLATFORM_WINDOWS
    VkWin32SurfaceCreateInfoKHR createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = GetModuleHandle(NULL);
    createInfo.hwnd = (HWND)nativeWindow;
    PFN_vkCreateWin32SurfaceKHR fn = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(vkInstance, "vkCreateWin32SurfaceKHR");
    result = fn(vkInstance, &createInfo, NULL, vkSurface);
    #elif defined(PLATFORM_APPLE)
    VkMetalSurfaceCreateInfoEXT createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    createInfo.pLayer = (CAMetalLayer*)nativeWindow;
    PFN_vkCreateMetalSurfaceEXT fn = (PFN_vkCreateMetalSurfaceEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateMetalSurfaceEXT");
    result = fn(vkInstance, &createInfo, NULL, vkSurface);
    #elif defined(PLATFORM_ANDROID)
    VkAndroidSurfaceCreateInfoKHR createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.window = (ANativeWindow*)nativeWindow;
    PFN_vkCreateAndroidSurfaceKHR fn = (PFN_vkCreateAndroidSurfaceKHR)vkGetInstanceProcAddr(vkInstance, "vkCreateAndroidSurfaceKHR");
    result = fn(vkInstance, &createInfo, NULL, vkSurface);
    #elif defined(__WAYLAND__)
    VkWaylandSurfaceCreateInfoKHR createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.display = ((struct wl_display*)nativeWindow);
    createInfo.surface = ((struct wl_surface*)nativeWindow);
    PFN_vkCreateWaylandSurfaceKHR fn = (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(vkInstance, "vkCreateWaylandSurfaceKHR");
    result = fn(vkInstance, &createInfo, NULL, vkSurface);
    #elif defined(__X11__)
    VkXlibSurfaceCreateInfoKHR createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.dpy = XOpenDisplay(NULL);
    createInfo.window = (Window)nativeWindow;
    PFN_vkCreateXlibSurfaceKHR fn = (PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(vkInstance, "vkCreateXlibSurfaceKHR");
    result = fn(vkInstance, &createInfo, NULL, vkSurface);
    #endif

    return result == VK_SUCCESS;
    #else
    return -1000174001; // VK_ERROR_NOT_PERMITTED
    #endif
}

void cren_get_path(const char* subpath, const char* assetsRoot, int removeExtension, char* output, unsigned long long outputSize)
{
    snprintf(output, outputSize, "%s/%s", assetsRoot, subpath);

    if (removeExtension == 1) {
        char* lastDot = strrchr(output, '.'); // find the last '.' in the string
        if (lastDot) {
            *lastDot = '\0'; // truncate the string at the last '.'
        }
    }
}

unsigned char* cren_stbimage_load_from_file(const char* path, int desiredChannels, int* outWidth, int* outHeight, int* outChannels)
{
    int x, y, channels = 0;
    stbi_uc* pixels = stbi_load(path, &x, &y, &channels, desiredChannels);

    *outWidth = x;
    *outHeight = y;
    *outChannels = channels;
    return pixels;
}

void cren_stbimage_destroy(unsigned char* ptr)
{
    stbi_image_free(ptr);
}

const char* cren_stbimage_get_error()
{
    return stbi_failure_reason();
}
