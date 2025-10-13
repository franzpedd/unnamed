#pragma once

#include "Core/Defines.h"

#include "Util/ID.h"
#include "Util/Library.h"
#include "Util/Memory.h"

// forward declaration
namespace Cosmos { class Renderer; };
namespace Cosmos { class Entity; }

namespace Cosmos
{
	class COSMOS_API World
	{
	public:

		/// @brief constructor
		World(Unique<Renderer>& renderer);

		/// @brief destructor
		~World() = default;

		/// @brief returns a reference ot the world's entities
		inline Library<uint32_t, Entity*>& GetEntityLibraryRef() { return mEntities; }

	public:

		/// @brief attempts to add an existing entity into the world, returns false on failure
		bool AddEntity(Entity* entity);

		/// @brief attempts to create a new entity with an unique associated name, returns false on failure
		bool CreateEntity(const char* name = "Empty Entity");

		/// @brief attempts to destroy an entity, returns false if entity with idValue was not found
		bool DestroyEntity(uint32_t idValue);

		/// @brief extracts the entity from world without freeing it's resources
		Entity* ExtractEntity(uint32_t idValue);

		/// @brief transfer an entity with given id to another world
		bool TransferEntity(uint32_t id, World& otherWorld);

		/// @brief finds the entity by it's id value, returns NULL on failure
		Entity* FindEntityByID(uint32_t idValue);

	public:

		/// @brief deletes all entities on the world
		bool Destroy();

	public:

		Unique<Renderer>& mRenderer;
		IDGenerator mIDGenerator = {};
		Library<uint32_t, Entity*> mEntities = {};
	};
}