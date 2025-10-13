#include "World.h"

#include "Core/Application.h"
#include "Entity.h"

namespace Cosmos
{
	World::World(Unique<Renderer>& renderer)
		: mRenderer(renderer)
	{
	}

	bool World::AddEntity(Entity* entity)
	{
		if (!entity) return false;
		return mEntities.Insert(entity->GetIDValue(), entity);
	}

	bool World::CreateEntity(const char* name)
	{
		ID id = mIDGenerator.Create();
		
		if (mEntities.Contains(id.GetValue())) {
			mIDGenerator.Destroy(id);
			return false;
		}
		
		Entity* newEnt = new Entity(name, id);
		mEntities.Insert(id.GetValue(), newEnt);
		return true;
	}

	bool World::DestroyEntity(uint32_t idValue)
	{
		std::optional<Entity*> found = mEntities.Get(idValue);
		
		if (!found.has_value()) {
			return false;
		}
		
		Entity* entity = found.value();
		
		// destroy it's components here
		
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