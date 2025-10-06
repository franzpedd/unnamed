#pragma once

#include "Core/Defines.h"

// forward declarations
namespace Cosmos { class Application; }
namespace Cosmos { class Entity; }

namespace Cosmos
{
    class COSMOS_API Gizmos
    {
	public:

		typedef enum Mode
		{
			Undefined = (0u << 0),

			TranslateX = (1u << 0),
			TranslateY = (1u << 1),
			TranslateZ = (1u << 2),
			RotateX = (1u << 3),
			RotateY = (1u << 4),
			RotateZ = (1u << 5),
			RotateScreen = (1u << 6),
			ScaleX = (1u << 7),
			ScaleY = (1u << 8),
			ScaleZ = (1u << 9),
			Bounds = (1u << 10),
			ScaleXU = (1u << 11),
			ScaleYU = (1u << 12),
			ScaleZU = (1u << 13),

			Translate = TranslateX | TranslateY | TranslateZ,
			Rotate = RotateX | RotateY | RotateZ | RotateScreen,
			Scale = ScaleX | ScaleY | ScaleZ,
			ScaleU = ScaleXU | ScaleYU | ScaleZU,
			Universal = Translate | Rotate | ScaleU

		} Mode;

    public:

        /// @brief constructor
        Gizmos(Application* app);

        /// @ brief destructor
        ~Gizmos() = default;

		// returns current gizmo's mode
		inline Mode GetMode() const { return mMode; }

		// sets a new gizmos operation mode
		inline void SetMode(Mode mode) { mMode = mode; }

		// returns if grid snapping is on/off
		inline bool GetSnapping() const { return mSnapping; }

		// sets the grid snapping
		inline void SetSnapping(bool value) { mSnapping = value; }

		// returns the snapping value
		inline float GetSnappingValue() { return mSnappingValue; }

		// sets a new snapping value
		inline void SetSnappingValue(float value) { mSnappingValue = value; }

	public:

		// updates the gizmos on objects
		void OnUpdate(Entity* entity);

	private:

		Application* mApp = nullptr;
		Mode mMode = Mode::Undefined;
		bool mSelectedButton = false;
		bool mSnapping = false;
		float mSnappingValue = 1.0f;
    };
}