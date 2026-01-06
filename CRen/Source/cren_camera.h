#ifndef CREN_CAMERA_INCLUDED
#define CREN_CAMERA_INCLUDED

#include "cren_defines.h"
#include "cren_types.h"
#include <vecmath/vecmath.h>

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief creates and returns a camera
CREN_API CRenCamera* cren_camera_create(CRen_CameraType type, float initialAspectRatio, CRen_RendererAPI api);

/// @brief free camera resources
CREN_API void cren_camera_destroy(CRenCamera* camera);

/// @brief updates the camera logic for that frame
CREN_API void cren_camera_update(CRenCamera* camera, float timestep);

/// @brief sets a new aspect ratio for the camera
CREN_API void cren_camera_set_aspect_ratio(CRenCamera* camera, float aspect);

/// @brief returns the camera's current aspect ratio
CREN_API float cren_camera_get_aspect_ratio(CRenCamera* camera);

/// @brief returns the camera's current field of view
CREN_API float cren_camera_get_fov(CRenCamera* camera);

/// @brief applys a translation to the camera's view, moving the camera
CREN_API void cren_camera_translate(CRenCamera* camera, float3 dir);

/// @brief applys a rotation to the camera's view, rotating the camera
CREN_API void cren_camera_rotate(CRenCamera* camera, float3 dir);

/// @brief returns the camera's view matrix
CREN_API fmat4 cren_camera_get_view(CRenCamera* camera);

/// @brief returns the camera's inverse view matrix
CREN_API fmat4 cren_camera_get_view_inverse(CRenCamera* camera);

/// @brief returns the camera's perspective projection matrix
CREN_API fmat4 cren_camera_get_perspective(CRenCamera* camera);

/// @brief returns the camera's inverse perspective projection matrix
CREN_API fmat4 cren_camera_get_perspective_inverse(CRenCamera* camera);

/// @brief enables/disables the camera movement
CREN_API void cren_camera_set_lock(CRenCamera* camera, bool value);

/// @brief returns if camera can currently move
CREN_API bool cren_camera_get_lock(CRenCamera* camera);

/// @brief moves/stops moving the camera towards a direction
CREN_API void cren_camera_move(CRenCamera* camera, CRen_CameraDirection dir, bool moving);

/// @brief returns if the camera speed modifier is currently applyed
CREN_API bool cren_camera_get_speed_modifier(CRenCamera* camera, float* value);

/// @brief sets/unsets the camera's speed modifier
CREN_API void cren_camera_set_speed_modifier(CRenCamera* camera, bool status, float value);

/// @brief returns the camera's current 3d position
CREN_API float3 cren_camera_get_position(CRenCamera* camera);

/// @brief returns the camera's current 3d front position
CREN_API float3 cren_camera_get_front(CRenCamera* camera);

#ifdef __cplusplus 
}
#endif

#endif // CREN_CAMERA_INCLUDED