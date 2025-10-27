#ifndef CREN_BUFFER_INCLUDED
#define CREN_BUFFER_INCLUDED

#include "cren_defines.h"
#include <vecmath/vecmath.h>

/// @brief this buffer holds information that get's passed each frame to the shader, it's max size is 128 bytes (80 so far), reserved for model and id only
typedef struct BufferConstant
{
	align_as(4) uint32_t id;
	align_as(16) fmat4 model;
} BufferConstant;

/// @brief this buffer holds information about a camera's data, if multiple cameras exists each one should have access to it's own buffer
typedef struct BufferCamera
{
	align_as(16) fmat4 view;
	align_as(16) fmat4 viewInverse;
	align_as(16) fmat4 proj;
} BufferCamera;

/// @brief this buffer holds information about a quad, each quad has it's own buffer and serves the pourpuses of manipulating the behavior of the quad
typedef struct BufferQuad
{
	align_as(4) float billboard;	    // hints to always faces the camera
	align_as(4) float uv_rotation;		// rotates the uv/texture
	align_as(8) float2 lockAxis;	    // controls wich axis to lock
	align_as(8) float2 uv_offset;	    // used to offset the uv/texture
	align_as(8) float2 uv_scale;	    // used to scale the uv/texture
} BufferQuad;

#endif // CREN_BUFFER_INCLUDED