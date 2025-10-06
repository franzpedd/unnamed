#pragma once

#include "Core/Defines.h"
#include "Core/Input.h"
#include "WidgetTypes.h"
#include <vecmath/vecmath.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>

namespace Cosmos
{
	class COSMOS_API Widget
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

namespace Cosmos::UIWidget
{
	/// @brief returns the id for a given window name
	COSMOS_API ID GetID(const char* name);

	/// @brief window contexts
	COSMOS_API bool BeginContext(const char* name, bool* open = nullptr, ContextFlags flags = ContextFlags_None);
	COSMOS_API void EndContext();
	COSMOS_API bool BeginChildContext(const char* name, const float2& size = { 0.0f, 0.0f }, ContextChildFlags flags = ChildFlags_None, ContextFlags = ContextFlags_None);
	COSMOS_API void EndChildContext();
	COSMOS_API uint32_t Dockspace(ID id, const float2& size = { 0.0f, 0.0f });

	/// @brief context and window info
	COSMOS_API void SetNextWindowSize(const float2& size);
	COSMOS_API void SetNextWindowPos(const float2& pos, const float2& pivot = {0.0f, 0.0f});
	COSMOS_API float2 GetCurrentWindowSize();
	COSMOS_API float2 GetCurrentWindowPos();
	COSMOS_API float2 GetMainViewportSize();
	COSMOS_API float2 GetMainViewportPosition();
	COSMOS_API float2 GetContentRegionAvail();
	COSMOS_API bool IsItemHovered(HoveredFlags flags = HoveredFlags_None);

	/// @brief widget identation
	COSMOS_API void SetCursorPos(float2 pos);
	COSMOS_API void SetCursorPosX(float value);
	COSMOS_API void SetCursorPosY(float value);
	COSMOS_API float2 GetCursorPos();
	COSMOS_API float GetCursorPosX();
	COSMOS_API float GetCursorPosY();
	COSMOS_API void SameLine(float startOffset = 0.0f, float spacing = -1.0f);
	COSMOS_API void NewLine();
	COSMOS_API void Separator(float thickness = 1.0f, bool vertical = false);

	/// @brief style modification
	COSMOS_API void PushStyleVar(const StyleVar var, float value);
	COSMOS_API void PushStyleVar(const StyleVar var, float2 value);
	COSMOS_API void PopStyleVar(uint32_t count = 1);
	COSMOS_API void PushStyleColor(const StyleColor color, float4 value);
	COSMOS_API void PopStyleColor(uint32_t count = 1);
	COSMOS_API void PushItemWidth(float width);
	COSMOS_API void PopItemWidth();

	/// @brief various standart widgets
	COSMOS_API void Text(const char* fmt, ...);
	COSMOS_API void SeparatorText(const char* label);
	COSMOS_API bool URLText(const char* label, const char* url);
	COSMOS_API bool Button(const char* label, const float2 size = { 0.0f, 0.0f });
	COSMOS_API bool Selectable(const char* label, bool selected = false, SelectableFlags flags = SelectableFlags_None, const float2& size = { 0.0f, 0.0f });
	COSMOS_API void Image(uint64_t TexID, const float2& size = { 0.0f, 0.0f }, const float2& uv0 = { 0.0f, 0.0f }, const float2& uv1 = { 1.0f, 1.0f });
	COSMOS_API void SetTooltip(const char* fmt, ...);
	COSMOS_API bool SliderFloat(const char* label, float* v, float vmin, float vmax, const char* format = "%.3f", SliderFlags flags = SliderFlags_None);
}

namespace Cosmos::WidgetExtended
{
	/// @brief centered text in the window
	/// @param fmt va_arglist to format the text
	COSMOS_API void TextCentered(const char* fmt, ...);

	/// @brief little hack that transforms a table into a text with colored background
	/// @param bgCol the background color 0-255 for each value
	/// @param txtCol the text color 0-255 for each value
	/// @param label the text id/text
	/// @param fmt va_arglist to format the text
	COSMOS_API void TextBackground(float4 bgCol, float4 txtCol, const char* label, const char* fmt, ...);

	/// @brief custom checkbox
	/// @param label checkbox text
	/// @param v controls the checkbox is on/off
	/// @return true enabled, false on disabled
	COSMOS_API bool Checkbox(const char* label, bool* v);

	/// @brief custom checkbox slider
	/// @param label checkbox text
	/// @param v controls the checkbox is on/off
	/// @return true enabled, false on disabled
	COSMOS_API bool CheckboxSliderEx(const char* label, bool* v);

	/// @brief custom float controller
	/// @param label label control text
	/// @param x controller value
	COSMOS_API void FloatControl(const char* label, float* value);

	/// @brief custom 2d float controller
	/// @param label label control text
	/// @param x first controller value
	/// @param y second controller value
	COSMOS_API void Float2Control(const char* label, float* x, float* y);

	/// @brief custom 3d float controller
	/// @param label label control text
	/// @param x first controller value
	/// @param y second controller value
	/// @param z third controller value
	COSMOS_API void Float3Controller(const char* label, float* x, float* y, float* z);
}