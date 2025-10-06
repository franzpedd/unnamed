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
		virtual ~Editor();

	private:
		Dockspace* mDockspace = nullptr;
		Viewport* mViewport = nullptr;
	};
}