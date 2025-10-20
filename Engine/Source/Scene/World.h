#pragma once

#include "Core/Defines.h"

#include "Util/ID.h"
#include "Util/Library.h"
#include "Util/Memory.h"
#include <vecmath/vecmath.h>

// forward declaration
namespace Cosmos { class Application; }
namespace Cosmos { class Renderer; };
namespace Cosmos { class Entity; }

namespace Cosmos
{
	class COSMOS_API World
	{
	public:

		/// @brief constructor
		World(Application* app, Unique<Renderer>& renderer);

		/// @brief destructor
		~World();

		/// @brief returns a reference ot the world's entities
		inline Library<uint32_t, Entity*>& GetEntityLibraryRef() { return mEntities; }

	public:

		/// @brief attempts to add an existing entity into the world, returns false on failure
		bool AddEntity(Entity* entity);

		/// @brief attempts to create a new entity with an unique associated name, returns false on failure
		bool CreateEntity(const char* name = "Empty Entity", const float3& pos = {0.0f, 0.0f, 0.0f});

		/// @brief attempts to destroy an entity, returns false if entity with idValue was not found
		bool DestroyEntity(uint32_t idValue);

		/// @brief extracts the entity from world without freeing it's resources
		Entity* ExtractEntity(uint32_t idValue);

		/// @brief transfer an entity with given id to another world
		bool TransferEntity(uint32_t id, World& otherWorld);

		/// @brief finds the entity by it's id value, returns NULL on failure
		Entity* FindEntityByID(uint32_t idValue);

	public:

		/// @brief updates the world logic
		void OnUpdate(float timestep);

		/// @brief render the world drawables
		void OnRender(float timestep, int32_t stage);

		/// @brief deletes all entities on the world
		bool Destroy();

	public:

		Application* mApp = nullptr;
		Unique<Renderer>& mRenderer;
		IDGenerator mIDGenerator = {};
		Library<uint32_t, Entity*> mEntities = {};
	};
}