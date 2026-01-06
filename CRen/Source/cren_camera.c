#include "cren_camera.h"

#include "cren_error.h"
#include <memm/memm.h>
#include <vecmath/vecmath.h>

/// @brief cren camera data
struct CRenCamera
{
	CRen_CameraType type;
	CRen_RendererAPI api;

	// definitions
	float fov;
	float near;
	float far;
	float aspectRatio;
	float movementSpeed;
	float rotationSpeed;
	float modifierSpeed;

	// math
	fmat4 view;
	fmat4 viewInverse;
	fmat4 perspective;
	fmat4 perspectiveInverse;
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
};

/// @brief updates the view projection matrix of a camera
static void internal_camera_update_view_matrix(CRenCamera* camera) 
{
	// calculate target point
	float3 target = float3_add(&camera->position, &camera->frontPosition);
	const float3 worldUp = { 0.0f, 1.0f, 0.0f };
	
	// create the view matrix
	camera->view = fmat4_lookat_vulkan(&camera->position, &target, &worldUp);
	camera->viewPosition = camera->position;
}

CREN_API CRenCamera* cren_camera_create(CRen_CameraType type, float initialAspectRatio, CRen_RendererAPI api)
{
	CRenCamera* camera = (CRenCamera*)malloc(sizeof(CRenCamera));
	CREN_ASSERT(camera != NULL, "Failed to allocate memory for CRenCamera");

	camera->type = type;
	camera->api = api;
	camera->fov = 45.0f;
	camera->near = 0.1f;
	camera->far = 256.0f;
	camera->aspectRatio = initialAspectRatio;
	camera->movementSpeed = 1.0f;
	camera->rotationSpeed = 1.0f;
	camera->modifierSpeed = 2.5f;

	camera->perspective = fmat4_identity();
	camera->perspectiveInverse = fmat4_identity();
	camera->view = fmat4_identity();
	camera->rotation = (float3){ 0.0f, 0.0f, 0.0f };
	camera->position = (float3){ 0.0f, 1.0f, 0.0f };
	camera->scale = (float3){ 1.0f, 1.0f, 1.0f };
	camera->viewPosition = (float3){ 0.0f, 0.0f, 0.0f };
	camera->frontPosition = (float3){ 1.0f, 0.0f, 0.0f };

	// calculate initial perspective
	if (camera->api == CREN_RENDERER_API_VULKAN_1_1 || camera->api == CREN_RENDERER_API_VULKAN_1_2 || camera->api == CREN_RENDERER_API_VULKAN_1_3) {
		CREN_LOG(CREN_LOG_SEVERITY_INFO, "This math function needs verifing");
		camera->perspective = fmat4_perspective_vulkan(to_fradians(camera->fov), initialAspectRatio, camera->near, camera->far);
		camera->perspectiveInverse = fmat4_inverse(&camera->perspective);
	}
	else {
		CREN_LOG(CREN_LOG_SEVERITY_FATAL, "The camera system is only functional under vulkan right now");
	}

	 camera->shouldMove = camera->movingBackward = camera->movingForward = camera->movingLeft = camera->movingRight = false;

	// update initial view
	internal_camera_update_view_matrix(camera);

	return camera;
}

CREN_API void cren_camera_destroy(CRenCamera* camera)
{
	if (camera) free(camera);
}

CREN_API void cren_camera_update(CRenCamera* camera, float timestep)
{
	if (!camera->shouldMove) return;

	// calculate front vector
	float yaw = to_fradians(camera->rotation.xyz.y);
	float pitch = to_fradians(camera->rotation.xyz.x);
	camera->frontPosition.xyz.x = f_cos(yaw) * f_cos(pitch);
	camera->frontPosition.xyz.y = f_sin(pitch);
	camera->frontPosition.xyz.z = f_sin(yaw) * f_cos(pitch);
	camera->frontPosition = float3_normalize(&camera->frontPosition);

	// calculate movement speed and right vector
	float moveSpeed = timestep * camera->movementSpeed;
	if (camera->modifierPressed) {
		moveSpeed *= camera->modifierSpeed;
	}

	const float3 worldUp = { 0.0f, 1.0f, 0.0f };
	float3 right = float3_cross(&worldUp, &camera->frontPosition);
	right = float3_normalize(&right);

	// apply movement
	if (camera->movingForward) {
		float3 movement = float3_scale(&camera->frontPosition, moveSpeed);
		camera->position = float3_add(&camera->position, &movement);
	}
	if (camera->movingBackward) {
		float3 movement = float3_scale(&camera->frontPosition, moveSpeed);
		camera->position = float3_sub(&camera->position, &movement);
	}
	if (camera->movingLeft) {
		float3 movement = float3_scale(&right, moveSpeed);
		camera->position = float3_sub(&camera->position, &movement);
	}
	if (camera->movingRight) {
		float3 movement = float3_scale(&right, moveSpeed);
		camera->position = float3_add(&camera->position, &movement);
	}

	// update view matrix
	internal_camera_update_view_matrix(camera);

	camera->viewInverse = fmat4_inverse(&camera->view);
}

CREN_API void cren_camera_set_aspect_ratio(CRenCamera* camera, float aspect)
{
	if (camera->api == CREN_RENDERER_API_VULKAN_1_1 || camera->api == CREN_RENDERER_API_VULKAN_1_2 || camera->api == CREN_RENDERER_API_VULKAN_1_3) {
		camera->perspective = fmat4_perspective_vulkan(to_fradians(camera->fov), aspect, camera->near, camera->far);
		camera->perspectiveInverse = fmat4_inverse(&camera->perspective);
	}

	else {
		CREN_LOG(CREN_LOG_SEVERITY_FATAL, "The camera system is only functional under vulkan right now");
	}

	camera->aspectRatio = aspect;
}

CREN_API float cren_camera_get_aspect_ratio(CRenCamera* camera)
{
	if (!camera) return 1.0f;
	return camera->aspectRatio;
}

CREN_API float cren_camera_get_fov(CRenCamera* camera)
{
	if (!camera) return 1.0f;
	return camera->fov;
}

CREN_API void cren_camera_translate(CRenCamera* camera, float3 dir)
{
	camera->position = float3_add(&camera->position, &dir);
	internal_camera_update_view_matrix(camera);
}

CREN_API void cren_camera_rotate(CRenCamera* camera, float3 dir)
{
	// avoid scene flip
	if (camera->rotation.xyz.x >= 89.0f) camera->rotation.xyz.x = 89.0f;
	if (camera->rotation.xyz.x <= -89.0f) camera->rotation.xyz.x = -89.0f;

	// reset rotation on 360 degrees
	if (camera->rotation.xyz.x >= 360.0f) camera->rotation.xyz.x = 0.0f;
	if (camera->rotation.xyz.x <= -360.0f) camera->rotation.xyz.x = 0.0f;
	if (camera->rotation.xyz.y >= 360.0f) camera->rotation.xyz.y = 0.0f;
	if (camera->rotation.xyz.y <= -360.0f) camera->rotation.xyz.y = 0.0f;

	// apply rotation speed
	dir.xyz.x *= camera->rotationSpeed * 0.5f;
	dir.xyz.y *= camera->rotationSpeed * 0.5f;

	camera->rotation = float3_add(&camera->rotation, &dir);
	internal_camera_update_view_matrix(camera);
}

CREN_API fmat4 cren_camera_get_view(CRenCamera* camera)
{
	if (!camera) return fmat4_identity();
	return camera->view;
}

CREN_API fmat4 cren_camera_get_view_inverse(CRenCamera* camera)
{
	if (!camera) return fmat4_identity();
	return camera->viewInverse;
}

CREN_API fmat4 cren_camera_get_perspective(CRenCamera* camera)
{
	if (!camera) return fmat4_identity();
	return camera->perspective;
}

CREN_API fmat4 cren_camera_get_perspective_inverse(CRenCamera* camera)
{
	if (!camera) return fmat4_identity();
	return camera->perspectiveInverse;
}

CREN_API void cren_camera_set_lock(CRenCamera* camera, bool value)
{
	if (!camera) return;
	camera->shouldMove = value;
}

CREN_API bool cren_camera_get_lock(CRenCamera* camera)
{
	if (!camera) return false;
	return camera->shouldMove;
}

CREN_API void cren_camera_move(CRenCamera* camera, CRen_CameraDirection dir, bool moving)
{
	if (!camera) return;

	switch (dir)
	{
		case CREN_CAMERA_DIRECTION_FORWARD: { camera->movingForward = moving; break; }
		case CREN_CAMERA_DIRECTION_BACKWARD: { camera->movingBackward = moving; break; }
		case CREN_CAMERA_DIRECTION_LEFT: { camera->movingLeft = moving; break; }
		case CREN_CAMERA_DIRECTION_RIGHT: { camera->movingRight = moving; break; }
	}
}

CREN_API bool cren_camera_get_speed_modifier(CRenCamera* camera, float* value)
{
	if (!camera) return false;
	if(value) *(value) = camera->modifierSpeed;
	return camera->modifierPressed;
}

CREN_API void cren_camera_set_speed_modifier(CRenCamera* camera, bool status, float value)
{
	if (!camera) return;
	camera->modifierPressed = status;
	camera->modifierSpeed = value;
}

CREN_API float3 cren_camera_get_position(CRenCamera* camera)
{
	if (!camera) return (float3){ 0.0f, 0.0f, 0.0f };
	return camera->position;
}

CREN_API float3 cren_camera_get_front(CRenCamera* camera)
{
	if (!camera) return (float3) { 0.0f, 0.0f, 0.0f };
	return camera->frontPosition;
}
