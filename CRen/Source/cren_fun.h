#ifndef CREN_FUN_INCLUDED
#define CREN_FUN_INCLUDED

#include "cren_defines.h"
#include "cren_types.h"
#include <vecmath/vecmath.h>

/// @brief attempts to pick an object on the screen based on screen coordinates
CREN_API uint32_t cren_pick_object(CRenContext* context, float2 screenCoord);

#endif // CREN_FUN_INCLUDED