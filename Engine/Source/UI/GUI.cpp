#include "UI/GUI.h"
#include "UI/Icons.h"
#include "Core/Application.h"

#include <cren_error.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl3.cpp>
#include <imgui/backends/imgui_impl_vulkan.cpp>
#include <imguizmo/imguizmo.h>

// fonts
#include <font/lucide.c>
#include <font/awesome.c>
#include <font/robotomono_medium.c>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace Cosmos
{
	static int Internal_SDL3_CreateVulkanSurface(ImGuiViewport* viewport, ImU64 vk_instance, const void* vk_allocator, ImU64* out_vk_surface)
	{
		ImGui_ImplSDL3_ViewportData* vd = (ImGui_ImplSDL3_ViewportData*)viewport->PlatformUserData;
		(void)vk_allocator;

		CREN_LOG(CRenLogSeverity_Todo, "Use cren_surface_create instead of SDL's one");
		bool ret = SDL_Vulkan_CreateSurface(vd->Window, (VkInstance)vk_instance, NULL, (VkSurfaceKHR*)out_vk_surface);
		return ret ? 0 : 1; // ret ? VK_SUCCESS : VK_NOT_READY 
	}

	static void Internal_ImGui_ReturnVulkanError(VkResult err)
	{
		if(err != VK_SUCCESS) {
			CREN_LOG(CRenLogSeverity_Error, "ImGui internal error: Result %d", err);
		}
	}

	static ImFont* sIconFA = nullptr;
	static ImFont* sIconLC = nullptr;
	static ImFont* sRobotoMono = nullptr;

	GUI::GUI(Application* app)
		: mApp(app)
	{
		// initial config
		IMGUI_CHECKVERSION();
		mContext = ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		CRen_Platform platform = cren_detect_platform();

		if (platform == CREN_PLATFORM_ANDROID) {
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
			io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
		}

		if (io.BackendFlags | ImGuiBackendFlags_PlatformHasViewports && io.BackendFlags | ImGuiBackendFlags_RendererHasViewports) {
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		}

		io.IniFilename = "UI.ini";
		io.WantCaptureMouse = true;

		ImGui::StyleColorsDark();
		SetStyle();

		CRenContext* renderer = mApp->GetRendererRef()->GetCRenContext();

		// sdl and vulkan initialization
		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		platformIO.Platform_CreateVkSurface = Internal_SDL3_CreateVulkanSurface;

		ImGui::SetCurrentContext((ImGuiContext*)mContext);
		ImGui_ImplSDL3_InitForVulkan(mApp->GetWindowRef()->GetAPIWindow());
		
		CRenVulkanBackend* vkBackend = (CRenVulkanBackend*)cren_get_vulkan_backend(mApp->GetRendererRef()->GetCRenContext());

		ImGui_ImplVulkan_PipelineInfo mainPipe = { 0 };
		mainPipe.RenderPass = vkBackend->uiRenderphase->renderpass->renderPass;
		mainPipe.Subpass = 0;
		mainPipe.MSAASamples = vkBackend->uiRenderphase->renderpass->msaa;
		//mainPipe.PipelineRenderingCreateInfo; // not applicable
		mainPipe.SwapChainImageUsage = 0;

		ImGui_ImplVulkan_InitInfo appInfo = { 0 };
		appInfo.ApiVersion = crenvk_encodeversion(mApp->GetRendererRef()->GetAPI());
		appInfo.Instance = vkBackend->instance.instance;
		appInfo.PhysicalDevice = vkBackend->device.physicalDevice;
		appInfo.Device = vkBackend->device.device;
		appInfo.QueueFamily = vkBackend->device.graphicsQueueIndex;
		appInfo.Queue = vkBackend->device.graphicsQueue;
		appInfo.DescriptorPool = vkBackend->uiRenderphase->descPool;
		//appInfo.DescriptorPoolSize = 0; // optional
		appInfo.MinImageCount = vkBackend->swapchain.swapchainImageCount;
		appInfo.ImageCount = vkBackend->swapchain.swapchainImageCount;
		appInfo.PipelineCache = VK_NULL_HANDLE;
		appInfo.PipelineInfoMain = mainPipe;
		appInfo.PipelineInfoForViewports = mainPipe; // we're managing our own viewport implementation
		appInfo.UseDynamicRendering = false;
		appInfo.Allocator = NULL;
		appInfo.CheckVkResultFn = Internal_ImGui_ReturnVulkanError;
		appInfo.MinAllocationSize = 1024 * 1024;
		//appInfo.CustomShaderVertCreateInfo; // customize vertex shader
		//appInfo.CustomShaderFragCreateInfo; // customize fragment shader
		ImGui_ImplVulkan_Init(&appInfo);
		
		// fonts
		constexpr const ImWchar iconRanges1[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		constexpr const ImWchar iconRanges2[] = { ICON_MIN_LC, ICON_MAX_LC, 0 };
		float iconSize = 13.0f;
		float fontSize = 18.0f;

		if (platform == CREN_PLATFORM_ANDROID) {
			CREN_LOG(CRenLogSeverity_Todo, "[Android:Todo]: Figure out an automate way to resize things, maybe wait ImGui-texture branch?");
			iconSize = 13.0f * 3.0f;
			fontSize = 18.0f * 3.0f;
		}

		ImFontConfig iconCFG;
		iconCFG.MergeMode = true;
		iconCFG.GlyphMinAdvanceX = iconSize;
		iconCFG.PixelSnapH = true;

		sRobotoMono = io.Fonts->AddFontFromMemoryCompressedTTF(txt_robotomono_medium_compressed_data, txt_robotomono_medium_compressed_size, fontSize);
		sIconFA = io.Fonts->AddFontFromMemoryCompressedTTF(icon_awesome_compressed_data, icon_awesome_compressed_size, iconSize, &iconCFG, iconRanges1);
		sIconLC = io.Fonts->AddFontFromMemoryCompressedTTF(icon_lucide_compressed_data, icon_lucide_compressed_size, iconSize, &iconCFG, iconRanges2);
		io.Fonts->Build();
		
		CREN_LOG(CRenLogSeverity_Warn, "Maybe resolve this commented func");
		//ImGui_ImplVulkan_CreateFontsTexture();
	}

	GUI::~GUI()
	{
		CRenVulkanBackend* backend = (CRenVulkanBackend*)cren_get_vulkan_backend(mApp->GetRendererRef()->GetCRenContext());
		CRenContext* renderer = mApp->GetRendererRef()->GetCRenContext();
		//vkDeviceWaitIdle(backend->device.device);

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL3_Shutdown();

		for (auto& widget : mWidgets.GetElementsRef()) {
			delete widget;
		}

		mWidgets.GetElementsRef().clear();
		ImGui::DestroyContext((ImGuiContext*)mContext);
	}

	void GUI::OnUpdate()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnUpdate();
		}

		// end frame
		ImGui::Render();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void GUI::OnRender(int stage)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnRender(stage);
		}
	}

	void GUI::AddWidget(Widget* widget)
	{
		if (!widget) return;

		if (!FindWidgetByName(widget->GetName())) {
			mWidgets.Push(widget);
		}
	}

	Widget* GUI::FindWidgetByName(const char* name)
	{
		Widget* found = nullptr;

		for (auto& widget : mWidgets.GetElementsRef()) {
			if (widget->GetName() == name) {
				found = widget;
				break;
			}
		}

		return found;
	}

	void GUI::ToggleCursor(bool hide)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (hide) {
			io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
			return;
		}

		io.ConfigFlags ^= ImGuiConfigFlags_NoMouse;
	}

	void GUI::OnMinimize()
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnMinimize();
		}
	}

	void GUI::OnRestore(int width, int height)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnRestore(width, height);
		}
	}

	void GUI::OnResize(int width, int height)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnResize(width, height);
		}
	}

	void GUI::OnKeyPress(Input::Keycode keycode, Input::Keymod mod, bool held)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnKeyPress(keycode, mod, held);
		}
	}

	void GUI::OnKeyRelease(Input::Keycode keycode)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnKeyRelease(keycode);
		}
	}

	void GUI::OnButtonPress(Input::Buttoncode buttoncode, Input::Keymod mod)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnButtonPress(buttoncode, mod);
		}
	}

	void GUI::OnButtonRelease(Input::Buttoncode buttoncode)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnButtonRelease(buttoncode);
		}
	}

	void GUI::OnMouseScroll(double xoffset, double yoffset)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnMouseScroll(xoffset, yoffset);
		}
	}

	void GUI::OnMouseMove(double xpos, double ypos)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnMouseMove(xpos, ypos);
		}
	}

	void GUI::OnDPIChange(float scale)
	{
	}

	void GUI::SetMinImageCount(unsigned int count)
	{
		ImGui_ImplVulkan_SetMinImageCount(count);
	}

	void GUI::DrawRawData(void* commandbuffer)
	{
		ImDrawData* data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(data, (VkCommandBuffer)commandbuffer);
	}

	bool GUI::WantToCaptureMouse()
	{
		return ImGui::GetIO().WantCaptureMouse;
	}

	void GUI::SetStyle()
	{
		ImGuiStyle* style = &ImGui::GetStyle();
		style->WindowPadding = ImVec2(10.0f, 10.0f);
		style->FramePadding = ImVec2(6.0f, 4.0f);
		style->ItemSpacing = ImVec2(9.0f, 4.0f);
		style->ItemInnerSpacing = ImVec2(3.0f, 3.0f);
		style->TouchExtraPadding = ImVec2(0.0f, 0.0f);
		style->IndentSpacing = 5.0f;
		style->ScrollbarSize = 20.0f;
		style->GrabMinSize = 10.0f;
		style->WindowBorderSize = 0.0f;
		style->ChildBorderSize = 0.0f;
		style->PopupBorderSize = 0.0f;
		style->FrameBorderSize = 0.0f;
		style->WindowRounding = 4.0f;
		style->ChildRounding = 0.0f;
		style->FrameRounding = 3.0f;
		style->PopupRounding = 3.0f;
		style->ScrollbarRounding = 3.0f;
		style->GrabRounding = 3.0f;
		style->TabBorderSize = 1.0f;
		style->TabBarBorderSize = 1.0f;
		style->TabCloseButtonMinWidthSelected = -1.0f;
		style->TabCloseButtonMinWidthUnselected = 8.0f;
		style->TabRounding = 3.0f;
		style->CellPadding = ImVec2(3.0f, 3.0f);
		style->TableAngledHeadersAngle = 10.0f;
		style->TableAngledHeadersTextAlign = ImVec2(0.5f, 0.0f);
		style->WindowTitleAlign = ImVec2(0.0f, 0.5f);
		style->WindowBorderHoverPadding = 10.0f;
		style->WindowMenuButtonPosition = ImGuiDir_Left;
		style->ColorButtonPosition = ImGuiDir_Right;
		style->ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style->SelectableTextAlign = ImVec2(0.0f, 0.0f);
		style->SeparatorTextBorderSize = 4.0f;
		style->SeparatorTextAlign = ImVec2(0.0f, 0.5f);
		style->LogSliderDeadzone = 4.0f;
		style->ImageBorderSize = 0.0f;
		style->Alpha = 1.0f;

		ImVec4* colors = style->Colors;

		bool darkMode = true;
		const ImVec4 blank = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		const ImVec4 background = darkMode ? ImVec4(0.08f, 0.08f, 0.08f, 0.9f) : ImVec4(0.69f, 0.69f, 0.69f, 1.0f);
		const ImVec4 framing = darkMode ? ImVec4(0.19f, 0.19f, 0.19f, 0.33f) : ImVec4(0.80f, 0.80f, 0.80f, 0.33f);
		const ImVec4 text = darkMode ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

		// text
		{
			colors[ImGuiCol_Text] = text;
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.50f, 0.0f, 0.50f, 1.0f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.92f, 0.92f, 0.89f, 1.0f);
			colors[ImGuiCol_TextLink] = ImVec4(0.35f, 0.35f, 0.92f, 0.86f);
		}
		// windows
		{
			colors[ImGuiCol_WindowBg] = background;
			colors[ImGuiCol_ChildBg] = blank;
			colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
			colors[ImGuiCol_MenuBarBg] = blank;
		}
		// titles
		{
			colors[ImGuiCol_TitleBg] = background;
			colors[ImGuiCol_TitleBgActive] = background;
			colors[ImGuiCol_TitleBgCollapsed] = background;
		}
		// frames
		{
			const ImVec4 hovered = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
			const ImVec4 active = ImVec4(0.43f, 0.43f, 0.43f, 1.0f);

			colors[ImGuiCol_FrameBg] = framing;
			colors[ImGuiCol_FrameBgHovered] = hovered;
			colors[ImGuiCol_FrameBgActive] = active;
			colors[ImGuiCol_InputTextCursor] = hovered;
		}
		// decorations
		{
			const ImVec4 idle = ImVec4(1.0f, 1.0f, 1.0f, 0.27f);
			const ImVec4 hovered = ImVec4(0.25f, 0.25f, 0.25f, 0.95f);
			const ImVec4 active = ImVec4(1.0f, 1.0f, 1.0f, 0.50f);

			colors[ImGuiCol_Border] = blank;
			colors[ImGuiCol_BorderShadow] = blank;
			colors[ImGuiCol_ScrollbarBg] = blank;
			colors[ImGuiCol_ScrollbarGrab] = framing;
			colors[ImGuiCol_ScrollbarGrabHovered] = idle;
			colors[ImGuiCol_ScrollbarGrabActive] = idle;
			colors[ImGuiCol_Separator] = idle;
			colors[ImGuiCol_ResizeGrip] = idle;
			colors[ImGuiCol_ResizeGripHovered] = hovered;
			colors[ImGuiCol_ResizeGripActive] = active;
			colors[ImGuiCol_SliderGrab] = idle;
			colors[ImGuiCol_SliderGrabActive] = active;
			colors[ImGuiCol_CheckMark] = active;
		}
		// widgets
		{
			const ImVec4 idle = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
			const ImVec4 header = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
			const ImVec4 hovered = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
			const ImVec4 active = ImVec4(0.43f, 0.43f, 0.43f, 1.0f);

			colors[ImGuiCol_Button] = header;
			colors[ImGuiCol_ButtonHovered] = hovered;
			colors[ImGuiCol_ButtonActive] = active;
			colors[ImGuiCol_Header] = header;
			colors[ImGuiCol_HeaderActive] = framing;
			colors[ImGuiCol_HeaderHovered] = framing;
			colors[ImGuiCol_Tab] = header;
			colors[ImGuiCol_TabHovered] = hovered;
			colors[ImGuiCol_TabSelected] = active;
			colors[ImGuiCol_TabSelectedOverline] = active;
			colors[ImGuiCol_TabDimmed] = hovered;
			colors[ImGuiCol_TabDimmedSelected] = active;
			colors[ImGuiCol_TabDimmedSelectedOverline] = active;
			colors[ImGuiCol_DragDropTarget] = header;
			colors[ImGuiCol_NavCursor] = header;
			colors[ImGuiCol_NavWindowingHighlight] = header;
			colors[ImGuiCol_NavWindowingDimBg] = hovered;
			colors[ImGuiCol_ModalWindowDimBg] = active;
			colors[ImGuiCol_PlotLines] = text;
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.35f, 1.0f, 0.35f, 1.0f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.9f, 1.0f, 0.7f, 1.0f);
			colors[ImGuiCol_TableHeaderBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
			colors[ImGuiCol_TableBorderStrong] = ImVec4(0.33f, 0.33f, 0.33f, 1.0f);
			colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.23f, 1.0f);
			colors[ImGuiCol_TableRowBg] = blank;
			colors[ImGuiCol_TableRowBgAlt] = blank;
			colors[ImGuiCol_DockingPreview] = idle;
			colors[ImGuiCol_DockingEmptyBg] = hovered;
		}
	}
}