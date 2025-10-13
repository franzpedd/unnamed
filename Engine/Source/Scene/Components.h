#pragma once

#include "Util/Datafile.h"
#include <vecmath/vecmath.h>

// forward declarations
namespace Cosmos { class Entity; }

namespace Cosmos
{
	struct TransformComponent
	{
	public:

		/// @brief constructor
		TransformComponent(float3 translation = { 0.0f, 0.0f, 0.0f }, float3 rotation = { 0.0f, 0.0f, 0.0f }, float3 scale = { 1.0f, 1.0f, 1.0f });

		/// @brief saves the component into a data file
		static void Save(Entity* entity, Datafile& dataFile);

		/// @brief loads the component into the entity from data file
		static void Load(Entity* entity, Datafile& dataFile);

	public:

		// returns the transformation matrix
		fmat4 GetTransform();

	public:

		float3 translation;
		float3 rotation;
		float3 scale;
	};

	struct EditorComponent
	{
	public:

		/// @brief constrctor
		EditorComponent();

		/// @brief saves the component into a data file
		static void Save(Entity* entity, Datafile& dataFile);

		/// @brief loads the component into the entity from data file
		static void Load(Entity* entity, Datafile& dataFile);

	public:

	};
}