#include "UI/Widget.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

namespace Cosmos
{
	Widget::Widget(const char* name, bool visible)
		: mName(name), mVisible(visible)
	{
	}
}

namespace Cosmos::UIWidget
{
	COSMOS_API ID GetID(const char* name)
	{
		return (ID)ImGui::GetID("##MyDockspace");
	}

	COSMOS_API bool BeginContext(const char* name, bool* open, ContextFlags flags)
	{
		return ImGui::Begin(name, open, flags);
	}

	COSMOS_API void EndContext()
	{
		ImGui::End();
	}

	COSMOS_API bool BeginChildContext(const char* name, const float2& size, ContextChildFlags childFlags, ContextFlags windowFlags)
	{
		return ImGui::BeginChild(name, { size.xy.x, size.xy.y }, childFlags, windowFlags);
	}

	COSMOS_API void EndChildContext()
	{
		ImGui::EndChild();
	}

	COSMOS_API uint32_t Dockspace(ID id, const float2& size)
	{
		return ImGui::DockSpace(id, { size.xy.x, size.xy.y });
	}

	COSMOS_API void SetNextWindowSize(const float2& size)
	{
		ImGui::SetNextWindowSize({ size.xy.x, size.xy.y });
	}

	COSMOS_API void SetNextWindowPos(const float2& pos, const float2& pivot)
	{
		ImGui::SetNextWindowPos({ pos.xy.x, pos.xy.y }, 0, { pivot.xy.x, pivot.xy.y });
	}

	COSMOS_API float2 GetCurrentWindowSize()
	{
		ImVec2 size = ImGui::GetWindowSize();
		return { size.x, size.y };
	}

	COSMOS_API float2 GetCurrentWindowPos()
	{
		ImVec2 pos = ImGui::GetWindowPos();
		return { pos.x, pos.y };
	}

	COSMOS_API float2 GetMainViewportSize()
	{
		ImVec2 size = ImGui::GetMainViewport()->Size;
		return { size.x, size.y };
	}

	COSMOS_API float2 GetMainViewportPosition()
	{
		ImVec2 pos = ImGui::GetMainViewport()->Pos;
		return { pos.x, pos.y };
	}

	COSMOS_API float2 GetContentRegionAvail()
	{
		ImVec2 size = ImGui::GetContentRegionAvail();
		return { size.x, size.y };
	}

	COSMOS_API bool IsItemHovered(HoveredFlags flags)
	{
		return ImGui::IsItemHovered((ImGuiHoveredFlags)flags);
	}

	COSMOS_API void SetCursorPos(float2 pos)
	{
		ImGui::SetCursorPos({ pos.xy.x, pos.xy.y });
	}

	COSMOS_API void SetCursorPosX(float value)
	{
		ImGui::SetCursorPosX(value);
	}

	COSMOS_API void SetCursorPosY(float value)
	{
		ImGui::SetCursorPosY(value);
	}

	COSMOS_API float2 GetCursorPos()
	{
		ImVec2 pos = ImGui::GetCursorPos();
		return { pos.x, pos.y };
	}

	COSMOS_API float GetCursorPosX()
	{
		return ImGui::GetCursorPosX();
	}

	COSMOS_API float GetCursorPosY()
	{
		return ImGui::GetCursorPosY();
	}

	COSMOS_API void SameLine(float startOffset, float spacing)
	{
		ImGui::SameLine(startOffset, spacing);
	}

	COSMOS_API void NewLine()
	{
		ImGui::NewLine();
	}

	COSMOS_API void Separator(float thickness, bool vertical)
	{
		ImGui::SeparatorEx(vertical == true ? ImGuiSeparatorFlags_Vertical : ImGuiSeparatorFlags_Horizontal, thickness);
	}

	COSMOS_API void PushStyleVar(const StyleVar var, float value)
	{
		ImGui::PushStyleVar((ImGuiStyleVar)var, value);
	}

	COSMOS_API void PushStyleVar(const StyleVar var, float2 value)
	{
		ImGui::PushStyleVar((ImGuiStyleVar)var, { value.xy.x, value.xy.y });
	}

	COSMOS_API void PopStyleVar(uint32_t count)
	{
		ImGui::PopStyleVar(count);
	}

	COSMOS_API void PushStyleColor(const StyleColor color, float4 value)
	{
		ImGui::PushStyleColor((ImGuiCol)color, { value.xyzw.x, value.xyzw.y, value.xyzw.z, value.xyzw.w });
	}

	COSMOS_API void PopStyleColor(uint32_t count)
	{
		ImGui::PopStyleColor(count);
	}

	COSMOS_API void PushItemWidth(float width)
	{
		ImGui::PushItemWidth(width);
	}

	COSMOS_API void PopItemWidth()
	{
		ImGui::PopItemWidth();
	}

	COSMOS_API void Text(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		ImGui::TextV(fmt, args);
		va_end(args);
	}

	COSMOS_API void SeparatorText(const char* label)
	{
		ImGui::SeparatorText(label);
	}

	COSMOS_API bool URLText(const char* label, const char* url)
	{
		return ImGui::TextLinkOpenURL(label, url);
	}

	COSMOS_API bool Button(const char* label, const float2 size)
	{
		return ImGui::Button(label, { size.xy.x, size.xy.y });
	}

	COSMOS_API bool Selectable(const char* label, bool selected, SelectableFlags flags, const float2& size)
	{
		return ImGui::Selectable(label, selected, flags, { size.xy.x, size.xy.y });
	}

	COSMOS_API void Image(uint64_t TexID, const float2& size, const float2& uv0, const float2& uv1)
	{
		ImGui::Image((ImTextureRef)TexID, { size.xy.x, size.xy.y }, { uv0.xy.x, uv0.xy.y }, { uv1.xy.x, uv1.xy.y });
	}

	COSMOS_API void SetTooltip(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		ImGui::SetTooltipV(fmt, args);
		va_end(args);
	}
	COSMOS_API bool SliderFloat(const char* label, float* v, float vmin, float vmax, const char* format, SliderFlags flags)
	{
		return ImGui::SliderFloat(label, v, vmin, vmax, format, (ImGuiSliderFlags)flags);
	}
}

namespace Cosmos::WidgetExtended
{
	COSMOS_API void TextCentered(const char* fmt, ...)
	{
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = ImGui::CalcTextSize(fmt).x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		va_list args;
		va_start(args, fmt);
		ImGui::TextV(fmt, args);
		va_end(args);
	}

	COSMOS_API void FloatControl(const char* label, float* value)
	{
		ImGui::PushID(label);

		constexpr ImVec4 colorX = ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f };
		constexpr ImVec4 colorY = ImVec4{ 0.25f, 0.7f, 0.2f, 1.0f };
		constexpr ImVec4 colorZ = ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f };

		// x
		{
			ImGui::PushStyleColor(ImGuiCol_Button, colorX);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorX);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorX);

			ImGui::SmallButton("X");
			ImGui::SameLine();
			ImGui::PushItemWidth(50);
			ImGui::DragFloat("##X", value, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::SameLine();
			ImGui::PopItemWidth();

			ImGui::PopStyleColor(3);
		}

		ImGui::NewLine();

		ImGui::PopID();
	}

	COSMOS_API void Float2Control(const char* label, float* x, float* y)
	{
		ImGui::PushID(label);

		constexpr ImVec4 colorX = ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f };
		constexpr ImVec4 colorY = ImVec4{ 0.25f, 0.7f, 0.2f, 1.0f };
		constexpr ImVec4 colorZ = ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f };

		// x
		{
			ImGui::PushStyleColor(ImGuiCol_Button, colorX);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorX);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorX);

			ImGui::SmallButton("X");
			ImGui::SameLine();
			ImGui::PushItemWidth(50);
			ImGui::DragFloat("##X", x, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::SameLine();
			ImGui::PopItemWidth();

			ImGui::PopStyleColor(3);
		}

		// y
		{
			ImGui::PushStyleColor(ImGuiCol_Button, colorY);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorY);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorY);

			ImGui::SmallButton("Y");
			ImGui::SameLine();
			ImGui::PushItemWidth(50);
			ImGui::DragFloat("##Y", y, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::SameLine();
			ImGui::PopItemWidth();

			ImGui::PopStyleColor(3);
		}

		ImGui::NewLine();

		ImGui::PopID();
	}

	COSMOS_API void Float3Controller(const char* label, float* x, float* y, float* z)
	{
		ImGui::PushID(label);

		constexpr ImVec4 colorX = ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f };
		constexpr ImVec4 colorY = ImVec4{ 0.25f, 0.7f, 0.2f, 1.0f };
		constexpr ImVec4 colorZ = ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f };

		// x
		{

			ImGui::PushStyleColor(ImGuiCol_Button, colorX);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorX);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorX);

			ImGui::SmallButton("X");
			ImGui::SameLine();
			ImGui::PushItemWidth(50);
			ImGui::DragFloat("##X", x, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::SameLine();
			ImGui::PopItemWidth();

			ImGui::PopStyleColor(3);
		}

		// y
		{
			ImGui::PushStyleColor(ImGuiCol_Button, colorY);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorY);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorY);

			ImGui::SmallButton("Y");
			ImGui::SameLine();
			ImGui::PushItemWidth(50);
			ImGui::DragFloat("##Y", y, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::SameLine();
			ImGui::PopItemWidth();

			ImGui::PopStyleColor(3);
		}

		// z
		{
			ImGui::PushStyleColor(ImGuiCol_Button, colorZ);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorZ);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorZ);

			ImGui::SmallButton("Z");
			ImGui::SameLine();
			ImGui::PushItemWidth(50);
			ImGui::DragFloat("##Z", z, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::SameLine();
			ImGui::PopItemWidth();

			ImGui::PopStyleColor(3);
		}

		ImGui::NewLine();

		ImGui::PopID();
	}

	COSMOS_API void VerticalSeparator(float thickness)
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, thickness);
	}

	COSMOS_API void TextBackground(float4 bgCol, float4 txtCol, const char* label, const char* fmt, ...)
	{
		ImVec4 bg = ImVec4(bgCol.xyzw.x, bgCol.xyzw.y, bgCol.xyzw.z, bgCol.xyzw.w);
		ImVec4 txt = ImVec4(txtCol.xyzw.x, txtCol.xyzw.y, txtCol.xyzw.z, txtCol.xyzw.w);

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10.0f, 10.0f));
		if (ImGui::BeginTable(label, 1, ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit)) {
			ImGui::TableNextRow();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(bg));
			ImGui::TableSetColumnIndex(0);

			ImGui::TextColored(txt, "%s", fmt);

			ImGui::EndTable();
		}
		ImGui::PopStyleVar();
	}

	COSMOS_API bool Checkbox(const char* label, bool* v)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		if (window->SkipItems) {
			return false;
		}

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		ImGuiID id = window->GetID(label);
		ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

		const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2((float)(label_size.y + style.FramePadding.y * 0.5), (float)(label_size.y + style.FramePadding.y * 0.5)));
		ImGui::ItemSize(check_bb, style.FramePadding.y);

		ImRect total_bb = check_bb;
		if (label_size.x > 0) {
			ImGui::SameLine(0, style.ItemInnerSpacing.x);
		}

		const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y) - ImVec2(0, 2), window->DC.CursorPos + ImVec2(0, style.FramePadding.y) + label_size);
		if (label_size.x > 0) {
			ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
			total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
		}

		if (!ImGui::ItemAdd(total_bb, id)) {
			return false;
		}

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
		if (pressed) {
			*v = !(*v);
		}

		ImGui::RenderFrame(check_bb.Min, check_bb.Max, ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);
		if (*v) {
			const float check_sz = ImMin(check_bb.GetWidth(), check_bb.GetHeight());
			const float pad = ImMax(1.0f, (float)(int)(check_sz / 6.0f));
			const ImVec2 pts[] = {
				ImVec2 {check_bb.Min.x + pad, check_bb.Min.y + ((check_bb.Max.y - check_bb.Min.y) / 2)},
				ImVec2 {check_bb.Min.x + ((check_bb.Max.x - check_bb.Min.x) / 3), check_bb.Max.y - pad * 1.5f},
				ImVec2 {check_bb.Max.x - pad, check_bb.Min.y + pad}
			};

			window->DrawList->AddPolyline(pts, 3, ImGui::GetColorU32(ImGuiCol_CheckMark), false, 2.0f);
		};

		if (g.LogEnabled) {
			ImVec2 text_pos = text_bb.GetTL();  // Store the temporary in a variable first
			ImGui::LogRenderedText(&text_pos, *v ? "[X]" : "[]");
		}
		if (label_size.x > 0.0f) {
			ImGui::RenderText(text_bb.GetTL(), label);
		}

		return pressed;
	}

	COSMOS_API bool CheckboxSliderEx(const char* label, bool* v)
	{
		ImGui::Spacing();

		ImGuiWindow* window = ImGui::GetCurrentWindow();

		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = ImGuiStyle();
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
		const ImVec2 pading = ImVec2(2, 2);
		const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.y + style.FramePadding.x * 6, label_size.y + style.FramePadding.y / 2));

		ImGui::ItemSize(check_bb, style.FramePadding.y);

		ImRect total_bb = check_bb;
		if (label_size.x > 0) {
			ImGui::SameLine(0, style.ItemInnerSpacing.x);
		}

		const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y), window->DC.CursorPos + ImVec2(0, style.FramePadding.y) + label_size);

		if (label_size.x > 0) {
			ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
			total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
		}

		if (!ImGui::ItemAdd(total_bb, id)) return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);

		if (pressed) {
			*v = !(*v);
		}

		const ImVec4 enabled = ImVec4(1.00f, 1.00f, 1.00f, 1.0f);
		const ImVec4 disabled = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
		const ImVec4 enabledBg = ImVec4(0.70f, 0.70f, 0.70f, 1.0f);

		const float check_sz = ImMin(check_bb.GetWidth(), check_bb.GetHeight());
		const float check_sz2 = check_sz / 2;
		const float pad = ImMax(1.0f, (float)(int)(check_sz / 4.f));
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 6, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 5, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 4, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 3, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 2, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 1, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 1, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 2, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 3, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 4, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 5, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 6, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);

		if (*v)
		{
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 6, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 5, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 4, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 3, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 2, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 1, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 1, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 3, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 4, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 5, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 6, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 6, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabled), 12);
		}

		else {
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 6, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabled), 12);
		}

		if (label_size.x > 0.0f) {
			ImGui::RenderText(text_bb.GetTL(), label);
		}

		ImGui::Spacing();

		return pressed;
	}
}
