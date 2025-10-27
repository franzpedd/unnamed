#include "World.h"

#include "Core/Application.h"
#include "Entity.h"
#include "Components.h"

namespace Cosmos
{
	World::World(Application* app, Unique<Renderer>& renderer)
		: mApp(app), mRenderer(renderer)
	{
	}

	World::~World()
	{
		Destroy();
	}

	bool World::AddEntity(Entity* entity)
	{
		if (!entity) return false;
		if (mEntities.Contains(entity->GetID())) {
			CREN_LOG(CREN_LOG_SEVERITY_ERROR, "Attempting to add an entity to a World that already contains such id registered");
			return false;
		}

		return mEntities.Insert(entity->GetID(), entity);
	}

	bool World::CreateEntity(const char* name, const float3& pos)
	{
		uint32_t id = cren_create_id(mRenderer->GetCRenContext());
		if (id == 0) {
			CREN_LOG(CREN_LOG_SEVERITY_ERROR, "The renderer could not create a unique ID");
			return false;
		}

		if (mEntities.Contains(id)) {
			CREN_LOG(CREN_LOG_SEVERITY_ERROR, "This world already has an entity with such ID");
			return false;
		}
		
		Entity* newEnt = new Entity(name, id);

		// add components
		newEnt->AddComponent<TransformComponent>();
		newEnt->GetComponent<TransformComponent>()->translation = pos;
		newEnt->GetComponent<TransformComponent>()->rotation = { 0.0f, 0.0f, 0.0f };
		newEnt->GetComponent<TransformComponent>()->scale = { 0.25f, 0.25f, 0.25f };

		newEnt->AddComponent<EditorComponent>();
		newEnt->GetComponent<EditorComponent>()->quad = cren_quad_create(mRenderer->GetCRenContext(), mApp->GetAssetPath("textures/entity.png").c_str(), id);

		mEntities.Insert(id, newEnt);
		return true;
	}

	bool World::DestroyEntity(uint32_t idValue)
	{
		std::optional<Entity*> found = mEntities.Get(idValue);
		
		if (!found.has_value()) {
			return false;
		}
		
		Entity* entity = found.value();
		if (!entity) {
			CREN_LOG(CREN_LOG_SEVERITY_WARN, "Trying to delete an entity with invalid id: %d", idValue);
			return false;
		}
		
		// destroy components
		if(entity->HasComponent<EditorComponent>()) 
		{
			if (entity->GetComponent<EditorComponent>()->quad) {
				cren_quad_destroy(mRenderer->GetCRenContext(), entity->GetComponent<EditorComponent>()->quad);
			}
			entity->RemoveComponent<EditorComponent>();
		}

		if (entity->HasComponent<TransformComponent>()) {
			entity->RemoveComponent<TransformComponent>();
		}
		
		mEntities.Erase(idValue);
		delete entity;
		bool deletedIDRes = mIDGenerator.Destroy((ID)(idValue));
		
		return deletedIDRes;
	}

	Entity* World::ExtractEntity(uint32_t idValue)
	{
		std::optional<Entity*> found = mEntities.Get(idValue);
		
		if (!found.has_value()) {
			return nullptr;
		}
		
		Entity* entity = found.value();
		mEntities.Erase(idValue);
		mIDGenerator.Destroy(idValue);
		
		return entity;
	}

	bool World::TransferEntity(uint32_t id, World& otherWorld)
	{
		Entity* entity = ExtractEntity(id);
		if (entity) {
			otherWorld.AddEntity(entity);
			return true;
		}
		return false;
	}

	Entity* World::FindEntityByID(uint32_t idValue)
	{
		return mEntities.Get(idValue).value_or(nullptr);
	}

	void World::OnUpdate(float timestep)
	{
		// not doing anything for now
	}

	void World::OnRender(float timestep, int32_t stage)
	{
		CRenContext* context = mRenderer->GetCRenContext();

		for (auto& [id, entity] : mEntities) {
			
			// validation
			if (id == 0 || !entity) continue;

			// model matrix TODO: apply timestep?
			TransformComponent* transformComponent = entity->GetComponent<TransformComponent>();
			if (!transformComponent) continue;

			// editor component
			EditorComponent* editorComponent = entity->GetComponent<EditorComponent>();
			if (editorComponent) {
				if (editorComponent->visible) {
					CRenQuad* quad = editorComponent->quad;
					if (quad) {
						fmat4 modelMatrix = transformComponent->GetTransform();
						cren_quad_render(context, quad, (CRen_RenderStage)stage, modelMatrix);
					}
				}
			}
		}
	}

	bool World::Destroy()
	{
		std::vector<uint32_t> entityIDs;
		entityIDs.reserve(mEntities.Size());
		
		for (auto& [id, entity] : mEntities) {
			entityIDs.push_back(id);
		}
		
		for (uint32_t id : entityIDs) {
			DestroyEntity(id);
		}
		
		mEntities.Clear();
		mIDGenerator.Reset();

		return true;
	}
}