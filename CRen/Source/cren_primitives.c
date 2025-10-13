#include "cren_primitives.h"

#include "cren_context.h"
#include "cren_error.h"
#include <memm/memm.h>

typedef struct CRenQuadParams
{
    align_as(4) unsigned int billboard;	// hints to always faces the camera
    align_as(4) float uv_rotation;		// rotates the uv/texture
    align_as(8) float2 lockAxis;	    // controls wich axis to lock
    align_as(8) float2 uv_offset;	    // used to offset the uv/texture
    align_as(8) float2 uv_scale;	    // used to scale the uv/texture
} CRenQuadParams;

struct CRenQuad
{
    unsigned long long id;
    CRenQuadParams params;

    #ifdef CREN_BUILD_WITH_VULKAN
    //CRenVulkanQuadBackend backend;
    #endif
};

CREN_API CRenQuad* cren_quad_create(CRenContext* context, const char* albedoPath)
{
    CRenQuad* quad = (CRenQuad*)malloc(sizeof(CRenQuad));
    if (!quad) return NULL;

    quad->id = cren_create_id(context);
    quad->params.billboard = 1;
    quad->params.uv_rotation = 0.0f;
    quad->params.lockAxis.xy.x = quad->params.lockAxis.xy.y = 0.0f;
    quad->params.uv_offset.xy.x = quad->params.uv_offset.xy.y = 0.0f;
    quad->params.uv_scale.xy.x = quad->params.uv_scale.xy.y = 1.0f;
    //
    //VkResult res = crenvk_quad_create(context, &quad, albedoPath);
    //if (res != VK_SUCCESS) {
    //    CREN_LOG(CRenLogSeverity_Error, "Failed to create vulkan quad backend");
    //}
    //

    return quad;
}

CREN_API void cren_quad_destroy(CRenContext* context, CRenQuad* quad)
{
    if (!context || !quad) return;

    if (cren_unregister_id(context, quad->id)) {
        CREN_LOG(CREN_LOG_SEVERITY_WARN, "Key %d could not be unregistered, this may indicate a bug", quad->id);
    }

    free(quad);
}

CREN_API void cren_quad_render(CRenContext* context, CRenQuad* quad, const fmat4 transform)
{
	//return CREN_API void();
}

CREN_API uint32_t cren_quad_get_id(CRenQuad* quad)
{
    return quad ? quad->id : 0;
}
