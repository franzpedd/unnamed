#include "UI/Viewport.h"

namespace Cosmos
{
	Viewport::Viewport(Cosmos::Application* app)
		: Widget("Viewport", true), mApp(app), mGizmo(app)
	{
		CREN_LOG(CREN_LOG_SEVERITY_TODO, "Update camera aspect ration uppon resize event");
		CREN_LOG(CREN_LOG_SEVERITY_TODO, "Update gizmo on selected entity");
		CreateGridResources();
	}

	Viewport::~Viewport()
	{
		CRenVulkanBackend* renderer = (CRenVulkanBackend*)cren_get_vulkan_backend(mApp->GetRendererRef()->GetCRenContext());
		vkDeviceWaitIdle(renderer->device.device);
		crenvk_pipeline_destroy(renderer->device.device, mGrid.crenPipeline);

		vkDestroyDescriptorPool(renderer->device.device, mGrid.descriptorPool, NULL);
	}

	void Viewport::OnUpdate()
	{
		CRenContext* context = mApp->GetRendererRef()->GetCRenContext();
		CRenVulkanBackend* renderer = (CRenVulkanBackend*)cren_get_vulkan_backend(context);

		UIWidget::BeginContext("Viewport", nullptr, UIWidget::ContextFlags_NoScrollbar);

		// update viewport position
		float2 viewportPos = UIWidget::GetCurrentWindowPos();
		cren_set_viewport_pos(context, viewportPos);

		// draw viewport
		UIWidget::Image((uint64_t)renderer->viewportRenderphase->descriptorSet, UIWidget::GetContentRegionAvail());

		// update viewport size, boundaries and camera aspect ratio
		float2 viewportSize = UIWidget::GetCurrentWindowSize();
		cren_set_viewport_size(context, viewportSize);
		cren_camera_set_aspect_ratio(cren_get_camera(context), viewportSize.xy.x / viewportSize.xy.y);

		// widgets
		DrawMenu(viewportPos.xy.x + 5.0f, viewportPos.xy.y + 5.0f);
		DrawContextMenu();
		//mGizmo.OnUpdate(NULL); // modify this when entity-selection is enabled

		UIWidget::EndContext();

		// update the settings window
		DrawSettings();
	}

	void Viewport::OnRender(int stage)
	{
		CRenVulkanBackend* renderer = (CRenVulkanBackend*)cren_get_vulkan_backend(mApp->GetRendererRef()->GetCRenContext());
		
		// draw grid
		if (mGrid.visible && stage != CREN_RENDER_STAGE_PICKING) {
			unsigned int currentFrame = renderer->swapchain.currentFrame;
			VkDeviceSize offsets[] = { 0 };
			VkCommandBuffer cmdbuffer = renderer->viewportRenderphase->renderpass->commandBuffers[currentFrame];
		
			vkCmdBindPipeline(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGrid.crenPipeline->pipeline);
			vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGrid.crenPipeline->layout, 0, 1, &mGrid.descriptorSets[currentFrame], 0, nullptr);
			vkCmdDraw(cmdbuffer, 6, 1, 0, 0);
		}
	}

	void Viewport::OnResize(int width, int height)
	{
		// viewport is internally-resized
	}

	void Viewport::OnKeyPress(Cosmos::Input::Keycode keycode, Cosmos::Input::Keymod mod, bool held)
	{
		CRenCamera* camera = cren_get_camera(mApp->GetRendererRef()->GetCRenContext());
		
		if (keycode == Cosmos::Input::KEYCODE_Z) {
			if (cren_camera_can_move(camera)) {
				mApp->GetWindowRef()->ToogleCursor(false);
				mApp->GetGUIRef()->ToggleCursor(false);
				cren_camera_enable_move(camera, false);
				return;
			}
		
			mApp->GetWindowRef()->ToogleCursor(true);
			mApp->GetGUIRef()->ToggleCursor(true);
			cren_camera_enable_move(camera, true);
		}
	}

	void Viewport::OnButtonPress(Cosmos::Input::Buttoncode buttoncode, Cosmos::Input::Keymod mod)
	{
		//// check for entity selection
		//if (buttoncode == Cosmos::Input::Buttoncode::BUTTON_LEFT && mod == Cosmos::Input::Keymod::KEYMOD_NONE) {
		//	//CRenContext* context = mApp->GetRendererRef().GetContext();
		//	//
		//	//float2 cursorPos = mApp->GetWindowRef().GetCursorPosition(true);
		//	//
		//	//bool clickedInViewport = true;
		//	//clickedInViewport &= cursorPos.x >= context->viewportPos.x && cursorPos.x <= (context->viewportPos.x + context->viewportSize.x);
		//	//clickedInViewport &= cursorPos.y >= context->viewportPos.y && cursorPos.y <= (context->viewportPos.y + context->viewportSize.y);
		//	//
		//	//if (clickedInViewport) {
		//	//	//printf("Reading at pixel %f, %f \n", cursorPos.x, cursorPos.y);
		//	//	//unsigned long long id = cren_vulkan_picking_read_pixel(context, cursorPos.x, cursorPos.y);
		//	//	//COSMOS_LOG(LogSeverity::Info, "ID %lld", id);
		//	//}
		//	//
		//	//// check if cursor position is inside viewport
		//	//float2 cursorPos;
		//	//mApp->GetWindowRef().GetViewportCursorPosition(context->viewportPos.x, context->viewportPos.y, context->viewportSize.x, context->viewportSize.y, &cursorPos.x, &cursorPos.y);
		//	//
		//	//if (clickedInViewport) {
		//	//	COSMOS_LOG(LogSeverity::Info, "Reading pixel at: %f, %f", cursorPos.x, cursorPos.y);
		//	//	
		//	//	unsigned long long id = cren_vulkan_picking_read_pixel(context, cursorPos.x, cursorPos.y);
		//	//	COSMOS_LOG(LogSeverity::Info, "ID %lld", id);
		//	//	
		//	//	if (id == 0) return;
		//	//	
		//	//	Entity* entity = mApp->GetWorldRef().FindEntityByID(id);
		//	//	if (!entity) return;
		//	//	
		//	//	if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) mSelectedEntities.clear();
		//	//	mSelectedEntities.push_back(entity);
		//	//}
		//}
	}

	void Viewport::DrawMenu(float xpos, float ypos)
	{
		static const float4 activeCol = { 1.0f, 1.0f, 1.0f, 0.5f };
		static unsigned int selectedGizmos = 0;
		static const Gizmos::Mode modes[4] = { Gizmos::Mode::Undefined, Gizmos::Mode::Translate, Gizmos::Mode::Rotate, Gizmos::Mode::Scale };
		static const std::string texts[4] = { ICON_LC_MOUSE_POINTER_2, ICON_LC_MOVE_3D, ICON_LC_ROTATE_3D, ICON_LC_SCALE_3D };
		static const std::string tooltips[4] = { "Selection", "Translation", "Rotation", "Scale" };

		UIWidget::SetNextWindowPos({ xpos + 15.0f, ypos + 35.0f });
		
		UIWidget::BeginChildContext("##ViewportMenubar");
		
		for (unsigned short i = 0; i < 4; i++) {

			bool coloredButton = selectedGizmos == i;
			if (coloredButton) { UIWidget::PushStyleColor(UIWidget::StyleColor_Button, activeCol); }
		
			if (UIWidget::Button(texts[i].c_str())) {
				mGizmo.SetMode(modes[i]);
				selectedGizmos = i;
			}
		
			if (coloredButton) { UIWidget::PopStyleColor(); }
			if (UIWidget::IsItemHovered(UIWidget::HoveredFlags_AllowWhenDisabled)) { UIWidget::SetTooltip("%s", tooltips[i].c_str()); }
		
			UIWidget::SameLine();
			UIWidget::SetCursorPosX(UIWidget::GetCursorPosX() - 5.0f);
		}
		
		UIWidget::Separator(1.0f, true);
		UIWidget::SameLine();
		
		// grid
		UIWidget::PushItemWidth(50.0f);
		UIWidget::PushStyleVar(UIWidget::StyleVar_ScrollbarPadding, 2.0f);
		UIWidget::SetCursorPosX(UIWidget::GetCursorPosX() - 5.0f);
		
		bool selectedButton = false;
		float snapping = mGizmo.GetSnappingValue();
		if (UIWidget::SliderFloat("##Snapping", &snapping, 0.005f, 10.0f, "%.2f")) { mGizmo.SetSnappingValue(snapping); }
		
		UIWidget::PopStyleVar();
		UIWidget::PopItemWidth();
		
		if (UIWidget::IsItemHovered(UIWidget::HoveredFlags_AllowWhenDisabled)) { UIWidget::SetTooltip("Grid snapping value"); }
		
		UIWidget::SameLine();
		
		bool selectedSnapping = mGizmo.GetSnapping();
		selectedButton = selectedSnapping;
		if (selectedSnapping) {
			UIWidget::PushStyleColor(UIWidget::StyleColor_Button, activeCol);
		}
		
		UIWidget::SetCursorPosX(UIWidget::GetCursorPosX() - 5.0f);
		
		if (UIWidget::Button(ICON_LC_MAGNET)) { mGizmo.SetSnapping(!mGizmo.GetSnapping()); }
		
		if (selectedButton) { UIWidget::PopStyleColor(); }
		
		if (UIWidget::IsItemHovered(UIWidget::HoveredFlags_AllowWhenDisabled)) { UIWidget::SetTooltip("Enables/Disables snapping with the grid"); }
		
		UIWidget::SameLine();
		
		static bool selectedGrid = true;
		selectedButton = selectedGrid;
		if (selectedButton) { UIWidget::PushStyleColor(UIWidget::StyleColor_Button, activeCol); }
		
		UIWidget::SetCursorPosX(UIWidget::GetCursorPosX() - 5.0f);
		
		if (UIWidget::Button(ICON_LC_GRID_3X3)) {
			mGrid.visible = !mGrid.visible;
			selectedGrid = !selectedGrid;
		}
		
		if (selectedButton) { UIWidget::PopStyleColor(); }
		if (UIWidget::IsItemHovered(UIWidget::HoveredFlags_AllowWhenDisabled)) { UIWidget::SetTooltip("Enables/Disables grid on viewport"); }
		
		UIWidget::SameLine();
		UIWidget::SetCursorPosX(UIWidget::GetCursorPosX() - 5.0f);
		UIWidget::Separator(1.0f, true);
		
		// debug
		UIWidget::SameLine();
		UIWidget::SetCursorPosX(UIWidget::GetCursorPosX() - 5.0f);
		if (UIWidget::Button(ICON_LC_SETTINGS)) { mSettings.visible = !mSettings.visible; }
		
		UIWidget::EndChildContext();
	}

	void Viewport::DrawContextMenu()
	{
		//if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight)) {
		//	ImGui::SeparatorText(ICON_LC_BRUSH " Context Menu");
		//
		//	if (ImGui::MenuItem(ICON_LC_PLUS " Add Entity")) {
		//		mApp->GetWorldRef().CreateEntity();
		//	}
		//
		//	ImGui::EndPopup();
		//}
	}

	void Viewport::DrawSettings()
	{
		/*
		if (!mSettings.visible) return;
		static Settings::MenuOption selected = Settings::MenuOption::General;
		
		UIWidget::BeginContext("Settings", &mSettings.visible);

		UIWidget::BeginChildContext("LeftMenu", float2{ UIWidget::GetContentRegionAvail().xy.x * 0.30f, 0 }, UIWidget::ChildFlags_Borders);
		{
			UIWidget::SeparatorText(ICON_FA_BARS " Settings Menu");
			if (UIWidget::Selectable("General Info", selected == Settings::MenuOption::General)) selected = Settings::MenuOption::General;

			UIWidget::SeparatorText(ICON_LC_BUG " Debug Info");
			if (UIWidget::Selectable("Entity List", selected == Settings::MenuOption::General)) selected = Settings::MenuOption::EntityList;
		}
		UIWidget::EndContext();
		
		UIWidget::SameLine();

		UIWidget::BeginChildContext("RightContent", float2{ 0.0f, 0.0f }, UIWidget::ChildFlags_Borders, UIWidget::ContextFlags_HorizontalScrollbar);
		{
			switch (selected)
			{
				case Settings::MenuOption::General: { DrawGeneralInfo(); break; }
				case Settings::MenuOption::EntityList: { DrawEntityList(); break; }
			}
		}
		UIWidget::EndChildContext();
		
		UIWidget::EndContext();
		*/
	}

	void Viewport::CreateGridResources()
	{
		CRenContext* context = mApp->GetRendererRef()->GetCRenContext();
		CRenVulkanBackend* renderer = (CRenVulkanBackend*)cren_get_vulkan_backend(context);
		
		// create grid pipeline
		char vert[CREN_PATH_MAX_SIZE], frag[CREN_PATH_MAX_SIZE];
		cren_get_path("shaders/compiled/grid.vert.spv", mApp->GetAssetsDir(), 0, vert, sizeof(vert));
		cren_get_path("shaders/compiled/grid.frag.spv", mApp->GetAssetsDir(), 0, frag, sizeof(frag));
		
		vkPipelineCreateInfo pipeCI = { 0 };
		pipeCI.renderpass = renderer->viewportRenderphase->renderpass;
		pipeCI.vertexShader = crenvk_pipeline_create_shader(renderer->device.device, "Grid.vert", vert, SHADER_TYPE_VERTEX);
		pipeCI.fragmentShader = crenvk_pipeline_create_shader(renderer->device.device, "Grid.frag", frag, SHADER_TYPE_FRAGMENT);
		pipeCI.vertexComponentsCount = 0;
		pipeCI.passingVertexData = 0;
		pipeCI.bindingsCount = 1;
		pipeCI.bindings[0].binding = 0;
		pipeCI.bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pipeCI.bindings[0].descriptorCount = 1;
		pipeCI.bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pipeCI.bindings[0].pImmutableSamplers = NULL;
		
		mGrid.crenPipeline = (vkPipeline*)malloc(sizeof(vkPipeline));
		CREN_ASSERT(mGrid.crenPipeline != NULL, "Failed to allocate memory for grid pipeline");
		
		CREN_ASSERT(crenvk_pipeline_create(renderer->device.device, &pipeCI, mGrid.crenPipeline) == VK_SUCCESS, "Failed to create grid pipeline");
		CREN_ASSERT(crenvk_pipeline_build(renderer->device.device, mGrid.crenPipeline) == VK_SUCCESS, "Failed to build grid pipeline");

		// create descriptor pool and descriptor sets
		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
		
		VkDescriptorPoolCreateInfo descPoolCI = {};
		descPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descPoolCI.poolSizeCount = 1;
		descPoolCI.pPoolSizes = &poolSize;
		descPoolCI.maxSets = CREN_CONCURRENTLY_RENDERED_FRAMES;
		CREN_ASSERT(vkCreateDescriptorPool(renderer->device.device, &descPoolCI, nullptr, &mGrid.descriptorPool) == VK_SUCCESS, "Failed to create descriptor pool");
		
		std::vector<VkDescriptorSetLayout> layouts(CREN_CONCURRENTLY_RENDERED_FRAMES, mGrid.crenPipeline->descriptorSetLayout);
		
		VkDescriptorSetAllocateInfo descSetAllocInfo = {};
		descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descSetAllocInfo.descriptorPool = mGrid.descriptorPool;
		descSetAllocInfo.descriptorSetCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
		descSetAllocInfo.pSetLayouts = layouts.data();
		CREN_ASSERT(vkAllocateDescriptorSets(renderer->device.device, &descSetAllocInfo, mGrid.descriptorSets) == VK_SUCCESS, "Failed to allocate descriptor sets");
		
		for (size_t i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
			vkBuffer* crenBuffer = (vkBuffer*)shashtable_lookup(renderer->buffersLib, "Camera");
			VkDescriptorBufferInfo bufferInfo = { 0 };
			bufferInfo.buffer = crenBuffer->buffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(vkBufferCamera);
		
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = mGrid.descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
		
			vkUpdateDescriptorSets(renderer->device.device, 1, &descriptorWrite, 0, nullptr);
		}
	}

	void Viewport::DrawGeneralInfo()
	{
		CREN_LOG(CREN_LOG_SEVERITY_TODO, "Address the url links when changing repo");

		UIWidget::SeparatorText(ICON_FA_INFO_CIRCLE " About the project: Cosmos");
		UIWidget::Text("Cosmos is a framework that allows the developer to create applications for desktop and mobile");
		UIWidget::Text("It is but a wish of mine to develop games with it");
		UIWidget::Text("It's hosted on");
		UIWidget::SameLine();
		UIWidget::URLText("github repository ##CosmosGit", "https://github.com/franzpedd/unnamed");
		
		UIWidget::SeparatorText(ICON_FA_INFO_CIRCLE " About the project: CRen");
		UIWidget::Text("CRen is currently developed by franzpedd (Edson Pirassol Junior) as a Computer Engineering Bachelor's degree final thesis");
		UIWidget::Text("It's hosted on");
		UIWidget::SameLine();
		UIWidget::URLText("github repository ##CrenGit", "https://github.com/franzpedd/unnamed/tree/main/cren");
		UIWidget::SameLine();
		UIWidget::Text(" and its a 3D renderer written with the Vulkan rendering API using minimal dependencies, ideally none at some point");
		
		UIWidget::SeparatorText(ICON_FA_INFO_CIRCLE " About the project: Engine");
		UIWidget::Text("A 3D Engine that uses, among other libraries the CRen library.");
		UIWidget::Text("Ideally it should be able to handle the gameplay loop, physics, sound, networking and glue everything together");
		UIWidget::Text("This project is not part of the final thesis, therefore it can use thirdparty dependencies, if they're permissive");
		
		UIWidget::SeparatorText(ICON_FA_INFO_CIRCLE " About the project: Editor");
		UIWidget::Text("Editor is an application that uses Cosmos and CRen combined in order to provide a Level Builder for games developed with the Engine");
		UIWidget::Text("It serves as the pourpuse of showing how to develop applications using the entire toolkit, wich is itself more than an Engine but a Framework entirely");
		
		UIWidget::SeparatorText(ICON_FA_INFO_CIRCLE " Considerations");
		UIWidget::Text("Since CRen is not platform dependent, it can be used to ship any game or application to all platforms that Vulkan runs");
		UIWidget::Text("Not only, the Engine uses SDL as the window manager, therefore it's possible to develop to mobile platforms");
		UIWidget::Text("A project for Android Studio is available on the");
		UIWidget::SameLine();
		UIWidget::URLText("github repository ##AndroidGit", "https://github.com/franzpedd/unnamed/tree/main/project_android");
	}

	void Viewport::DrawEntityList()
	{
		//ImGui::SeparatorText("All Entities present in the world are listed below:");
		//int idpush = 0;
		//static int selected = 0;
		//for (auto& item : mApp->GetWorldRef().GetEntityLibraryRef().GetAllRefs()) {
		//	ImGui::PushID(idpush++);
		//	ImGui::Text("%ld:", item.second->GetComponent<Cosmos::IDComponent>()->id.value);
		//	ImGui::SameLine();
		//	if (ImGui::Selectable(item.second->GetComponent<Cosmos::NameComponent>()->name.c_str(), selected == idpush)) {
		//		selected = idpush;
		//	}
		//
		//	ImGui::PopID();
		//}
	}
}