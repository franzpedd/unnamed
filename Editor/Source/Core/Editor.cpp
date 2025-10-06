#include "Editor.h"
#include "UI/Dockspace.h"
#include "UI/Viewport.h"

namespace Cosmos
{
	Editor::Editor(const ApplicationCreateInfo& ci)
		: Application(ci)
	{
		mDockspace = new Dockspace(this);
		GetGUIRef()->AddWidget(mDockspace);

		mViewport = new Viewport(this);
		GetGUIRef()->AddWidget(mViewport);
	}

	Editor::~Editor()
	{
		delete mViewport;
		delete mDockspace;
	}
}