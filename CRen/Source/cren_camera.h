#ifndef CREN_CAMERA_INCLUDED
#define CREN_CAMERA_INCLUDED

#include "cren_defines.h"
#include "cren_types.h"
#include <vecmath/vecmath.h>

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief defines multiple camera types
typedef enum CRen_CameraType
{
	CREN_CAMERA_TYPE_LOOK_AT,
	CREN_CAMERA_TYPE_FREE_LOOK
} CRen_CameraType;

/// @brief cren camera data
typedef struct CRenCamera
{
	CRen_CameraType type;
	CRen_Renderer api;

	// definitions
	float fov;
	float near;
	float far;
	float aspectRatio;
	float movementSpeed;
	float rotationSpeed;
	float modifierSpeed;

	// math
	fmat4 perspective;
	fmat4 view;
	float3 rotation;
	float3 position;
	float3 scale;
	float3 viewPosition;
	float3 frontPosition;

	// movement
	bool shouldMove;
	bool modifierPressed;
	bool movingForward;
	bool movingBackward;
	bool movingLeft;
	bool movingRight;
} CRenCamera;

/// @brief creates and returns a camera
CREN_API CRenCamera cren_camera_create(CRen_CameraType type, float initialAspectRatio, CRen_Renderer api);

/// @brief updates the camera frame
/// @param camera cren's camera memory address
/// @param timestep interpolation between frames
CREN_API void cren_camera_update(CRenCamera* camera, float timestep);

/// @brief sets a new aspect ratio for the camera
/// @param camera camera's memory address
/// @param aspect new aspect ratio
CREN_API void cren_camera_set_aspect_ratio(CRenCamera* camera, float aspect);

/// @brief applys a translation to the camera's view, moving the camera
/// @param camera camera's memory address
/// @param dir the delta vector to move the camera towards to
CREN_API void cren_camera_translate(CRenCamera* camera, float3 dir);

/// @brief applys a rotation to the camera's view, rotating the camera
/// @param camera camera's memory address
/// @param dir the delta vector to rotate the camera towards for
CREN_API void cren_camera_rotate(CRenCamera* camera, float3 dir);

#ifdef __cplusplus 
}
#endif

#endif // CREN_CAMERA_INCLUDED