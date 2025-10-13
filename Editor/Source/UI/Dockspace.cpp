#include "UI/Dockspace.h"

namespace Cosmos
{
	Dockspace::Dockspace(Cosmos::Application* app)
		: Cosmos::Widget("Dockspace", true), mApp(app) 
	{
		mFlags = (UIWidget::ContextFlags)(
			UIWidget::ContextFlags_NoTitleBar
			| UIWidget::ContextFlags_NoResize
			| UIWidget::ContextFlags_NoMove
			| UIWidget::ContextFlags_NoCollapse
			| UIWidget::ContextFlags_NoBringToFrontOnFocus
			| UIWidget::ContextFlags_NoNavFocus
			);
	}

	void Dockspace::OnUpdate()
	{
		UIWidget::SetNextWindowPos(UIWidget::GetMainViewportPosition());
		UIWidget::SetNextWindowSize(UIWidget::GetMainViewportSize());

		UIWidget::BeginContext("##Dockspace", 0, mFlags);
		UIWidget::Dockspace(UIWidget::GetID("##MyDockspace"));
		UIWidget::EndContext();
	}
}