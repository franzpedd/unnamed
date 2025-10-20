#pragma once

#include <Cosmos.h>
#include <cren.h>

namespace Cosmos
{
	class Viewport : public Cosmos::Widget
	{
	public:

		// constructor
		Viewport(Cosmos::Application* app);

		// destructor
		virtual ~Viewport();

	public:

		/// @brief updates the ui logic
		virtual void OnUpdate() override;

		/// @brief right-place to draw/render objects related to the viewport
		virtual void OnRender(int stage) override;

		/// @brief the application was resized, must also resize the viewport
		virtual void OnResize(int width, int height) override;

		/// @brief a key was pressed, must address it
		virtual void OnKeyPress(Cosmos::Input::Keycode keycode, Cosmos::Input::Keymod mod, bool held) override;

		/// @brief this is called by the window, signaling a button was pressed with/without mods
		virtual void OnButtonPress(Cosmos::Input::Buttoncode buttoncode, Cosmos::Input::Keymod mod) override;

	private:

		/// @brief displays a horizontal menu inside the viewport
		void DrawHorizontalMenu(float xpos, float ypos);

		/// @brief displays the settings window, with various debug information about the editor
		void DrawSettings();

		/// @brief draws the entity list, usefull for debug and easy access to all entities on the scene
		void DrawEntityList();

		/// @brief draw's a statistics subwindow at top-right of the viewport
		void DrawStatistics();

		/// @brief create and setup the grid resources
		void CreateGridResources();

	private:

		Cosmos::Application* mApp = nullptr;
		Cosmos::Gizmos mGizmo;

		struct Grid
		{
			bool visible = true;
			vkShader vertexShader = {};
			vkShader fragmentShader = {};
			VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
			VkDescriptorSet descriptorSets[CREN_CONCURRENTLY_RENDERED_FRAMES] = { 0 };
			vkPipeline* crenPipeline = NULL;
		} mGrid;

		struct Settings
		{
			enum MenuOption // this is menus that exists on the viewport
			{ 
				EntityList,

				MenuOption_Max
			}; 
			bool menusVisible[MenuOption_Max] = { false };
			bool visible = false;
		} mSettings;

		struct HorizontalMenu
		{
			enum MenuOption
			{
				Unselected = -1,
				Gizmo_Selection,
				Gizmo_Translate,
				Gizmo_Rotate,
				Gizmo_Scale,

				MenuOption_Max
			};

			const char* icon[4] = { ICON_LC_MOUSE_POINTER_2, ICON_LC_MOVE_3D, ICON_LC_ROTATE_3D, ICON_LC_SCALE_3D };
			const char* tooltip[4] = { "Selection", "Translation", "Rotation", "Scale" };
			MenuOption selectedOption = Gizmo_Selection;
		} mHorizontalMenu;

		struct VerticalMenu
		{
			enum MenuOption {
				Unselected = -1,
				AddEntity,

				MenuOption_Max
			};

			const char* icon[1] = { ICON_LC_SQUARE };
			const char* tooltip[1] = { "Add Entity" };
			MenuOption selectedOption = Unselected;
		} mVerticalMenu;

		struct Statistics 
		{
			bool visible;
		} mStatistics;

		std::vector<Cosmos::Entity*> mSelectedEntities;
	};
}