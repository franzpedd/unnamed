#include "UI/Gizmos.h"

#include "Core/Application.h"
//#include "Scene/Components.h"
//#include "Scene/Entity.h"

#include <vecmath/vecmath.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <imgui.h>

namespace Cosmos
{
	static bool epsilon_equal(float a, float b, float epsilon)
    {
		return fabsf(a - b) <= epsilon;
	}

	static bool Decompose(const fmat4* transform, float3* translation, float3* rotation, float3* scale)
    {
		fmat4 localMatrix = *transform;
		constexpr float epsilon = 1e-6f;

		// normalize the matrix
		if (epsilon_equal(localMatrix.data[3][3], 0.0f, epsilon)) return false;

		bool row0 = !epsilon_equal(localMatrix.data[0][3], 0.0f, epsilon);
		bool row1 = !epsilon_equal(localMatrix.data[1][3], 0.0f, epsilon);
		bool row2 = !epsilon_equal(localMatrix.data[2][3], 0.0f, epsilon);

		if (row0 || row1 || row2) {
			localMatrix.data[0][3] = localMatrix.data[1][3] = localMatrix.data[2][3] = 0.0f;
			localMatrix.data[3][3] = 1.0f;
		}

		// handle translation
		translation->xyz.x = localMatrix.data[3][0];
		translation->xyz.y = localMatrix.data[3][1];
		translation->xyz.z = localMatrix.data[3][2];
		localMatrix.data[3][0] = localMatrix.data[3][1] = localMatrix.data[3][2] = 0.0f;

		// handle scale
		float3 row[3];
		row[0] = { localMatrix.data[0][0], localMatrix.data[0][1], localMatrix.data[0][2] };
		row[1] = { localMatrix.data[1][0], localMatrix.data[1][1], localMatrix.data[1][2] };
		row[2] = { localMatrix.data[2][0], localMatrix.data[2][1], localMatrix.data[2][2] };

		// compute scale factor and normalize rows
		scale->xyz.x = float3_length(&row[0]);
		scale->xyz.y = float3_length(&row[1]);
		scale->xyz.z = float3_length(&row[2]);
		row[0] = float3_normalize(&row[0]);
		row[1] = float3_normalize(&row[1]);
		row[2] = float3_normalize(&row[2]);

		// extract rotation
		rotation->xyz.y = asinf(-row[0].xyz.z);

		if (cosf(rotation->xyz.y) != 0.0f) {
			rotation->xyz.x = atan2f(row[1].xyz.z, row[2].xyz.z);
			rotation->xyz.z = atan2f(row[0].xyz.y, row[0].xyz.x);
		}

		else {
			rotation->xyz.x = atan2f(-row[2].xyz.x, row[1].xyz.y);
			rotation->xyz.z = 0.0f;
		}

		return true;
	}

	Gizmos::Gizmos(Application* app)
		: mApp(app)
	{

	}

	void Gizmos::OnUpdate(Entity* entity)
	{
		//if (entity == NULL) return;
		//if (!entity->HasComponent<TransformComponent>()) return;
		//
		//ImGuizmo::SetOrthographic(false);
		//ImGuizmo::SetDrawlist();
		//
		//// viewport rect
		//float vpWidth = (float)ImGui::GetWindowWidth();
		//float vpHeight = (float)ImGui::GetWindowHeight();
		//ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, vpWidth, vpHeight);
		//
		//// camera
		//auto& camera = mRenderer.GetContext()->camera;
		//
		//mat4 view = camera.view;
		//mat4 proj = mat4_perspectiveRH(to_radians(camera.fov), vpWidth / vpHeight, camera.near, camera.far, 0);
		//proj.data[1][1] *= -1.0f;
		//
		//// entity
		//
		//auto* tc = entity->GetComponent<TransformComponent>();
		//mat4 transform = tc->GetTransform();
		//
		//// snapping
		//float snapValue = mMode == Gizmo::Mode::Rotate ? mSnappingValue + 5.0f : mSnappingValue;
		//float snapValues[3] = { snapValue, snapValue, snapValue };
		//
		//// gizmos drawing
		//ImGuizmo::Manipulate
		//(
		//	mat4_value_ptr(&view),
		//	mat4_value_ptr(&proj),
		//	(ImGuizmo::OPERATION)mMode,
		//	ImGuizmo::MODE::LOCAL,
		//	mat4_value_ptr(&transform),
		//	nullptr,
		//	mSnapping ? snapValues : nullptr
		//);
		//
		//if (ImGuizmo::IsUsing())
		//{
		//	float3 translation, rotation, scale;
		//	Decompose(&transform, &translation, &rotation, &scale);
		//
		//	float3 deltaRotation = float3_sub(rotation, tc->rotation);
		//	tc->translation = translation;
		//	tc->rotation = float3_add(tc->rotation, deltaRotation);
		//	tc->scale = scale;
		//}
	}
}