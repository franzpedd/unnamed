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

		/// @brief displays a menu within the viewport
		void DrawMenu(float xpos, float ypos);

		/// @brief displays a menu when a mouse right click occurs inside the viewport
		void DrawContextMenu();

		/// @brief displays the settings window, with various debug information about the editor
		void DrawSettings();

		/// @brief create and setup the grid resources
		void CreateGridResources();

		/// @brief draws useful information about the entity, project and etc
		void DrawGeneralInfo();

		/// @brief draws the entity list, usefull for debug and easy access to all entities on the scene
		void DrawEntityList();

		/// @brief draw's a statistics subwindow at top-right of the viewport
		void DrawStatistics();

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
				General, 
				EntityList,

				MenuOption_Max
			}; 
			bool menusVisible[MenuOption_Max];
			bool visible;
		} mSettings;

		struct Statistics 
		{
			bool visible;
		} mStatistics;

		std::vector<Cosmos::Entity*> mSelectedEntities;
	};
}