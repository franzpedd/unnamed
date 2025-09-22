#include "crenvk_context.h"

CRenVulkanBackend crenvk_initialize(unsigned int width, unsigned int height, void* window, const char* appName, const char* rootPath, unsigned int appVersion, CRen_Renderer api, bool vsync, bool msaa, bool validations, bool customViewport)
{
    CRenVulkanBackend backend = { 0 };
    crenvk_instance_create(&backend.instance, appName, appVersion, api, validations);
    crenvk_device_create()
    return backend;
}
