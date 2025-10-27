#include "Components.h"
#include "Entity.h"

namespace Cosmos
{
	TransformComponent::TransformComponent(float3 translation, float3 rotation, float3 scale)
		: translation(translation), rotation(rotation), scale(scale)
	{
	}

	void TransformComponent::Save(Entity* entity, Datafile& dataFile)
	{
		if (entity->HasComponent<TransformComponent>()) {
			std::string uuid = std::to_string(entity->GetID());
			auto* component = entity->GetComponent<TransformComponent>();
			auto& place = dataFile[uuid]["Transform"];

			place["Translation"]["X"].SetDouble(component->translation.xyz.x);
			place["Translation"]["Y"].SetDouble(component->translation.xyz.y);
			place["Translation"]["Z"].SetDouble(component->translation.xyz.z);

			place["Rotation"]["X"].SetDouble(component->rotation.xyz.x);
			place["Rotation"]["Y"].SetDouble(component->rotation.xyz.y);
			place["Rotation"]["Z"].SetDouble(component->rotation.xyz.z);

			place["Scale"]["X"].SetDouble(component->scale.xyz.x);
			place["Scale"]["Y"].SetDouble(component->scale.xyz.y);
			place["Scale"]["Z"].SetDouble(component->scale.xyz.z);
		}
	}

	void TransformComponent::Load(Entity* entity, Datafile& dataFile)
	{
		if (dataFile.Exists("Transform")) {
			entity->AddComponent<TransformComponent>();
			auto* component = entity->GetComponent<TransformComponent>();

			auto& dataT = dataFile["Transform"]["Translation"];
			component->translation = { (float)dataT["X"].GetDouble(), (float)dataT["Y"].GetDouble(), (float)dataT["Z"].GetDouble() };

			auto& dataR = dataFile["Transform"]["Rotation"];
			component->rotation = { (float)dataR["X"].GetDouble(), (float)dataR["Y"].GetDouble(), (float)dataR["Z"].GetDouble() };

			auto& dataS = dataFile["Transform"]["Scale"];
			component->scale = { (float)dataS["X"].GetDouble(), (float)dataS["Y"].GetDouble(), (float)dataS["Z"].GetDouble() };
		}
	}

	fmat4 TransformComponent::GetTransform()
	{
		float3 rotRad = { to_fradians(rotation.xyz.x), to_fradians(rotation.xyz.y), to_fradians(rotation.xyz.z) };
		fquat q = fquat_from_euler(&rotRad);
		fmat4 rotMat = fquat_to_fmat4_rowmajor(&q);

		fmat4 result = fmat4_identity();
		result.matrix.m00 = rotMat.matrix.m00 * scale.xyz.x;
		result.matrix.m01 = rotMat.matrix.m01 * scale.xyz.x;
		result.matrix.m02 = rotMat.matrix.m02 * scale.xyz.x;

		result.matrix.m10 = rotMat.matrix.m10 * scale.xyz.y;
		result.matrix.m11 = rotMat.matrix.m11 * scale.xyz.y;
		result.matrix.m12 = rotMat.matrix.m12 * scale.xyz.y;

		result.matrix.m20 = rotMat.matrix.m20 * scale.xyz.z;
		result.matrix.m21 = rotMat.matrix.m21 * scale.xyz.z;
		result.matrix.m22 = rotMat.matrix.m22 * scale.xyz.z;

		result.matrix.m30 = translation.xyz.x;
		result.matrix.m31 = translation.xyz.y;
		result.matrix.m32 = translation.xyz.z;

		return result;
	}

	EditorComponent::EditorComponent()
	{
	}

	void EditorComponent::Save(Entity* entity, Datafile& dataFile)
	{
	}

	void EditorComponent::Load(Entity* entity, Datafile& dataFile)
	{
	}
}