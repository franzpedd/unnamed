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
		cren_camera_set_aspect_ratio(cren_get_main_camera(context), viewportSize.xy.x / viewportSize.xy.y);

		// widgets
		DrawHorizontalMenu(viewportPos.xy.x + 5.0f, viewportPos.xy.y + 5.0f);
		//mGizmo.OnUpdate(NULL); // modify this when entity-selection is enabled

		DrawStatistics();
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
		CRenCamera* camera = cren_get_main_camera(mApp->GetRendererRef()->GetCRenContext());

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
		if (buttoncode == Input::BUTTON_LEFT && mod == Input::Keymod::KEYMOD_NONE)
		{
			// creating a new empty entity
			bool areaWithinViewport = true; // I should modify this to be inside the viewport
			static bool alreadyCreated = false; // just creating one entity for now, developing at it's peak
			if (mVerticalMenu.selectedOption == VerticalMenu::MenuOption::AddEntity && areaWithinViewport && !alreadyCreated) {

				// using 0,0,0 for now but I should address this
				mApp->GetRendererRef()->GetWorld()->CreateEntity("New Entity", {0.0f, 0.0f, 0.0f});
				alreadyCreated = true;
			}
		}
	}

	void Viewport::DrawHorizontalMenu(float xpos, float ypos)
	{
		static const float4 activeCol = { 1.0f, 1.0f, 1.0f, 0.5f };

		UIWidget::SetNextWindowPos({ xpos + 15.0f, ypos + 35.0f });

		UIWidget::BeginChildContext("##ViewportMenubar");

		///////////////////////////////////////////////////////////////////////////////////////////// horizontal menu

		// gizmo manipulation
		{
			for (uint8_t i = 0; i < HorizontalMenu::MenuOption::MenuOption_Max; i++) {
				bool coloredButton = mHorizontalMenu.selectedOption == i;
				if (coloredButton) UIWidget::PushStyleColor(UIWidget::StyleColor_Button, activeCol);

				if (UIWidget::Button(mHorizontalMenu.icon[i])) {
					if (mHorizontalMenu.selectedOption == i) {
						mHorizontalMenu.selectedOption = HorizontalMenu::MenuOption::Unselected;
					}

					else {
						mHorizontalMenu.selectedOption = (HorizontalMenu::MenuOption)i;
					}
				}

				if (coloredButton) { UIWidget::PopStyleColor(); }
				if (UIWidget::IsItemHovered(UIWidget::HoveredFlags_AllowWhenDisabled)) UIWidget::SetTooltip("%s", mHorizontalMenu.tooltip[i]);

				UIWidget::SameLine();
				UIWidget::SetCursorPosX(UIWidget::GetCursorPosX() - 5.0f);
			}
		}

		UIWidget::Separator(1.0f, true);
		UIWidget::SameLine();

		bool selectedButton = false;

		// grid snapping value
		{
			UIWidget::PushItemWidth(50.0f);
			UIWidget::PushStyleVar(UIWidget::StyleVar_ScrollbarPadding, 2.0f);
			UIWidget::SetCursorPosX(UIWidget::GetCursorPosX() - 5.0f);

			
			float snapping = mGizmo.GetSnappingValue();
			if (UIWidget::SliderFloat("##Snapping", &snapping, 0.005f, 10.0f, "%.2f")) { mGizmo.SetSnappingValue(snapping); }

			UIWidget::PopStyleVar();
			UIWidget::PopItemWidth();

			if (UIWidget::IsItemHovered(UIWidget::HoveredFlags_AllowWhenDisabled)) { UIWidget::SetTooltip("Grid snapping value"); }
		}

		UIWidget::SameLine();

		// grid snapping enable/disable
		{
			bool selectedSnapping = mGizmo.GetSnapping();
			selectedButton = selectedSnapping;
			if (selectedSnapping) {
				UIWidget::PushStyleColor(UIWidget::StyleColor_Button, activeCol);
			}

			UIWidget::SetCursorPosX(UIWidget::GetCursorPosX() - 5.0f);

			if (UIWidget::Button(ICON_LC_MAGNET)) { mGizmo.SetSnapping(!mGizmo.GetSnapping()); }

			if (selectedButton) { UIWidget::PopStyleColor(); }

			if (UIWidget::IsItemHovered(UIWidget::HoveredFlags_AllowWhenDisabled)) { UIWidget::SetTooltip("Enables/Disables snapping with the grid"); }

		}

		// grid visibility
		{
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
		}


		// debug
		{
			UIWidget::SameLine();
			UIWidget::SetCursorPosX(UIWidget::GetCursorPosX() - 5.0f);

			if (UIWidget::Button(ICON_LC_SETTINGS)) {
				mSettings.visible = !mSettings.visible;
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////// vertical menu

		// entity manipulation
		{
			for (uint8_t i = 0; i < VerticalMenu::MenuOption::MenuOption_Max; i++) {
				bool coloredButton = mVerticalMenu.selectedOption == i;
				if (coloredButton) UIWidget::PushStyleColor(UIWidget::StyleColor_Button, activeCol);

				if (UIWidget::Button(mVerticalMenu.icon[i])) {
					if (mVerticalMenu.selectedOption == i) {
						mVerticalMenu.selectedOption = VerticalMenu::MenuOption::Unselected;
					}

					else {
						mVerticalMenu.selectedOption = (VerticalMenu::MenuOption)i;
					}
				}

				if (coloredButton) { UIWidget::PopStyleColor(); }
				if (UIWidget::IsItemHovered(UIWidget::HoveredFlags_AllowWhenDisabled)) UIWidget::SetTooltip("%s", mVerticalMenu.tooltip[i]);
			}
		}

		UIWidget::EndChildContext();
	}

	void Viewport::DrawSettings()
	{
		if (!mSettings.visible) return;
		static Settings::MenuOption selected = Settings::MenuOption::EntityList;

		UIWidget::BeginContext("Settings", &mSettings.visible);

		UIWidget::BeginChildContext("LeftMenu", float2{ UIWidget::GetContentRegionAvail().xy.x * 0.30f, 0 }, UIWidget::ChildFlags_Borders);
		{
			UIWidget::SeparatorText(ICON_LC_BUG " Debug Info");
			if (UIWidget::Selectable("Entity List", selected == Settings::MenuOption::EntityList)) selected = Settings::MenuOption::EntityList;
		}
		UIWidget::EndChildContext();

		UIWidget::SameLine();

		UIWidget::BeginChildContext("RightContent", float2{ 0.0f, 0.0f }, UIWidget::ChildFlags_Borders, UIWidget::ContextFlags_HorizontalScrollbar);
		{
			switch (selected)
			{
			case Settings::MenuOption::EntityList: { DrawEntityList(); break; }
			}
		}
		UIWidget::EndChildContext();

		UIWidget::EndContext();
	}

	void Viewport::DrawEntityList()
	{
		UIWidget::SeparatorText("All entities presents in the current loaded world are listed below:");
		int localID = 0;
		static int selectedEntityIndex = 0;

		for (const auto& entity : mApp->GetRendererRef()->GetWorld()->GetEntityLibraryRef().GetAllRef()) {
			UIWidget::PushID(localID++);
			UIWidget::Text("%d", entity.second->GetID());
			UIWidget::SameLine();

			if (UIWidget::Selectable(entity.second->GetName(), selectedEntityIndex == localID)) {
				selectedEntityIndex = localID;
			}

			UIWidget::PopID();
		}
	}

	void Viewport::DrawStatistics()
	{
		// let's 'glue' the statis to the right-side
		float2 vpPos = cren_get_viewport_pos(mApp->GetRendererRef()->GetCRenContext());
		float2 vpSize = cren_get_viewport_size(mApp->GetRendererRef()->GetCRenContext());
		UIWidget::SetNextWindowPos({ vpPos.xy.x + (vpSize.xy.x - 200.0f), vpPos.xy.y + 40.0f });
		float2 mousePos = mApp->GetWindowRef()->GetCursorPos();
		float3 cameraPos = cren_camera_get_position(mApp->GetRendererRef()->GetMainCamera());
		
		UIWidget::BeginChildContext("##Statistics", { 230.0f, 125.0f }, UIWidget::ChildFlags_None);
		
		UIWidget::Text(ICON_LC_FLAME		 " [%.2f]", (float)mApp->GetAverageFPS());
		if (UIWidget::IsItemHovered(UIWidget::HoveredFlags_AllowWhenDisabled)) UIWidget::SetTooltip("Average frames / second");
		
		UIWidget::Text(ICON_LC_MOUSE_POINTER " [%.2f, %.2f]", mousePos.xy.x, mousePos.xy.y);
		if (UIWidget::IsItemHovered(UIWidget::HoveredFlags_AllowWhenDisabled)) UIWidget::SetTooltip("Cursor position, relative to monitor");
		
		UIWidget::Text(ICON_LC_CAMERA        " [%.2f, %.2f, %.2f]", cameraPos.xyz.x, cameraPos.xyz.y, cameraPos.xyz.z);
		if (UIWidget::IsItemHovered(UIWidget::HoveredFlags_AllowWhenDisabled)) UIWidget::SetTooltip("Camera 3D position in the world");
		
		UIWidget::Text(ICON_LC_RATIO		 " [%.2f, %.2f]", vpPos.xy.x, vpPos.xy.y);
		if (UIWidget::IsItemHovered(UIWidget::HoveredFlags_AllowWhenDisabled)) UIWidget::SetTooltip("Viewport position, relative to monitor");
		
		UIWidget::Text(ICON_LC_PROPORTIONS	 " [%.2f, %.2f]", vpSize.xy.x, vpSize.xy.y);
		if (UIWidget::IsItemHovered(UIWidget::HoveredFlags_AllowWhenDisabled)) UIWidget::SetTooltip("Viewport size");
		UIWidget::EndChildContext();
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
}