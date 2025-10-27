#include "cren_primitives.h"

#include "cren_context.h"
#include "cren_error.h"
#include "Vulkan/crenvk_primitives.h"
#include <memm/memm.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CRenTexture2D
{
    const char* path;
    int32_t width;
    int32_t height;
    int32_t channels;
    int32_t mipLevels;
    #ifdef CREN_BUILD_WITH_VULKAN
    CRenVKTexture2D* backend;
    #endif
};

CREN_API CRenTexture2D* cren_texture2d_create_from_path(CRenContext* context, const char* path, bool uiTexture)
{
    CRenTexture2D* texture = (CRenTexture2D*)malloc(sizeof(CRenTexture2D));
    texture->backend = NULL;
    if (!texture) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate memory for CRenTexture2D");
        return NULL;
    }

    texture->path = path;

    #ifdef CREN_BUILD_WITH_VULKAN
    VkResult result = crenvk_texture2d_create_from_path
    (
        cren_get_vulkan_backend(context),
        path,
        uiTexture,
        cren_using_custom_viewport(context),
        &texture->backend,
        &texture->width,
        &texture->height,
        &texture->mipLevels
    );
    if (result != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create vulkan Texture 2D");

        free(texture);
        return NULL;
    }
    #endif

    return texture;
}

CREN_API CRenTexture2D* cren_texture2d_create_from_buffer(CRenContext* context, uint8_t* buffer, size_t bufferLen, int32_t width, int32_t height, bool uiTexture)
{
    CRenTexture2D* texture = (CRenTexture2D*)malloc(sizeof(CRenTexture2D));
    if (!texture) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to allocate memory for CRenTexture2D");
        return NULL;
    }

    texture->width = width;
    texture->height = height;

    #ifdef CREN_BUILD_WITH_VULKAN
    
    VkResult result = crenvk_texture2d_create_from_buffer
    (
        cren_get_vulkan_backend(context),
        buffer,
        bufferLen,
        width,
        height,
        uiTexture,
        cren_using_custom_viewport(context),
        &texture->backend,
        &texture->mipLevels
    );
    if (result != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create vulkan Texture 2D");

        free(texture);
        return NULL;
    }
    #endif

    return texture;
}

CREN_API void cren_texture2d_destroy(CRenContext* context, CRenTexture2D* texture)
{
    if (!context || !texture) return;

    #ifdef CREN_BUILD_WITH_VULKAN
    if (texture->backend) crenvk_texture2d_destroy(cren_get_vulkan_backend(context), texture->backend);
    #endif

    free(texture);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quad
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CRenQuad
{
    uint32_t id;
    CRenTexture2D* texture;

    #ifdef CREN_BUILD_WITH_VULKAN
    CRenVKQuad* backend;
    #endif
};

CREN_API CRenQuad* cren_quad_create(CRenContext* context, const char* albedoPath, uint32_t id)
{
    CRenQuad* quad = (CRenQuad*)malloc(sizeof(CRenQuad));
    if (!quad) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create CRen Quad");
        return NULL;
    }

    quad->id = id;
    quad->texture = cren_texture2d_create_from_path(context, albedoPath, false);
    if (!quad->texture) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create albedo texture for CRen Quad");
        free(quad);
        return NULL;
    }

    #ifdef CREN_BUILD_WITH_VULKAN
    quad->backend = NULL;
    VkResult res = crenvk_quad_create_from_path(cren_get_vulkan_backend(context), albedoPath, cren_using_custom_viewport(context), &quad->backend);
    if (res != VK_SUCCESS) {
        CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Failed to create CRen Quad vulkan backend");
        crenvk_texture2d_destroy(cren_get_vulkan_backend(context), quad->texture->backend);
        free(quad);
        return NULL;
    }

    crenvk_quad_update_descriptors(cren_get_vulkan_backend(context), quad->backend, quad->texture->backend);
    #endif

    return quad;
}

CREN_API void cren_quad_destroy(CRenContext* context, CRenQuad* quad)
{
    if (!context || !quad) return;

    cren_texture2d_destroy(context, quad->texture);
    #ifdef CREN_BUILD_WITH_VULKAN
    crenvk_quad_destroy(cren_get_vulkan_backend(context), quad->backend);
    #endif
    free(quad);
}

CREN_API void cren_quad_update(CRenContext* context, CRenQuad* quad)
{
    #ifdef CREN_BUILD_WITH_VULKAN
    crenvk_quad_update(cren_get_vulkan_backend(context), quad->backend);
    #endif
}

CREN_API void cren_quad_render(CRenContext* context, CRenQuad* quad, CRen_RenderStage stage, const fmat4 modelMatrix)
{
    #ifdef CREN_BUILD_WITH_VULKAN
    crenvk_quad_render(cren_get_vulkan_backend(context), quad->backend, stage, modelMatrix, quad->id, cren_using_custom_viewport(context));
    #endif
}

CREN_API uint32_t cren_quad_get_id(CRenContext* context, CRenQuad* quad)
{
    if (!context) return 0;
    return quad ? quad->id : 0;
}

CREN_API bool cren_quad_get_billboard(CRenContext* context, CRenQuad* quad)
{
    if (!context) return false;

    if (quad) {
        if (quad->backend) {
            return quad->backend->params.billboard == 1.0f ? true : false;
        }
   }

    return false;
}

CREN_API void cren_quad_set_billboard(CRenContext* context, CRenQuad* quad, bool value)
{
    if (!context) return;
    if (quad) {
        if (quad->backend) {
            if (value) {
                quad->backend->params.billboard = 1.0f;
            }

            else {
                quad->backend->params.billboard = 0.0f;
            }

            cren_quad_update(context, quad);
        }
    }
}

CREN_API bool cren_quad_get_lock_axis_x(CRenContext* context, CRenQuad* quad)
{
    if (!context) return false;
    if (quad) {
        return quad->backend->params.lockAxis.xy.x == 1.0f ? true : false;
    }

    return false;
}

CREN_API void cren_quad_set_lock_axis_x(CRenContext* context, CRenQuad* quad, bool lock)
{
    if (!context) return;
    if (quad) {
        if (lock) {
            quad->backend->params.lockAxis.xy.x = 1.0f;
        }

        else {
            quad->backend->params.lockAxis.xy.x = 0.0f;
        }

        cren_quad_update(context, quad);
    }
}

CREN_API bool cren_quad_get_lock_axis_y(CRenContext* context, CRenQuad* quad)
{
    if (!context) return false;
    if (quad) {
        return quad->backend->params.lockAxis.xy.y == 1.0f ? true : false;
    }

    return false;
}

CREN_API void cren_quad_set_lock_axis_y(CRenContext* context, CRenQuad* quad, bool lock)
{
    if (!context) return;
    if (quad) {
        if (lock) {
            quad->backend->params.lockAxis.xy.y = 1.0f;
        }

        else {
            quad->backend->params.lockAxis.xy.y = 0.0f;
        }

        cren_quad_update(context, quad);
    }
}
