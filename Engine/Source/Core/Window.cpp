#include "Core/Window.h"
#include "Core/Application.h"

#include <cren.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <backends/imgui_impl_sdl3.h>

namespace Cosmos
{
	Window::Window(Application* app, const char* title, int width, int height, bool fullscreen, const char* rootPath)
		: mApp(app), mTitle(title), mWidth(width), mHeight(height)
	{
		if (SDL_Init(SDL_INIT_VIDEO) == false) {
			CREN_LOG(CREN_LOG_SEVERITY_FATAL, "SDL could not be initialized, more info: %s", SDL_GetError());
			return;
		}

		SDL_WindowFlags flags = fullscreen == true ? SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_FULLSCREEN
			: SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;

		mNativeWindow = SDL_CreateWindow(title, width, height, flags);

		if (!mNativeWindow) {
			CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Window could not be created, more info: %s", SDL_GetError());
			SDL_Quit();
			return;
		}

		CREN_LOG(CREN_LOG_SEVERITY_TODO, "Implement mobile touch events");
		CREN_LOG(CREN_LOG_SEVERITY_TODO, "Implement imgui event");

		// window logo
		char iconPath[128];
		cren_get_path("textures/icon.png", rootPath, 0, iconPath, sizeof(iconPath));

		int iconWidth, iconHeight, iconChannels;
		unsigned char* sIcon = cren_stbimage_load_from_file(iconPath, 4, &iconWidth, &iconHeight, &iconChannels);
		if (!sIcon) {
			CREN_LOG(CREN_LOG_SEVERITY_ERROR, "%s", cren_stbimage_get_error());
		}

		SDL_Surface* iconSurface = SDL_CreateSurfaceFrom(iconWidth, iconHeight, SDL_PIXELFORMAT_RGBA32, sIcon, iconWidth * 4);
		SDL_SetWindowIcon(mNativeWindow, iconSurface);
		SDL_DestroySurface(iconSurface);
		cren_stbimage_destroy(sIcon);
	}

	Window::~Window()
	{
		SDL_DestroyWindow((SDL_Window*)mNativeWindow);
		SDL_Quit();
	}

	void Window::OnUpdate()
	{
		SDL_Event event = { 0 };

		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);

			switch (event.type)
			{
				case SDL_EVENT_QUIT:
				{
					mShouldClose = true;
					break;
				}

				case SDL_EVENT_KEY_DOWN:
				{
					mApp->OnKeyPress((Input::Keycode)event.key.scancode, (Input::Keymod)event.key.mod, false);
					break;
				}

				case SDL_EVENT_KEY_UP:
				{
					mApp->OnKeyRelease((Input::Keycode)event.key.scancode);
					break;
				}

				case SDL_EVENT_MOUSE_BUTTON_DOWN:
				{
					mApp->OnButtonPress((Input::Buttoncode)event.button.button, Input::Keymod::KEYMOD_NONE);
					break;
				}

				case SDL_EVENT_MOUSE_BUTTON_UP:
				{
					mApp->OnButtonRelease((Input::Buttoncode)event.button.button);
					break;
				}

				case SDL_EVENT_MOUSE_WHEEL:
				{
					mApp->OnMouseScroll((double)event.wheel.x, -event.wheel.y);
					break;
				}

				case SDL_EVENT_MOUSE_MOTION:
				{
					mApp->OnMouseMove((double)event.motion.xrel, (double)event.motion.yrel);
					break;
				}

				case SDL_EVENT_WINDOW_RESIZED:
				{
					mWidth = event.window.data1;
					mHeight = event.window.data2;
					mApp->OnResize(event.window.data1, event.window.data2);
					break;
				}

				case SDL_EVENT_WINDOW_MINIMIZED:
				{
					mMinimized = true;
					mApp->OnMinimize();
					break;
				}

				case SDL_EVENT_WINDOW_RESTORED:
				{
					mMinimized = false;
					mApp->OnRestore(mWidth, mHeight);
					break;
				}

				case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
				{
					if (event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED) {
						mApp->OnDPIChange(SDL_GetWindowDisplayScale(mNativeWindow));
					}

					break;
				}

				case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
				{
					if (event.window.windowID == SDL_GetWindowID(mNativeWindow)) {
						float newRate = GetRefreshRate();
						mApp->SetTargetFrameTime(newRate, 1.0 / newRate);
					}

					break;
				}

				default:
				{
					break;
				}
			}
		}
	}

	void Window::ToogleCursor(bool hide)
	{
		if (hide) {
			SDL_HideCursor();
		}

		else {
			SDL_ShowCursor();
		}
	}

	bool Window::IsKeyDown(Input::Keycode key)
	{
		const bool* keyboardState = SDL_GetKeyboardState(NULL);
		return keyboardState[key] == 1;
	}

    const char *const* Window::GetRequiredExtensions(unsigned int* count)
    {
		return SDL_Vulkan_GetInstanceExtensions(count);
    }

    void Window::CreateSurface(void *instance, void *surface)
    {
		if (!SDL_Vulkan_CreateSurface(mNativeWindow, (VkInstance)instance, nullptr, (VkSurfaceKHR*)surface)) {
			CREN_LOG(CREN_LOG_SEVERITY_FATAL, "Failed to create Vulkan Surface trought SDL");
		}
    }

	float Window::GetRefreshRate()
	{
		float rate = 60.0f; // 60 is in case SDL doesn't define
		SDL_DisplayID display = SDL_GetDisplayForWindow(mNativeWindow);
		if (display == 0) {
			return rate;
		}

		const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(display);
		if (mode && mode->refresh_rate > 0) {
			rate = mode->refresh_rate;
		}

		return rate;
	}
}