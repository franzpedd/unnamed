#ifndef CREN_PRIMITIVES_INCLUDED
#define CREN_PRIMITIVES_INCLUDED

#include "cren_defines.h"
#include "cren_types.h"
#include <vecmath/vecmath.h>

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief creates a quad primitive object
CREN_API CRenQuad* cren_quad_create(CRenContext* context, const char* albedoPath);

/// @brief destroys a quad object
CREN_API void cren_quad_destroy(CRenContext* context, CRenQuad* quad);

/// @brief renders the quad based on a 3d transform matrix
CREN_API void cren_quad_render(CRenContext* context, CRenQuad* quad, const fmat4 transform);

/// @brief returns the id of a quad object
CREN_API uint32_t cren_quad_get_id(CRenQuad* quad);

#ifdef __cplusplus 
}
#endif

#endif // CREN_PRIMITIVES_INCLUDED