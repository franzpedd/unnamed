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
			std::string uuid = std::to_string(entity->GetIDValue());
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
		fmat4 identity = fmat4_identity();
		float3 rotRad = { to_fradians(rotation.xyz.x), to_fradians(rotation.xyz.y), to_fradians(rotation.xyz.z) };
		fquat q = fquat_from_euler(&rotRad);

		fmat4 rmat = fquat_to_fmat4_colmajor(&q);
		fmat4 smat = fmat4_scale_colmajor(&rmat, &scale);
		fmat4 tmat = fmat4_translate_colmajor(&identity, &translation);

		fmat4 rtmat = fmat4_mul(&rmat, &tmat); // (rotate * translate)
		return fmat4_mul(&smat, &rtmat); // scale * (rotate * translate)
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