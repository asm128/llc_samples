#include "llc_sample_gui.h"
#include "llc_app_impl.h"
#include "llc_cstring.h"
#include "llc_framework.h"
#include "llc_runtime.h"
#include "llc_string.h"
#include "llc_system_key.h"

stacxpr ::llc::u0_t SAMPLE_GUI_THEME_COUNT = 3;
stacxpr ::llc::u0_t PALETTE_SHADE_COUNT = 16;
stacxpr ::llc::u0_t THEME_COUNT = SAMPLE_GUI_THEME_COUNT;
stacxpr ::llc::u0_t STATE_COUNT = 8;
stacxpr ::llc::u0_t ALIGNMENT_COUNT = 9;

GDEFINE_ENUM_TYPE(SAMPLE_GUI_THEME, ::llc::u0_t);
GDEFINE_ENUM_VALUED(SAMPLE_GUI_THEME, AZURE, 0, "Palette: Azure");
GDEFINE_ENUM_VALUED(SAMPLE_GUI_THEME, AMBER, 1, "Palette: Amber");
GDEFINE_ENUM_VALUED(SAMPLE_GUI_THEME, MONO , 2, "Palette: Mono");

GDEFINE_ENUM_TYPE(SAMPLE_GUI_STATE, ::llc::u0_t);
GDEFINE_ENUM_VALUED(SAMPLE_GUI_STATE, NORMAL           , 0                                                                  , "NORMAL");
GDEFINE_ENUM_VALUED(SAMPLE_GUI_STATE, HOVERED          , ::llc::GUI_CONTROL_FLAG_Hovered                                    , "HOVERED");
GDEFINE_ENUM_VALUED(SAMPLE_GUI_STATE, PRESSED          , ::llc::GUI_CONTROL_FLAG_Pressed                                    , "PRESSED");
GDEFINE_ENUM_VALUED(SAMPLE_GUI_STATE, DISABLED         , ::llc::GUI_CONTROL_FLAG_Disabled                                   , "DISABLED");
GDEFINE_ENUM_VALUED(SAMPLE_GUI_STATE, SELECTED         , ::llc::GUI_CONTROL_FLAG_Selected                                   , "SELECTED");
GDEFINE_ENUM_VALUED(SAMPLE_GUI_STATE, SELECTED_HOVERED , ::llc::GUI_CONTROL_FLAG_Selected | ::llc::GUI_CONTROL_FLAG_Hovered, "SELECTED + HOVERED");
GDEFINE_ENUM_VALUED(SAMPLE_GUI_STATE, SELECTED_PRESSED , ::llc::GUI_CONTROL_FLAG_Selected | ::llc::GUI_CONTROL_FLAG_Pressed, "SELECTED + PRESSED");
GDEFINE_ENUM_VALUED(SAMPLE_GUI_STATE, SELECTED_DISABLED, ::llc::GUI_CONTROL_FLAG_Selected | ::llc::GUI_CONTROL_FLAG_Disabled, "SELECTED + DISABLED");
GDEFINE_ENUM_VALUE (SAMPLE_GUI_STATE, FOCUSED          , ::llc::GUI_CONTROL_FLAG_Focused);
GDEFINE_ENUM_VALUE (SAMPLE_GUI_STATE, ACTION           , ::llc::GUI_CONTROL_FLAG_Action);

GDEFINE_ENUM_TYPE(SAMPLE_GUI_ALIGNMENT, ::llc::u0_t);
GDEFINE_ENUM_VALUE(SAMPLE_GUI_ALIGNMENT, TOP_LEFT     , ::llc::ALIGN_TOP_LEFT);
GDEFINE_ENUM_VALUE(SAMPLE_GUI_ALIGNMENT, CENTER_TOP   , ::llc::ALIGN_CENTER_TOP);
GDEFINE_ENUM_VALUE(SAMPLE_GUI_ALIGNMENT, TOP_RIGHT    , ::llc::ALIGN_TOP_RIGHT);
GDEFINE_ENUM_VALUE(SAMPLE_GUI_ALIGNMENT, CENTER_LEFT  , ::llc::ALIGN_CENTER_LEFT);
GDEFINE_ENUM_VALUE(SAMPLE_GUI_ALIGNMENT, CENTER       , ::llc::ALIGN_CENTER);
GDEFINE_ENUM_VALUE(SAMPLE_GUI_ALIGNMENT, CENTER_RIGHT , ::llc::ALIGN_CENTER_RIGHT);
GDEFINE_ENUM_VALUE(SAMPLE_GUI_ALIGNMENT, BOTTOM_LEFT  , ::llc::ALIGN_BOTTOM_LEFT);
GDEFINE_ENUM_VALUE(SAMPLE_GUI_ALIGNMENT, CENTER_BOTTOM, ::llc::ALIGN_CENTER_BOTTOM);
GDEFINE_ENUM_VALUE(SAMPLE_GUI_ALIGNMENT, BOTTOM_RIGHT , ::llc::ALIGN_BOTTOM_RIGHT);

sttc ::llc::vcst_t sampleGUIThemeText(::llc::u0_t theme) {
	rtrn ::llc::get_value_descv((SAMPLE_GUI_THEME)(theme % SAMPLE_GUI_THEME_COUNT));
}

sttc ::llc::vcst_t sampleGUIThemeName(::llc::u0_t theme) {
	rtrn ::llc::get_value_namev((SAMPLE_GUI_THEME)(theme % SAMPLE_GUI_THEME_COUNT));
}

sttc ::llc::SColorBGRA sampleGUIThemeColor(::llc::u0_t theme) nxpt {
	stacxpr ::llc::SColorBGRA colors[SAMPLE_GUI_THEME_COUNT] =
		{ {248, 208,  72}
		, { 32, 176, 248}
		, {232, 232, 232}
		};
	rtrn colors[theme % SAMPLE_GUI_THEME_COUNT];
}

sttc ::llc::SColorBGRA sampleGUIShade(::llc::SColorBGRA color, ::llc::u0_t shade) nxpt {
	shade = ::llc::min(shade, (::llc::u0_t)(PALETTE_SHADE_COUNT - 1));
	rtrn color * ((PALETTE_SHADE_COUNT - (::llc::f2_t)shade) / PALETTE_SHADE_COUNT);
}

sttc ::llc::SColorBGRA sampleGUIDesaturate(::llc::SColorBGRA color) nxpt {
	cnst ::llc::u0_t gray = (::llc::u0_t)(((::llc::u2_t)color.r + color.g + color.b) / 3);
	rtrn
		{ (::llc::u0_t)((color.b + (::llc::u1_t)gray * 2) / 3)
		, (::llc::u0_t)((color.g + (::llc::u1_t)gray * 2) / 3)
		, (::llc::u0_t)((color.r + (::llc::u1_t)gray * 2) / 3)
		};
}

sttc ::llc::u0_t sampleGUIStateShade(::llc::u0_t state) nxpt {
	stacxpr ::llc::u0_t shades[::llc::GUI_CONTROL_PALETTE_COUNT] = {8, 10, 5, 11, 4, 7, 2, 7, 1, 13};
	rtrn shades[state % ::llc::GUI_CONTROL_PALETTE_COUNT];
}

sttc ::llc::u0_t sampleGUIStateBorderOffset(::llc::u0_t state) nxpt {
	stacxpr ::llc::u0_t offsets[::llc::GUI_CONTROL_PALETTE_COUNT] = {5, 1, 7, 3, 0, 1, 7, 3, 5, 5};
	rtrn offsets[state % ::llc::GUI_CONTROL_PALETTE_COUNT];
}

sttc ::llc::SColorBGRA sampleGUITextColor(::llc::SColorBGRA background) nxpt {
	cnst ::llc::u2_t luminance = (::llc::u2_t)background.r * 299 + (::llc::u2_t)background.g * 587 + (::llc::u2_t)background.b * 114;
	rtrn ::sampleGUIShade({255, 255, 255}, luminance > 150000 ? 15 : 1);
}

sttc ::llc::err_t sampleGUIResourcesCreate(::llc::SGUI & gui) {
	if_true_fe(0 == gui.Colors.create());
	if_fail_fe(gui.Colors->Palettes.resize(SAMPLE_GUI_THEME_COUNT * ::llc::GUI_CONTROL_PALETTE_COUNT));
	for(::llc::u0_t iTheme = 0; iTheme < SAMPLE_GUI_THEME_COUNT; ++iTheme)
	for(::llc::u0_t iState = 0; iState < ::llc::GUI_CONTROL_PALETTE_COUNT; ++iState) {
		::llc::astatic<::llc::bgra, ::llc::UI_CONTROL_AREA_COUNT> & palette = gui.Colors->Palettes[iTheme * ::llc::GUI_CONTROL_PALETTE_COUNT + iState];
		::llc::SColorBGRA tone = ::sampleGUIThemeColor(iTheme);
		if(iState == ::llc::GUI_CONTROL_PALETTE_DISABLED || iState == ::llc::GUI_CONTROL_PALETTE_SELECTED_DISABLED)
			tone = ::sampleGUIDesaturate(tone);
		cnst ::llc::u0_t shade = ::sampleGUIStateShade(iState);
		cnst ::llc::SColorBGRA base = ::sampleGUIShade(tone, shade);
		cnst ::llc::u0_t borderOffset = ::sampleGUIStateBorderOffset(iState);
		cnst ::llc::u0_t borderShade = shade > 7
			? (::llc::u0_t)(shade > borderOffset ? shade - borderOffset : 0)
			: (::llc::u0_t)::llc::min((::llc::u2_t)shade + borderOffset, (::llc::u2_t)PALETTE_SHADE_COUNT - 1);
		cnst ::llc::SColorBGRA borderColor = ::sampleGUIShade(tone, borderShade);
		for(::llc::u2_t iArea = 0; iArea < palette.size(); ++iArea)
			palette[iArea] = ::sampleGUIShade(tone, (::llc::u0_t)::llc::min((::llc::u2_t)shade + 5, (::llc::u2_t)PALETTE_SHADE_COUNT - 1));
		palette[::llc::UI_CONTROL_AREA_BACKGROUND] = ::sampleGUIShade(tone, (::llc::u0_t)::llc::min((::llc::u2_t)shade + 6, (::llc::u2_t)PALETTE_SHADE_COUNT - 1));
		palette[::llc::UI_CONTROL_AREA_CLIENT] = base;
		palette[::llc::UI_CONTROL_AREA_BORDER_LEFT] = borderColor;
		palette[::llc::UI_CONTROL_AREA_BORDER_TOP] = borderColor;
		palette[::llc::UI_CONTROL_AREA_BORDER_RIGHT] = borderColor;
		palette[::llc::UI_CONTROL_AREA_BORDER_BOTTOM] = borderColor;
		palette[::llc::UI_CONTROL_AREA_TEXT_BACKGROUND] = ::sampleGUIShade(tone, (::llc::u0_t)::llc::min((::llc::u2_t)shade + 2, (::llc::u2_t)PALETTE_SHADE_COUNT - 1));
		palette[::llc::UI_CONTROL_AREA_TEXT_FACE] = ::sampleGUITextColor(base);
	}
	::llc::SRasterFontManager fonts = {};
	if_fail_fe(::llc::rasterFontDefaults(fonts));
	gui.Fonts = fonts.Fonts;
	rtrn 0;
}

sttc void sampleGUIControlThemeApply(::llc::SGUI & gui, ::llc::cid_t idControl, ::llc::u0_t theme) nxpt {
	if(::llc::controlInvalid(gui, idControl))
		rtrn;
	::llc::SControlDraw & draw = gui.Controls.Draw[idControl];
	draw.UseNewPalettes = true;
	for(::llc::u0_t iState = 0; iState < ::llc::GUI_CONTROL_PALETTE_COUNT; ++iState)
		draw.Palettes[iState] = (::llc::s1_t)((theme % SAMPLE_GUI_THEME_COUNT) * ::llc::GUI_CONTROL_PALETTE_COUNT + iState);
}

sttc void sampleGUIThemeApply(::llc::SGUI & gui, ::llc::u0_t theme) nxpt {
	for(::llc::u2_t iControl = 0; iControl < gui.Controls.Draw.size(); ++iControl)
		::sampleGUIControlThemeApply(gui, (::llc::cid_t)iControl, theme);
}

sttc ::llc::vcst_t sampleGUIStateText(::llc::u0_t state) {
	rtrn ::llc::get_value_descv(::llc::get_enum<SAMPLE_GUI_STATE>().get_value_by_index(state < STATE_COUNT ? state : 0));
}

sttc ::llc::ALIGN	sampleGUIControlAlignment	(::llc::u0_t alignment) nxpt	{ rtrn (::llc::ALIGN)::llc::get_enum<SAMPLE_GUI_ALIGNMENT>().get_value_by_index(alignment % ALIGNMENT_COUNT); }
sttc ::llc::ALIGN	sampleGUITextAlignment		(::llc::u0_t alignment) nxpt	{ rtrn (::llc::ALIGN)::llc::get_enum<SAMPLE_GUI_ALIGNMENT>().get_value_by_index(ALIGNMENT_COUNT - 1 - alignment % ALIGNMENT_COUNT); }
sttc ::llc::vcst_t	sampleGUIAlignmentText		(::llc::u0_t alignment)			{ rtrn ::llc::get_enum<SAMPLE_GUI_ALIGNMENT>().get_label_by_index(alignment % ALIGNMENT_COUNT); }

sttc ::llc::GUI_CONTROL_FLAG sampleGUIStateFlags(::llc::u0_t state) nxpt {
	rtrn (::llc::GUI_CONTROL_FLAG)::llc::get_enum<SAMPLE_GUI_STATE>().get_value_by_index(state < STATE_COUNT ? state : 0);
}

sttc ::llc::GUI_CONTROL_PALETTE sampleGUIControlPalette(bool disabled, cnst ::llc::SControlState & state) nxpt {
	cnst bool highlighted = state.IsHovered() || state.IsFocused();
	if(state.IsSelected()) {
		if(disabled)          rtrn ::llc::GUI_CONTROL_PALETTE_SELECTED_DISABLED;
		if(state.IsPressed()) rtrn ::llc::GUI_CONTROL_PALETTE_SELECTED_PRESSED;
		if(highlighted)       rtrn ::llc::GUI_CONTROL_PALETTE_SELECTED_HOVER;
		rtrn ::llc::GUI_CONTROL_PALETTE_SELECTED;
	}
	if(disabled)          rtrn ::llc::GUI_CONTROL_PALETTE_DISABLED;
	if(state.IsPressed()) rtrn ::llc::GUI_CONTROL_PALETTE_PRESSED;
	if(highlighted)       rtrn ::llc::GUI_CONTROL_PALETTE_HOVER;
	rtrn ::llc::GUI_CONTROL_PALETTE_NORMAL;
}

sttc ::llc::err_t sampleGUIControlCreate(SSampleGUI & app, ::llc::cid_t & idControl, ::llc::vcst_t text, cnst ::llc::rect2s1_t & area, ::llc::s1_t font = -1, ::llc::ALIGN align = ::llc::ALIGN_TOP_LEFT) {
	if_fail_fe(idControl = ::llc::controlCreate(app.GUI));
	::llc::SControlPlacement & placement = app.GUI.Controls.Placement[idControl];
	placement.Area = area;
	placement.Align = align;
	placement.Margin = {2, 2, 2, 2};
	placement.Border = {3, 3, 3, 3};
	::llc::SControlDraw & draw = app.GUI.Controls.Draw[idControl];
	draw.UseNewPalettes = true;
	draw.ColorMode = ::llc::GUI_COLOR_MODE_Default;
	app.GUI.Controls.Text[idControl].Text = text;
	app.GUI.Controls.Text[idControl].FontSelected = font;
	rtrn 0;
}

sttc void sampleGUIThemeApply(SSampleGUI & app) nxpt {
	::sampleGUIThemeApply(app.GUI, app.Theme);
	for(::llc::u0_t iTheme = 0; iTheme < THEME_COUNT; ++iTheme)
		app.GUI.Controls.SetSelected(app.ControlsTheme[iTheme], app.Theme == iTheme);
}

sttc void sampleGUIStateCardsApply(SSampleGUI & app) nxpt {
	for(::llc::u0_t iState = 0; iState < STATE_COUNT; ++iState) {
		::llc::SControlState & state = app.GUI.Controls.States[app.ControlsState[iState]];
		cnst ::llc::GUI_CONTROL_FLAG updated = (::llc::GUI_CONTROL_FLAG)(state.Flag & ::llc::GUI_CONTROL_FLAG_Updated);
		state.Flag = (::llc::GUI_CONTROL_FLAG)(updated | ::sampleGUIStateFlags(iState));
	}
}

sttc ::llc::err_t sampleGUIStateFlagAppend(::llc::string & text, ::llc::vcst_t flag) {
	if(text.size())
		if_fail_fe(text.push_back('+'));
	rtrn text.append_string(flag);
}

sttc ::llc::err_t sampleGUIStateDescribe(cnst ::llc::SControlState & state, bool disabled, ::llc::string & text) {
	text.clear();
	if(state.IsSelected()) if_fail_fe(::sampleGUIStateFlagAppend(text, ::llc::get_value_namev(SAMPLE_GUI_STATE_SELECTED)));
	if(state.IsHovered ()) if_fail_fe(::sampleGUIStateFlagAppend(text, ::llc::get_value_namev(SAMPLE_GUI_STATE_HOVERED )));
	if(state.IsPressed ()) if_fail_fe(::sampleGUIStateFlagAppend(text, ::llc::get_value_namev(SAMPLE_GUI_STATE_PRESSED )));
	if(state.IsFocused ()) if_fail_fe(::sampleGUIStateFlagAppend(text, ::llc::get_value_namev(SAMPLE_GUI_STATE_FOCUSED )));
	if(disabled          ) if_fail_fe(::sampleGUIStateFlagAppend(text, ::llc::get_value_namev(SAMPLE_GUI_STATE_DISABLED)));
	if(state.IsAction  ()) if_fail_fe(::sampleGUIStateFlagAppend(text, ::llc::get_value_namev(SAMPLE_GUI_STATE_ACTION  )));
	if(0 == text.size())
		if_fail_fe(text.append_string(::llc::get_value_namev(SAMPLE_GUI_STATE_NORMAL)));
	rtrn text.size();
}

sttc ::llc::err_t sampleGUIControlTextSetOwned(SSampleGUI & app, ::llc::cid_t idControl, ::llc::astchar<512> & storage, ::llc::vcst_t text) {
	if(app.GUI.Controls.Text[idControl].Text == text)
		rtrn 0;
	if_true_fe(text.size() >= storage.size());
	for(::llc::u2_t iCharacter = 0; iCharacter < text.size(); ++iCharacter)
		storage.Storage[iCharacter] = text[iCharacter];
	storage.Storage[text.size()] = 0;
	rtrn ::llc::controlTextSet(app.GUI, idControl, {storage.Storage, text.size()});
}

sttc ::llc::vcst_t sampleGUIControlName(cnst SSampleGUI & app, ::llc::cid_t idControl) {
	if(idControl == app.ControlTitle         ) rtrn LLC_CXS("TITLE");
	if(idControl == app.ControlMode          ) rtrn LLC_CXS("MODE");
	if(idControl == app.ControlLive          ) rtrn LLC_CXS("LIVE CONTROL");
	if(idControl == app.ControlInspector     ) rtrn LLC_CXS("GLOBAL INSPECTOR");
	if(idControl == app.ControlHelp          ) rtrn LLC_CXS("HELP");
	if(idControl == app.ControlExit          ) rtrn LLC_CXS("EXIT");
	if(idControl == app.ControlAlignmentTitle) rtrn LLC_CXS("ALIGNMENT TITLE");
	if(idControl == app.ControlAlignmentRoot ) rtrn LLC_CXS("ALIGNMENT STAGE");

	rtrn app.GUI.Controls.Text[idControl].Text;
}

sttc ::llc::cid_t sampleGUIHoveredControl(cnst SSampleGUI & app) nxpt {
	for(::llc::u2_t iControl = 0; iControl < app.GUI.Controls.States.size(); ++iControl)
		if(false == ::llc::controlInvalid(app.GUI, (::llc::cid_t)iControl) && app.GUI.Controls.IsHovered((::llc::cid_t)iControl))
			rtrn (::llc::cid_t)iControl;
	rtrn ::llc::CID_INVALID;
}

sttc ::llc::err_t sampleGUILiveTextUpdate(SSampleGUI & app) {
	cnst ::llc::SControlState & state = app.GUI.Controls.States[app.ControlLive];
	cnst bool disabled = 0 != ::llc::controlDisabled(app.GUI, app.ControlLive);
	::llc::string stateText = {};
	if_fail_fe(::sampleGUIStateDescribe(state, disabled, stateText));
	cnst ::llc::GUI_CONTROL_PALETTE palette = ::sampleGUIControlPalette(disabled, state);
	cnst ::llc::vcst_t paletteName = ::llc::get_value_namev(palette);
	cnst ::llc::s1_t paletteSlot = app.GUI.Controls.Draw[app.ControlLive].Palettes[palette];
	::llc::astchar<512> text = {};
	cnst ::llc::err_t length = ::llc::sprintf_s(text.Storage, "LIVE | STATE:%.*s | PALETTE:%.*s [%i] | CLICK TO TOGGLE SELECTED", (int)stateText.size(), stateText.begin(), (int)paletteName.size(), paletteName.begin(), paletteSlot);
	if_fail_fe(length);
	rtrn ::sampleGUIControlTextSetOwned(app, app.ControlLive, app.TextLive, {text.Storage, (::llc::u2_t)length});
}

sttc ::llc::err_t sampleGUIInspectorTextUpdate(SSampleGUI & app, ::llc::cid_t idHovered) {
	cnst ::llc::vcst_t themeName = ::sampleGUIThemeName(app.Theme);
	cnst ::llc::vcst_t modeName = app.GUI.ColorModeDefault == ::llc::GUI_COLOR_MODE_3D ? LLC_CXS("3D") : LLC_CXS("FLAT");
	::llc::astchar<512> text = {};
	::llc::err_t length = 0;
	if(::llc::controlInvalid(app.GUI, idHovered))
		length = ::llc::sprintf_s(text.Storage, "GUI | THEME:%.*s | MODE:%.*s | HOVER:NONE", (int)themeName.size(), themeName.begin(), (int)modeName.size(), modeName.begin());
	else {
		cnst ::llc::SControlState & state = app.GUI.Controls.States[idHovered];
		cnst bool disabled = 0 != ::llc::controlDisabled(app.GUI, idHovered);
		::llc::string stateText = {};
		if_fail_fe(::sampleGUIStateDescribe(state, disabled, stateText));
		cnst ::llc::GUI_CONTROL_PALETTE palette = ::sampleGUIControlPalette(disabled, state);
		cnst ::llc::vcst_t paletteName = ::llc::get_value_namev(palette);
		cnst ::llc::vcst_t controlName = ::sampleGUIControlName(app, idHovered);
		cnst ::llc::vcst_t controlAlignName = ::llc::get_value_namev(app.GUI.Controls.Placement[idHovered].Align);
		cnst ::llc::vcst_t textAlignName = ::llc::get_value_namev(app.GUI.Controls.Text[idHovered].Align);
		cnst ::llc::s1_t paletteSlot = app.GUI.Controls.Draw[idHovered].Palettes[palette];
		length = ::llc::sprintf_s(text.Storage, "GUI | THEME:%.*s | MODE:%.*s | HOVER #%i %.*s | STATE:%.*s | PALETTE:%.*s [%i] | ALIGN:%.*s / TEXT:%.*s", (int)themeName.size(), themeName.begin(), (int)modeName.size(), modeName.begin(), idHovered, (int)controlName.size(), controlName.begin(), (int)stateText.size(), stateText.begin(), (int)paletteName.size(), paletteName.begin(), paletteSlot, (int)controlAlignName.size(), controlAlignName.begin(), (int)textAlignName.size(), textAlignName.begin());
	}
	if_fail_fe(length);
	rtrn ::sampleGUIControlTextSetOwned(app, app.ControlInspector, app.TextInspector, {text.Storage, (::llc::u2_t)length});
}

::llc::err_t sampleGUISetup(SSampleGUI & app) {
	app.GUI.SelectedFont = 2;
	app.GUI.ColorModeDefault = ::llc::GUI_COLOR_MODE_3D;
	if_fail_fe(::sampleGUIResourcesCreate(app.GUI));

	if_fail_fe(::sampleGUIControlCreate(app, app.ControlTitle, LLC_CXS("LLC GUI CONTROL PALETTES AND STATES"), {{24, 18}, {650, 42}}, 3));
	if_fail_fe(::sampleGUIControlCreate(app, app.ControlMode, LLC_CXS("Mode: 3D"), {{742, 18}, {184, 42}}, 2));
	for(::llc::u0_t iTheme = 0; iTheme < THEME_COUNT; ++iTheme) {
		if_fail_fe(::sampleGUIControlCreate(app, app.ControlsTheme[iTheme], ::sampleGUIThemeText(iTheme), {{(::llc::s1_t)(24 + 226 * iTheme), 76}, {206, 40}}, 2));
	}

	for(::llc::u0_t iState = 0; iState < STATE_COUNT; ++iState) {
		cnst ::llc::s1_t x = (::llc::s1_t)(24 + 226 * (iState % 4));
		cnst ::llc::s1_t y = (::llc::s1_t)(140 + 88 * (iState / 4));
		if_fail_fe(::sampleGUIControlCreate(app, app.ControlsState[iState], ::sampleGUIStateText(iState), {{x, y}, {206, 68}}, 2));
	}
	if_fail_fe(::sampleGUIControlCreate(app, app.ControlLive, LLC_CXS("LIVE"), {{24, 138}, {884, 64}}, 2, ::llc::ALIGN_BOTTOM_LEFT));
	if_fail_fe(::sampleGUIControlCreate(app, app.ControlInspector, LLC_CXS("GUI | HOVER:NONE"), {{24, 68}, {884, 56}}, 1, ::llc::ALIGN_BOTTOM_LEFT));
	if_fail_fe(::sampleGUIControlCreate(app, app.ControlHelp, LLC_CXS("TAB FOCUS | ENTER/SPACE ACTIVATE | ESC/EXIT CLOSE | PALETTES SHARE ONE 16-SHADE TONE"), {{136, 18}, {772, 36}}, 1, ::llc::ALIGN_BOTTOM_LEFT));
	if_fail_fe(::sampleGUIControlCreate(app, app.ControlExit, LLC_CXS("EXIT"), {{24, 18}, {96, 36}}, 2, ::llc::ALIGN_BOTTOM_LEFT));
	if_fail_fe(::sampleGUIControlCreate(app, app.ControlAlignmentTitle, LLC_CXS("CONTROL ALIGNMENT / OPPOSITE TEXT ALIGNMENT"), {{24, 18}, {332, 42}}, 1, ::llc::ALIGN_TOP_RIGHT));
	if_fail_fe(::sampleGUIControlCreate(app, app.ControlAlignmentRoot, LLC_CXS(""), {{24, 76}, {332, 446}}, -1, ::llc::ALIGN_TOP_RIGHT));
	for(::llc::u0_t iAlignment = 0; iAlignment < ALIGNMENT_COUNT; ++iAlignment) {
		if_fail_fe(::sampleGUIControlCreate(app, app.ControlsAlignment[iAlignment], ::sampleGUIAlignmentText(iAlignment), {{0, 0}, {108, 58}}, 1, ::sampleGUIControlAlignment(iAlignment)));
		app.GUI.Controls.Text[app.ControlsAlignment[iAlignment]].Align = ::sampleGUITextAlignment(iAlignment);
		if_fail_fe(::llc::controlSetParent(app.GUI, app.ControlsAlignment[iAlignment], app.ControlAlignmentRoot));
	}
	app.ControlsKeyboard[0] = app.ControlMode;
	for(::llc::u0_t iTheme = 0; iTheme < THEME_COUNT; ++iTheme) {
		app.ControlsKeyboard[1 + iTheme] = app.ControlsTheme[iTheme];
	}
	app.ControlsKeyboard[4] = app.ControlLive;
	app.ControlsKeyboard[5] = app.ControlExit;

	::sampleGUIThemeApply(app);
	::sampleGUIStateCardsApply(app);
	if_fail_fe(::sampleGUILiveTextUpdate(app));
	if_fail_fe(::sampleGUIInspectorTextUpdate(app, ::llc::CID_INVALID));
	rtrn 0;
}

::llc::err_t sampleGUIUpdate(SSampleGUI & app, bool systemRequestedExit) {
	if(systemRequestedExit || app.Window.Closed || app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Escape))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if(app.GUI.Controls.IsAction(app.ControlExit))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if(app.GUI.Controls.IsAction(app.ControlMode))
		app.GUI.ColorModeDefault = app.GUI.ColorModeDefault == ::llc::GUI_COLOR_MODE_3D ? ::llc::GUI_COLOR_MODE_Flat : ::llc::GUI_COLOR_MODE_3D;
	for(::llc::u0_t iTheme = 0; iTheme < THEME_COUNT; ++iTheme)
		if(app.GUI.Controls.IsAction(app.ControlsTheme[iTheme]))
			app.Theme = iTheme;
	if(app.GUI.Controls.IsAction(app.ControlLive))
		app.GUI.Controls.SetSelected(app.ControlLive, false == app.GUI.Controls.IsSelected(app.ControlLive));
	cnst ::llc::cid_t idHovered = ::sampleGUIHoveredControl(app);
	if_fail_fe(::llc::controlTextSet(app.GUI, app.ControlMode, app.GUI.ColorModeDefault == ::llc::GUI_COLOR_MODE_3D ? LLC_CXS("Mode: 3D") : LLC_CXS("Mode: Flat")));
	::sampleGUIThemeApply(app);
	::sampleGUIStateCardsApply(app);
	if_fail_fe(::sampleGUILiveTextUpdate(app));
	if_fail_fe(::sampleGUIInspectorTextUpdate(app, idHovered));
	rtrn 0;
}

::llc::err_t sampleGUIDraw(SSampleGUI & app, ::llc::g8bgra target) {
	if_fail_fe(target.fill(::llc::SColorBGRA{16, 14, 12}));
	rtrn ::llc::guiDraw(app.GUI, target);
}

#define LLC_SAMPLE_GUI_TITLE "LLC GUI palettes, states and alignment"

stct SApplication {
	::llc::SFramework	Framework;
	SSampleGUI			Sample;

	SApplication(::llc::SRuntimeValues & runtimeValues)
		: Framework(runtimeValues, {}), Sample(Framework.RootWindow, *Framework.GUI) {}
};

::llc::err_t setup(SApplication & app) {
	::llc::SWindow & window = app.Framework.RootWindow;
	window.Size = {1280, 540};
	if_null_ve(::llc::OS_NO_MEMORY, window.BackBuffer.create());
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	if_fail_fe(::llc::mainWindowCreate(window, app.Framework.RuntimeValues.PlatformDetail, window.Input));
	rtrn ::sampleGUISetup(app.Sample);
}

::llc::err_t cleanup(SApplication & app) {
	rtrn ::llc::mainWindowDestroy(app.Framework.RootWindow);
}

::llc::err_t update(SApplication & app, bool systemRequestedExit) {
	if(systemRequestedExit)
		rtrn ::llc::APPLICATION_STATE_EXIT;
	cnst ::llc::err_t result = ::llc::updateFramework(app.Framework);
	if(result)
		rtrn result;
	cnst ::llc::err_t resultSample = ::sampleGUIUpdate(app.Sample, false);
	::llc::sleep(1);
	rtrn resultSample;
}

::llc::err_t draw(SApplication & app) {
	::llc::SWindow & window = app.Framework.RootWindow;
	if(window.NoDraw)
		rtrn 0;
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	rtrn ::sampleGUIDraw(app.Sample, window.BackBuffer->Color.View);
}

LLC_DEFINE_APPLICATION_ENTRY_POINT(SApplication, LLC_SAMPLE_GUI_TITLE)
