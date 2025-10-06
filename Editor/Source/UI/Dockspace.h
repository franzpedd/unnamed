#pragma once

#include <Cosmos.h>

namespace Cosmos
{
	class Dockspace : public Cosmos::Widget
	{
	public:

		// constructor
		Dockspace(Cosmos::Application* application);

		// destructor
		virtual ~Dockspace() = default;

	public:

		// user interface updating
		virtual void OnUpdate() override;

	private:

		Cosmos::Application* mApp = nullptr;

		UIWidget::ContextFlags mFlags;
	};
}