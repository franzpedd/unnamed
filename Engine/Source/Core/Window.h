#pragma once

#include "Core/Defines.h"
#include "Core/Input.h"
#include <vecmath/vecmath.h>

// forward declarations
struct SDL_Window;
namespace Cosmos { class Application; }

namespace Cosmos
{
	class COSMOS_API Window
	{
	public:

		/// @brief constructs the window and set's up it's resources 
		Window(Application* app, const char* title, int width, int height, bool fullscreen, const char* rootPath);

		/// @brief shutsdown the window and it's resources
		~Window();

		/// @brief returns the window's underneath raw pointer, casted as void* so unnecessary includes can be avoided
		inline SDL_Window* GetAPIWindow() { return mNativeWindow; }

		/// @brief returns the window's width
		inline const int GetWidth() const { return mWidth; }

		/// @brief returns the window's height
		inline const int GetHeight() const { return mHeight; }

		/// @brief returns if a close request has been made and the window should be closed
		inline bool const ShouldClose() const { return mShouldClose; }

		/// @brief returns if a close request has been made and the window should be closed
		inline void Quit() { mShouldClose = true; }

		/// @brief returns if the window is currently minimized
		inline bool const IsMinimized() const { return mMinimized; }

	public:

		/// @brief updates the input/output window devices as well as the windows events, call this at the begining of the frame
		void OnUpdate();

		/// @brief hinds/shows the cursor on the window (locks it within the window if hidden)
		void ToogleCursor(bool hide);

		/// @brief returns if a key is currently pressed
		bool IsKeyDown(Input::Keycode key);

		/// @brief returns the required extensions by the windowing system
		const char* const* GetRequiredExtensions(unsigned int* count);

		/// @brief creates the vulkan surface, populates it and returns it to the renderer
		void CreateSurface(void* instance, void* surface);

		/// @brief returns the internal clock timer
		unsigned long long GetTimer();

		/// @brief returns the internal clock timer frequency
		unsigned long long GetTimerFrequency();
		
	public:

		/// @brief returns the monitor window is currently in refresh-rate, usefull for frame cap and resource management
		float GetRefreshRate();

		/// @brief returns the window's size
		float2 GetWindowSize();

		/// @brief returns the cursor position
		float2 GetCursorPos();

	protected:

		Application* mApp;
		SDL_Window* mNativeWindow = nullptr; // SDL's window is underneath this void*
		const char* mTitle;
		int mWidth;
		int mHeight;
		bool mShouldClose = false;
		bool mMinimized = false;
		double mLastMousePosX = 0.0;
		double mLastMousePosY = 0.0;
	};
}