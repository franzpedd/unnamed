#ifndef CREN_INCLUDED
#define CREN_INCLUDED

#include "cren_callbacks.h"
#include "cren_camera.h"
#include "cren_context.h"
#include "cren_defines.h"
#include "cren_error.h"
#include "cren_platform.h"
#include "cren_primitives.h"
#include "cren_types.h"

#ifdef CREN_BUILD_WITH_VULKAN
#include "Vulkan/crenvk_buffer.h"
#include "Vulkan/crenvk_context.h"
#include "Vulkan/crenvk_core.h"
#include "Vulkan/crenvk_pipeline.h"
#include "Vulkan/crenvk_primitives.h"
#include "Vulkan/crenvk_renderphase.h"
#endif

#endif // CREN_INCLUDED