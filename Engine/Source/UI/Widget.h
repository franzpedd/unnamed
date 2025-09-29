#pragma once

#include "Core/Input.h"
#include <vecmath/vecmath.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>

namespace Cosmos
{
	class Widget
	{
	public:

		// constructor
		Widget(const char* name, bool visible = false);

		// destructor
		virtual ~Widget() = default;

		// returns it's name
		inline const char* GetName() { return mName; }

		// returns if the widget is visible/displaying
		inline bool GetVisibility() { return mVisible; }

		// sets the widget visibility
		inline void SetVisibility(bool value) { mVisible = value; }

	public:

		// user interface drawing
		inline virtual void OnUpdate() {};

		// renderer drawing
		inline virtual void OnRender(int stage) {};

	public:

		/// @brief this is called by the window, signaling it's iconification
		inline virtual void OnMinimize() {};

		/// @brief this is called by the window, signaling it's size is not iconified/minimized anymore
		inline virtual void OnRestore(int width, int height) {};

		/// @brief this is called by the window, signaling the application and it's components to be resized to a new window size
		inline virtual void OnResize(int width, int height) {};

		/// @brief this is called by the window, signaling a key was pressed with/without a modfier and being held for a while or not
		inline virtual void OnKeyPress(Input::Keycode keycode, Input::Keymod mod, bool held) {};

		/// @brief this is called by the window, signaling a previously pressed key was released
		inline virtual void OnKeyRelease(Input::Keycode keycode) {};

		/// @brief this is called by the window, signaling a button was pressed with/without mods
		inline virtual void OnButtonPress(Input::Buttoncode buttoncode, Input::Keymod mod) {};

		/// @brief this is called by the window, signaling a previously pressed button was released
		inline virtual void OnButtonRelease(Input::Buttoncode buttoncode) {};

		/// @brief this is called by the window, signaling the mouse scroll was 'scroll'
		inline virtual void OnMouseScroll(double xoffset, double yoffset) {};

		/// @brief this is called by the window, signaling the mouse was moved to a new location
		inline virtual void OnMouseMove(double xpos, double ypos) {};

	protected:

		const char* mName = nullptr;
		bool mVisible = false;
	};
}

namespace Cosmos::WidgetExtended
{
	/// @brief centered text in the window
	/// @param fmt va_arglist to format the text
	void TextCentered(const char* fmt, ...);

	/// @brief little hack that transforms a table into a text with colored background
	/// @param bgCol the background color 0-255 for each value
	/// @param txtCol the text color 0-255 for each value
	/// @param label the text id/text
	/// @param fmt va_arglist to format the text
	void TextBackground(float4 bgCol, float4 txtCol, const char* label, const char* fmt, ...);

	/// @brief custom checkbox
	/// @param label checkbox text
	/// @param v controls the checkbox is on/off
	/// @return true enabled, false on disabled
	bool Checkbox(const char* label, bool* v);

	/// @brief custom checkbox slider
	/// @param label checkbox text
	/// @param v controls the checkbox is on/off
	/// @return true enabled, false on disabled
	bool CheckboxSliderEx(const char* label, bool* v);

	/// @brief custom float controller
	/// @param label label control text
	/// @param x controller value
	void FloatControl(const char* label, float* value);

	/// @brief custom 2d float controller
	/// @param label label control text
	/// @param x first controller value
	/// @param y second controller value
	void Float2Control(const char* label, float* x, float* y);

	/// @brief custom 3d float controller
	/// @param label label control text
	/// @param x first controller value
	/// @param y second controller value
	/// @param z third controller value
	void Float3Controller(const char* label, float* x, float* y, float* z);

	/// @brief makes a vertical separator with given thickness
	/// @param thickness how 'thick' the separator is
	void VerticalSeparator(float thickness);
}

namespace Cosmos::WidgetUtils
{
	class CenteredControlWrapper {
	public:
		explicit CenteredControlWrapper(bool result) : result_(result) {}

		operator bool() const {
			return result_;
		}

	private:
		bool result_;
	};

	class ControlCenterer {
	public:
		ControlCenterer(ImVec2 windowSize) : windowSize_(windowSize) {}

		template<typename Func>
		CenteredControlWrapper operator()(Func control) const {
			
			ImVec2 originalPos = ImGui::GetCursorPos();

			// Draw offscreen to calculate size
			ImGui::SetCursorPos(ImVec2(-10000.0f, -10000.0f));

			ImGui::PushID(this);
			control();
			ImGui::PopID();

			ImVec2 controlSize = ImGui::GetItemRectSize();

			// Draw at centered position
			ImGui::SetCursorPos(ImVec2((windowSize_.x - controlSize.x) * 0.5f, originalPos.y));

			control();

			return CenteredControlWrapper(ImGui::IsItemClicked());
		}

	private:
		ImVec2 windowSize_;
	};

#define CENTERED_CONTROL(control) Cosmos::WidgetUtils::ControlCenterer { ImGui::GetWindowSize() }([&]() { control; } )

}