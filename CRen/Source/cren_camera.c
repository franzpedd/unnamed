#include "cren_camera.h"

#include "cren_error.h"
#include <vecmath/vecmath.h>

/// @brief updates the view projection matrix of a camera
/// @param camera pointer to the camera
static void internal_camera_update_view_matrix(CRenCamera* camera)
{
	// calculate target point
	float3 target = float3_add(&camera->position, &camera->frontPosition);
	const float3 worldUp = { 0.0f, 1.0f, 0.0f };

	// create the view matrix
	camera->view = fmat4_lookat_agnostic(&camera->position, &target, &worldUp);
	camera->viewPosition = camera->position;
}

CRenCamera cren_camera_create(CRen_CameraType type, float initialAspectRatio, CRen_Renderer api)
{
	CRenCamera camera = { 0 };
	camera.type = type;
	camera.api = api;
	camera.fov = 45.0f;
	camera.near = 0.1f;
	camera.far = 256.0f;
	camera.aspectRatio = initialAspectRatio;
	camera.movementSpeed = 1.0f;
	camera.rotationSpeed = 1.0f;
	camera.modifierSpeed = 2.5f;

	camera.perspective = fmat4_identity();
	camera.view = fmat4_identity();
	camera.rotation = (float3){ 0.0f, 0.0f, 0.0f };
	camera.position = (float3){ 0.0f, 1.0f, 0.0f };
	camera.scale = (float3){ 1.0f, 1.0f, 1.0f };
	camera.viewPosition = (float3){ 0.0f, 0.0f, 0.0f };
	camera.frontPosition = (float3){ 0.0f, 0.0f, -1.0f };

	// calculate initial perspective
	if (camera.api == CREN_RENDERER_API_VULKAN_1_1 || camera.api == CREN_RENDERER_API_VULKAN_1_2 || camera.api == CREN_RENDERER_API_VULKAN_1_3) {
		CREN_LOG(CRenLogSeverity_Info, "This math function needs verifing");
		camera.perspective = fmat4_perspective_vulkan(to_fradians(camera.fov), initialAspectRatio, camera.near, camera.far);
	}
	else {
		CREN_LOG(CRenLogSeverity_Fatal, "The camera system is only functional under vulkan right now");
	}

	// update initial view
	internal_camera_update_view_matrix(&camera);

	return camera;
}

void cren_camera_update(CRenCamera* camera, float timestep)
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
	float3 right = float3_cross(&camera->frontPosition, &worldUp);
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
}

void cren_camera_set_aspect_ratio(CRenCamera* camera, float aspect)
{
	if (camera->api == CREN_RENDERER_API_VULKAN_1_1 || camera->api == CREN_RENDERER_API_VULKAN_1_2 || camera->api == CREN_RENDERER_API_VULKAN_1_3) {
		CREN_LOG(CRenLogSeverity_Info, "This math function needs verifing");
		camera->perspective = fmat4_perspective_vulkan(to_fradians(camera->fov), aspect, camera->near, camera->far);
	}

	else {
		CREN_LOG(CRenLogSeverity_Fatal, "The camera system is only functional under vulkan right now");
	}

	camera->aspectRatio = aspect;
}

void cren_camera_translate(CRenCamera* camera, float3 dir)
{
	camera->position = float3_add(&camera->position, &dir);
	internal_camera_update_view_matrix(camera);
}

void cren_camera_rotate(CRenCamera* camera, float3 dir)
{
	camera->rotation = float3_add(&camera->rotation, &dir);
	internal_camera_update_view_matrix(camera);
}
