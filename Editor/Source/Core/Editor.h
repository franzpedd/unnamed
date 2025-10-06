#pragma once

#include <Cosmos.h>

// forward declarations
namespace Cosmos { class Dockspace; }
namespace Cosmos { class Viewport; }

namespace Cosmos
{
	class Editor : public Application
	{
	public:

		/// @brief constructor
		Editor(const ApplicationCreateInfo& ci);

		/// @brief destructor
		virtual ~Editor() = default;

	protected:

		/// @brief calls before all internal objects still exists, for handling outside engine object-destruction that depends on it
		virtual void Shutdown() override;

	private:
		Dockspace* mDockspace = nullptr;
		Viewport* mViewport = nullptr;
	};
}