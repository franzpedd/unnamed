#include "cren_fun.h"

CREN_API uint32_t cren_pick_object(CRenContext* context, float2 screenCoord)
{
	if (!context) return 0;
	uint32_t res = 0;
	#ifdef CREN_BUILD_WITH_VULKAN
	res = crenvk_pick_object(context, &context->backend, screenCoord);
	#else 
	#error "Undefined backend";
	#endif
	return res;
}