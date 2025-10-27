#ifndef CREN_PRIMITIVES_INCLUDED
#define CREN_PRIMITIVES_INCLUDED

#include "cren_defines.h"
#include "cren_types.h"
#include <vecmath/vecmath.h>

#ifdef __cplusplus 
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture, this is most used internally or used by an user interface
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief creates a 2D texture based on it's path
CREN_API CRenTexture2D* cren_texture2d_create_from_path(CRenContext* context, const char* path, bool uiTexture);

/// @brief creates a 2D texture based on a memory buffer/data
CREN_API CRenTexture2D* cren_texture2d_create_from_buffer(CRenContext* context, uint8_t* buffer, size_t bufferLen, int32_t width, int32_t height, bool uiTexture);

/// @brief releases and free all resources used by the texture object
CREN_API void cren_texture2d_destroy(CRenContext* context, CRenTexture2D* texture);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quad
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief creates a quad primitive object
CREN_API CRenQuad* cren_quad_create(CRenContext* context, const char* albedoPath, uint32_t id);

/// @brief destroys a quad object
CREN_API void cren_quad_destroy(CRenContext* context, CRenQuad* quad);

/// @brief sends the quad data to the gpu
CREN_API void cren_quad_update(CRenContext* context, CRenQuad* quad);

/// @brief renders the quad into the world
CREN_API void cren_quad_render(CRenContext* context, CRenQuad* quad, CRen_RenderStage stage, const fmat4 modelMatrix);

/// @brief returns the quad's id
CREN_API uint32_t cren_quad_get_id(CRenContext* context, CRenQuad* quad);

/// @brief returns if the quad is acting like a billboard
CREN_API bool cren_quad_get_billboard(CRenContext* context, CRenQuad* quad);

/// @brief makes the quad act like a billboard
CREN_API void cren_quad_set_billboard(CRenContext* context, CRenQuad* quad, bool value);

/// @brief returns if the quad x axis is currently locked
CREN_API bool cren_quad_get_lock_axis_x(CRenContext* context, CRenQuad* quad);

/// @brief locks/unlocks the x axis of a quad
CREN_API void cren_quad_set_lock_axis_x(CRenContext* context, CRenQuad* quad, bool lock);

/// @brief returns if the quad y axis is currently locked
CREN_API bool cren_quad_get_lock_axis_y(CRenContext* context, CRenQuad* quad);

/// @brief locks/unlocks the y axis of a quad
CREN_API void cren_quad_set_lock_axis_y(CRenContext* context, CRenQuad* quad, bool lock);

#ifdef __cplusplus 
}
#endif

#endif // CREN_PRIMITIVES_INCLUDED