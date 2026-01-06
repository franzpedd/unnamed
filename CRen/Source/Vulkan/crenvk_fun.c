#include "crenvk_util.h"

CREN_API float2 cren_screen_to_framebuffer_coord(float2 screenCoord, float2 windowSize, float2 framebufferExtent)
{
    float2 normalized = {
        screenCoord.xy.x / windowSize.xy.x,
        screenCoord.xy.y / windowSize.xy.y
    };

    float2 pickingCoords = {
        normalized.xy.x * framebufferExtent.xy.x,
        (1.0f - normalized.xy.y)* framebufferExtent.xy.y
    };

    pickingCoords.xy.x = f_clamp(pickingCoords.xy.x, 0.0f, (float)framebufferExtent.xy.x - 1.0f);
    pickingCoords.xy.y = f_clamp(pickingCoords.xy.y, 0.0f, (float)framebufferExtent.xy.y - 1.0f);

    return pickingCoords;
}
