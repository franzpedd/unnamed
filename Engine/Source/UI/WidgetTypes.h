#pragma once

#include "Core/Defines.h"

namespace Cosmos::UIWidget
{
	typedef uint32_t ID;									// unique id for windows

	enum ContextFlags : uint32_t
	{
		ContextFlags_None = 0,
		ContextFlags_NoTitleBar = 1 << 0,					// Disable title-bar
		ContextFlags_NoResize = 1 << 1,						// Disable user resizing with the lower-right grip
		ContextFlags_NoMove = 1 << 2,						// Disable user moving the window
		ContextFlags_NoScrollbar = 1 << 3,					// Disable scrollbars (window can still scroll with mouse or programmatically)
		ContextFlags_NoScrollWithMouse = 1 << 4,			// Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
		ContextFlags_NoCollapse = 1 << 5,					// Disable user collapsing window by double-clicking on it. Also referred to as Window Menu Button (e.g. within a docking node).
		ContextFlags_AlwaysAutoResize = 1 << 6,				// Resize every window to its content every frame
		ContextFlags_NoBackground = 1 << 7,					// Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
		ContextFlags_NoSavedSettings = 1 << 8,				// Never load/save settings in .ini file
		ContextFlags_NoMouseInputs = 1 << 9,				// Disable catching mouse, hovering test with pass through.
		ContextFlags_MenuBar = 1 << 10,						// Has a menu-bar
		ContextFlags_HorizontalScrollbar = 1 << 11,			// Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(float2(width,0.0f)); prior to calling Begin() to specify width. Read code in _demo in the "Horizontal Scrolling" section.
		ContextFlags_NoFocusOnAppearing = 1 << 12,			// Disable taking focus when transitioning from hidden to visible state
		ContextFlags_NoBringToFrontOnFocus = 1 << 13,		// Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)
		ContextFlags_AlwaysVerticalScrollbar = 1 << 14,		// Always show vertical scrollbar (even if ContentSize.y < Size.y)
		ContextFlags_AlwaysHorizontalScrollbar = 1 << 15,	// Always show horizontal scrollbar (even if ContentSize.x < Size.x)
		ContextFlags_NoNavInputs = 1 << 16,					// No keyboard/gamepad navigation within the window
		ContextFlags_NoNavFocus = 1 << 17,					// No focusing toward this window with keyboard/gamepad navigation (e.g. skipped by CTRL+TAB)
		ContextFlags_UnsavedDocument = 1 << 18,				// Display a dot next to the title. When used in a tab/docking context, tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
		ContextFlags_NoDocking = 1 << 19,					// Disable docking of this window
		ContextFlags_NoNav = ContextFlags_NoNavInputs | ContextFlags_NoNavFocus,
		ContextFlags_NoDecoration = ContextFlags_NoTitleBar | ContextFlags_NoResize | ContextFlags_NoScrollbar | ContextFlags_NoCollapse,
		ContextFlags_NoInputs = ContextFlags_NoMouseInputs | ContextFlags_NoNavInputs | ContextFlags_NoNavFocus
	};

	enum ContextChildFlags : uint32_t
	{
		ChildFlags_None = 0,
		ChildFlags_Borders = 1 << 0,						// Show an outer border and enable WindowPadding. (IMPORTANT: this is always == 1 == true for legacy reason)
		ChildFlags_AlwaysUseWindowPadding = 1 << 1,			// Pad with style.WindowPadding even if no border are drawn (no padding by default for non-bordered child windows because it makes more sense)
		ChildFlags_ResizeX = 1 << 2,						// Allow resize from right border (layout direction). Enable .ini saving (unless ContextFlags_NoSavedSettings passed to window flags)
		ChildFlags_ResizeY = 1 << 3,						// Allow resize from bottom border (layout direction). "
		ChildFlags_AutoResizeX = 1 << 4,					// Enable auto-resizing width. Read "IMPORTANT: Size measurement" details above.
		ChildFlags_AutoResizeY = 1 << 5,					// Enable auto-resizing height. Read "IMPORTANT: Size measurement" details above.
		ChildFlags_AlwaysAutoResize = 1 << 6,				// Combined with AutoResizeX/AutoResizeY. Always measure size even when child is hidden, always return true, always disable clipping optimization! NOT RECOMMENDED.
		ChildFlags_FrameStyle = 1 << 7,						// Style the child window like a framed item: use FrameBg, FrameRounding, FrameBorderSize, FramePadding instead of ChildBg, ChildRounding, ChildBorderSize, WindowPadding.
		ChildFlags_NavFlattened = 1 << 8					// [BETA] Share focus scope, allow keyboard/gamepad navigation to cross over parent border to this child or between sibling child windows.
	};

	enum SelectableFlags : uint32_t
	{
		SelectableFlags_None = 0,
		SelectableFlags_NoAutoClosePopups = 1 << 0,			// Clicking this doesn't close parent popup window (overrides ItemFlags_AutoClosePopups)
		SelectableFlags_SpanAllColumns = 1 << 1,			// Frame will span all columns of its container table (text will still fit in current column)
		SelectableFlags_AllowDoubleClick = 1 << 2,			// Generate press events on double clicks too
		SelectableFlags_Disabled = 1 << 3,					// Cannot be selected, display grayed out text
		SelectableFlags_AllowOverlap = 1 << 4,				// (WIP) Hit testing to allow subsequent widgets to overlap this one
		SelectableFlags_Highlight = 1 << 5,					// Make the item be displayed as if it is hovered
		SelectableFlags_SelectOnNav = 1 << 6				// Auto-select when moved into, unless Ctrl is held. Automatic when in a BeginMultiSelect() block.
	};

	enum StyleVar : uint32_t
	{
		StyleVar_Alpha,										// float	Alpha
		StyleVar_DisabledAlpha,								// float	DisabledAlpha
		StyleVar_WindowPadding,								// float2	WindowPadding
		StyleVar_WindowRounding,							// float	WindowRounding
		StyleVar_WindowBorderSize,							// float	WindowBorderSize
		StyleVar_WindowMinSize,								// float2	WindowMinSize
		StyleVar_WindowTitleAlign,							// float2	WindowTitleAlign
		StyleVar_ChildRounding,								// float	ChildRounding
		StyleVar_ChildBorderSize,							// float	ChildBorderSize
		StyleVar_PopupRounding,								// float    PopupRounding
		StyleVar_PopupBorderSize,							// float    PopupBorderSize
		StyleVar_FramePadding,								// float2   FramePadding
		StyleVar_FrameRounding,								// float    FrameRounding
		StyleVar_FrameBorderSize,							// float    FrameBorderSize
		StyleVar_ItemSpacing,								// float2   ItemSpacing
		StyleVar_ItemInnerSpacing,							// float2   ItemInnerSpacing
		StyleVar_IndentSpacing,								// float    IndentSpacing
		StyleVar_CellPadding,								// float2   CellPadding
		StyleVar_ScrollbarSize,								// float    ScrollbarSize
		StyleVar_ScrollbarRounding,							// float    ScrollbarRounding
		StyleVar_ScrollbarPadding,							// float    ScrollbarPadding
		StyleVar_GrabMinSize,								// float    GrabMinSize
		StyleVar_GrabRounding,								// float    GrabRounding
		StyleVar_ImageBorderSize,							// float    ImageBorderSize
		StyleVar_TabRounding,								// float    TabRounding
		StyleVar_TabBorderSize,								// float    TabBorderSize
		StyleVar_TabMinWidthBase,							// float    TabMinWidthBase
		StyleVar_TabMinWidthShrink,							// float    TabMinWidthShrink
		StyleVar_TabBarBorderSize,							// float    TabBarBorderSize
		StyleVar_TabBarOverlineSize,						// float    TabBarOverlineSize
		StyleVar_TableAngledHeadersAngle,					// float    TableAngledHeadersAngle
		StyleVar_TableAngledHeadersTextAlign,				// float2	TableAngledHeadersTextAlign
		StyleVar_TreeLinesSize,								// float    TreeLinesSize
		StyleVar_TreeLinesRounding,							// float    TreeLinesRounding
		StyleVar_ButtonTextAlign,							// float2   ButtonTextAlign
		StyleVar_SelectableTextAlign,						// float2   SelectableTextAlign
		StyleVar_SeparatorTextBorderSize,					// float    SeparatorTextBorderSize
		StyleVar_SeparatorTextAlign,						// float2   SeparatorTextAlign
		StyleVar_SeparatorTextPadding,						// float2   SeparatorTextPadding
		StyleVar_DockingSeparatorSize						// float    DockingSeparatorSize
	};

	enum StyleColor : uint32_t
	{
		StyleColor_Text,
		StyleColor_TextDisabled,
		StyleColor_WindowBg,								// Background of normal windows
		StyleColor_ChildBg,									// Background of child windows
		StyleColor_PopupBg,									// Background of popups, menus, tooltips windows
		StyleColor_Border,
		StyleColor_BorderShadow,
		StyleColor_FrameBg,									// Background of checkbox, radio button, plot, slider, text input
		StyleColor_FrameBgHovered,
		StyleColor_FrameBgActive,
		StyleColor_TitleBg,									// Title bar
		StyleColor_TitleBgActive,							// Title bar when focused
		StyleColor_TitleBgCollapsed,						// Title bar when collapsed
		StyleColor_MenuBarBg,
		StyleColor_ScrollbarBg,
		StyleColor_ScrollbarGrab,
		StyleColor_ScrollbarGrabHovered,
		StyleColor_ScrollbarGrabActive,
		StyleColor_CheckMark,								// Checkbox tick and RadioButton circle
		StyleColor_SliderGrab,
		StyleColor_SliderGrabActive,
		StyleColor_Button,
		StyleColor_ButtonHovered,
		StyleColor_ButtonActive,
		StyleColor_Header,									// Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
		StyleColor_HeaderHovered,
		StyleColor_HeaderActive,
		StyleColor_Separator,
		StyleColor_SeparatorHovered,
		StyleColor_SeparatorActive,
		StyleColor_ResizeGrip,								// Resize grip in lower-right and lower-left corners of windows.
		StyleColor_ResizeGripHovered,
		StyleColor_ResizeGripActive,
		StyleColor_InputTextCursor,							// InputText cursor/caret
		StyleColor_TabHovered,								// Tab background, when hovered
		StyleColor_Tab,										// Tab background, when tab-bar is focused & tab is unselected
		StyleColor_TabSelected,								// Tab background, when tab-bar is focused & tab is selected
		StyleColor_TabSelectedOverline,						// Tab horizontal overline, when tab-bar is focused & tab is selected
		StyleColor_TabDimmed,								// Tab background, when tab-bar is unfocused & tab is unselected
		StyleColor_TabDimmedSelected,						// Tab background, when tab-bar is unfocused & tab is selected
		StyleColor_TabDimmedSelectedOverline,				//..horizontal overline, when tab-bar is unfocused & tab is selected
		StyleColor_DockingPreview,							// Preview overlay color when about to docking something
		StyleColor_DockingEmptyBg,							// Background color for empty node (e.g. CentralNode with no window docked into it)
		StyleColor_PlotLines,
		StyleColor_PlotLinesHovered,
		StyleColor_PlotHistogram,
		StyleColor_PlotHistogramHovered,
		StyleColor_TableHeaderBg,							// Table header background
		StyleColor_TableBorderStrong,						// Table outer and header borders (prefer using Alpha=1.0 here)
		StyleColor_TableBorderLight,						// Table inner borders (prefer using Alpha=1.0 here)
		StyleColor_TableRowBg,								// Table row background (even rows)
		StyleColor_TableRowBgAlt,							// Table row background (odd rows)
		StyleColor_TextLink,								// Hyperlink color
		StyleColor_TextSelectedBg,							// Selected text inside an InputText
		StyleColor_TreeLines,								// Tree node hierarchy outlines when using TreeNodeFlags_DrawLines
		StyleColor_DragDropTarget,							// Rectangle highlighting a drop target
		StyleColor_NavCursor,								// Color of keyboard/gamepad navigation cursor/rectangle, when visible
		StyleColor_NavWindowingHighlight,					// Highlight window when using CTRL+TAB
		StyleColor_NavWindowingDimBg,						// Darken/colorize entire screen behind the CTRL+TAB window list, when active
		StyleColor_ModalWindowDimBg							// Darken/colorize entire screen behind a modal window, when one is active
	};

	enum HoveredFlags : uint32_t
	{
		HoveredFlags_None = 0,								// Return true if directly over the item/window, not obstructed by another window, not obstructed by an active popup or modal blocking inputs under them.
		HoveredFlags_ChildWindows = 1 << 0,					// IsWindowHovered() only: Return true if any children of the window is hovered
		HoveredFlags_RootWindow = 1 << 1,					// IsWindowHovered() only: Test from root window (top most parent of the current hierarchy)
		HoveredFlags_AnyWindow = 1 << 2,					// IsWindowHovered() only: Return true if any window is hovered
		HoveredFlags_NoPopupHierarchy = 1 << 3,				// IsWindowHovered() only: Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
		HoveredFlags_DockHierarchy = 1 << 4,				// IsWindowHovered() only: Consider docking hierarchy (treat dockspace host as parent of docked window) (when used with _ChildWindows or _RootWindow)
		HoveredFlags_AllowWhenBlockedByPopup = 1 << 5,		// Return true even if a popup window is normally blocking access to this item/window
		//HoveredFlags_AllowWhenBlockedByModal = 1 << 6,	// Return true even if a modal popup window is normally blocking access to this item/window. FIXME-TODO: Unavailable yet.
		HoveredFlags_AllowWhenBlockedByActiveItem = 1 << 7, // Return true even if an active item is blocking access to this item/window. Useful for Drag and Drop patterns.
		HoveredFlags_AllowWhenOverlappedByItem = 1 << 8,	// IsItemHovered() only: Return true even if the item uses AllowOverlap mode and is overlapped by another hoverable item.
		HoveredFlags_AllowWhenOverlappedByWindow = 1 << 9,  // IsItemHovered() only: Return true even if the position is obstructed or overlapped by another window.
		HoveredFlags_AllowWhenDisabled = 1 << 10,			// IsItemHovered() only: Return true even if the item is disabled
		HoveredFlags_NoNavOverride = 1 << 11,				// IsItemHovered() only: Disable using keyboard/gamepad navigation state when active, always query mouse
		HoveredFlags_AllowWhenOverlapped = HoveredFlags_AllowWhenOverlappedByItem | HoveredFlags_AllowWhenOverlappedByWindow,
		HoveredFlags_RectOnly = HoveredFlags_AllowWhenBlockedByPopup | HoveredFlags_AllowWhenBlockedByActiveItem | HoveredFlags_AllowWhenOverlapped,
		HoveredFlags_RootAndChildWindows = HoveredFlags_RootWindow | HoveredFlags_ChildWindows,

		// Tooltips mode
		// - typically used in IsItemHovered() + SetTooltip() sequence.
		// - this is a shortcut to pull flags from 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav' where you can reconfigure desired behavior.
		//   e.g. 'TooltipHoveredFlagsForMouse' defaults to 'HoveredFlags_Stationary | HoveredFlags_DelayShort'.
		// - for frequently actioned or hovered items providing a tooltip, you want may to use HoveredFlags_ForTooltip (stationary + delay) so the tooltip doesn't show too often.
		// - for items which main purpose is to be hovered, or items with low affordance, or in less consistent apps, prefer no delay or shorter delay.
		HoveredFlags_ForTooltip = 1 << 12,					// Shortcut for standard flags when using IsItemHovered() + SetTooltip() sequence.

		// (Advanced) Mouse Hovering delays.
		// - generally you can use HoveredFlags_ForTooltip to use application-standardized flags.
		// - use those if you need specific overrides.
		HoveredFlags_Stationary = 1 << 13,				// Require mouse to be stationary for style.HoverStationaryDelay (~0.15 sec) _at least one time_. After this, can move on same item/window. Using the stationary test tends to reduces the need for a long delay.
		HoveredFlags_DelayNone = 1 << 14,				// IsItemHovered() only: Return true immediately (default). As this is the default you generally ignore this.
		HoveredFlags_DelayShort = 1 << 15,				// IsItemHovered() only: Return true after style.HoverDelayShort elapsed (~0.15 sec) (shared between items) + requires mouse to be stationary for style.HoverStationaryDelay (once per item).
		HoveredFlags_DelayNormal = 1 << 16,				// IsItemHovered() only: Return true after style.HoverDelayNormal elapsed (~0.40 sec) (shared between items) + requires mouse to be stationary for style.HoverStationaryDelay (once per item).
		HoveredFlags_NoSharedDelay = 1 << 17			// IsItemHovered() only: Disable shared delay system where moving from one item to the next keeps the previous timer for a short time (standard for tooltips with long delays)
	};

	enum SliderFlags : uint32_t
	{
		SliderFlags_None = 0,
		SliderFlags_Logarithmic = 1 << 5,				// Make the widget logarithmic (linear otherwise). Consider using ImGuiSliderFlags_NoRoundToFormat with this if using a format-string with small amount of digits.
		SliderFlags_NoRoundToFormat = 1 << 6,			// Disable rounding underlying value to match precision of the display format string (e.g. %.3f values are rounded to those 3 digits).
		SliderFlags_NoInput = 1 << 7,					// Disable CTRL+Click or Enter key allowing to input text directly into the widget.
		SliderFlags_WrapAround = 1 << 8,				// Enable wrapping around from max to min and from min to max. Only supported by DragXXX() functions for now.
		SliderFlags_ClampOnInput = 1 << 9,				// Clamp value to min/max bounds when input manually with CTRL+Click. By default CTRL+Click allows going out of bounds.
		SliderFlags_ClampZeroRange = 1 << 10,			// Clamp even if min==max==0.0f. Otherwise due to legacy reason DragXXX functions don't clamp with those values. When your clamping limits are dynamic you almost always want to use it.
		SliderFlags_NoSpeedTweaks = 1 << 11,			// Disable keyboard modifiers altering tweak speed. Useful if you want to alter tweak speed yourself based on your own logic.
		SliderFlags_AlwaysClamp = SliderFlags_ClampOnInput | SliderFlags_ClampZeroRange
	};

	enum PopupFlags : uint32_t
	{
		PopupFlags_None = 0,
		PopupFlags_MouseButtonLeft = 0,					// For BeginPopupContext*(): open on Left Mouse release. Guaranteed to always be == 0 (same as ImGuiMouseButton_Left)
		PopupFlags_MouseButtonRight = 1,				// For BeginPopupContext*(): open on Right Mouse release. Guaranteed to always be == 1 (same as ImGuiMouseButton_Right)
		PopupFlags_MouseButtonMiddle = 2,				// For BeginPopupContext*(): open on Middle Mouse release. Guaranteed to always be == 2 (same as ImGuiMouseButton_Middle)
		PopupFlags_MouseButtonMask_ = 0x1F,
		PopupFlags_MouseButtonDefault_ = 1,
		PopupFlags_NoReopen = 1 << 5,					// For OpenPopup*(), BeginPopupContext*(): don't reopen same popup if already open (won't reposition, won't reinitialize navigation)
		//PopupFlags_NoReopenAlwaysNavInit = 1 << 6,	// For OpenPopup*(), BeginPopupContext*(): focus and initialize navigation even when not reopening.
		PopupFlags_NoOpenOverExistingPopup = 1 << 7,	// For OpenPopup*(), BeginPopupContext*(): don't open if there's already a popup at the same level of the popup stack
		PopupFlags_NoOpenOverItems = 1 << 8,			// For BeginPopupContextWindow(): don't return true when hovering items, only when hovering empty space
		PopupFlags_AnyPopupId = 1 << 10,				// For IsPopupOpen(): ignore the ImGuiID parameter and test for any popup.
		PopupFlags_AnyPopupLevel = 1 << 11,				// For IsPopupOpen(): search/test at any level of the popup stack (default test in the current level)
		PopupFlags_AnyPopup = PopupFlags_AnyPopupId | PopupFlags_AnyPopupLevel,
	};
}