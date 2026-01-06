#ifndef CRENVK_FUN_INCLUDED
#define CRENVK_FUN_INCLUDED
#ifdef CREN_BUILD_WITH_VULKAN 

#include "cren_defines.h"
#include "cren_types.h"
#include <vecmath/vecmath.h>

/// @brief converts mouse coordinates into framebuffer coordinates
CREN_API float2 cren_screen_to_framebuffer_coord(float2 screenCoord, float2 windowSize, float2 framebufferExtent);

#endif // CREN_BUILD_WITH_VULKAN
#endif // CRENVK_FUN_INCLUDED