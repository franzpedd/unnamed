#pragma once

#include "Core/Defines.h"
#include "Core/Input.h"
#include "Core/Renderer.h"
#include "Core/Window.h"
#include "UI/GUI.h"
#include "Util/Memory.h"

namespace Cosmos
{
	struct COSMOS_API ApplicationCreateInfo
	{
		/// @brief this is the application name, used in the window as well as internally by the renderer
		const char* appName = nullptr;

		/// @brief tells the renderer we're going to create a customized viewport that is pottentially different than the one provided by cren (wich covers the entire window)
		bool customViewport = false;

		/// @brief tells the renderer we're going to need validations (not a good idea on an physical android device, only emulated)
		bool validations = true;

		/// @brief tells the window manager that the application will cover the entire window area
		bool fullscreen = false;

		/// @brief request vertical syncronization
		bool vsync = false;

		/// @brief tells the window manager it's width size, this can latter be changed
		int width = 1366;

		/// @brief tells the window manager it's height size, this can latter be changed
		int height = 768;

		/// @brief tells the engine and renderer where the assets directory is located
		const char* assetsPath = nullptr;

		/// @brief tells wich renderer to use
		CRen_RendererAPI renderer = CREN_RENDERER_API_VULKAN_1_1;

		/// @brief tells how many samples the MSAA algorithm should use
		CRen_MSAA msaa = CREN_MSAA_X4;
	};

	class COSMOS_API Application
	{
	public:

		/// @brief constructor
		Application(const ApplicationCreateInfo& ci);

		/// @brief destructor
		virtual ~Application() = default;

		/// @brief returns the assets path
		inline const char* GetAssetsDir() { return mAssetsPath; }

		/// @brief returns a reference to the window class
		inline Unique<Window>& GetWindowRef() { return mWindow; }

		/// @brief returns a refenrece to the renderer class
		inline Unique<Renderer>& GetRendererRef() { return mRenderer; }

		/// @brief returns a reference to the gui class
		inline Unique<GUI>& GetGUIRef() { return mGUI; }

		/// @brief sets the time between loop itarations, this comes from window refresh rate and changes when SDL_EVENT_WINDOW_DISPLAY_CHANGED happens
		inline void SetTargetFrameTime(double targetFPS, double frameTimer) { mTargetFPS = targetFPS; mTargetFrameTime = frameTimer; }

		/// @brief returns the average frames per second
		inline double GetAverageFPS() { return mAverageFPS; }

		/// @brief returns the target frames per second
		inline double GetTargetFPS() { return mTargetFPS; }

	public:

		/// @brief called for initializing main loop
		void Run();

		/// @brief forces the closing of the application
		void Quit();

	protected:

		/// @brief calls before all internal objects still exists, for handling outside engine object-destruction that depends on it
		virtual void Shutdown() = 0;

 	public:

		/// @brief this is called by the window, signaling it's iconification
		void OnMinimize();

		/// @brief this is called by the window, signaling it's size is not iconified/minimized anymore
		void OnRestore(int width, int height);

		/// @brief this is called by the window, signaling the application and it's components to be resized to a new window size
		void OnResize(int width, int height);

		/// @brief this is called by the window, signaling a key was pressed with/without a modfier and being held for a while or not
		void OnKeyPress(Input::Keycode keycode, Input::Keymod mod, bool held);

		/// @brief this is called by the window, signaling a previously pressed key was released
		void OnKeyRelease(Input::Keycode keycode);

		/// @brief this is called by the window, signaling a button was pressed with/without mods
		void OnButtonPress(Input::Buttoncode buttoncode, Input::Keymod mod);

		/// @brief this is called by the window, signaling a previously pressed button was released
		void OnButtonRelease(Input::Buttoncode buttoncode);

		/// @brief this is called by the window, signaling the mouse scroll was 'scroll'
		void OnMouseScroll(double xoffset, double yoffset);

		/// @brief this is called by the window, signaling the mouse was moved to a new location
		void OnMouseMove(double xpos, double ypos);

		/// @brief this is called by the window, the dots per inch has changed
		void OnDPIChange(float scale);

	private:

		ApplicationCreateInfo mApplicationCreateInfo;
		const char* mAssetsPath = nullptr;
		Unique<Window> mWindow;
		Unique<Renderer> mRenderer;
		Unique<GUI> mGUI;

		double mTimeStep = 0.0;
		double mAverageFPS = 0;
		double mTargetFPS = 0.0f;
		double mTargetFrameTime = 0.0f;
	};
}