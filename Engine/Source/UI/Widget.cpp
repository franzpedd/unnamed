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

namespace Cosmos::WidgetExtended
{
	void TextCentered(const char* fmt, ...)
	{
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = ImGui::CalcTextSize(fmt).x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		va_list args;
		va_start(args, fmt);
		ImGui::TextV(fmt, args);
		va_end(args);
	}

	void FloatControl(const char* label, float* value)
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

	void Float2Control(const char* label, float* x, float* y)
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

	void Float3Controller(const char* label, float* x, float* y, float* z)
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

	void VerticalSeparator(float thickness)
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, thickness);
	}

	void TextBackground(float4 bgCol, float4 txtCol, const char* label, const char* fmt, ...)
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

	bool Checkbox(const char* label, bool* v)
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

	bool CheckboxSliderEx(const char* label, bool* v)
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
