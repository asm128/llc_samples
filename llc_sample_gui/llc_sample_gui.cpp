#include "llc_sample_gui.h"
#include "llc_cstring.h"
#include "llc_runtime.h"
#include "llc_string.h"

stacxpr ::llc::u0_t THEME_COUNT = 3;
stacxpr ::llc::u0_t STATE_COUNT = 8;
stacxpr ::llc::u0_t ALIGNMENT_COUNT = 9;

sttc ::llc::vcst_t sampleGUIThemeText(::llc::u0_t theme) {
	switch(theme % THEME_COUNT) {
	default:
	case 0: rtrn LLC_CXS("Palette: Azure");
	case 1: rtrn LLC_CXS("Palette: Amber");
	case 2: rtrn LLC_CXS("Palette: Mono");
	}
}

sttc ::llc::vcst_t sampleGUIStateText(::llc::u0_t state) {
	switch(state) {
	default:
	case 0: rtrn LLC_CXS("NORMAL");
	case 1: rtrn LLC_CXS("HOVERED");
	case 2: rtrn LLC_CXS("PRESSED");
	case 3: rtrn LLC_CXS("DISABLED");
	case 4: rtrn LLC_CXS("SELECTED");
	case 5: rtrn LLC_CXS("SELECTED + HOVERED");
	case 6: rtrn LLC_CXS("SELECTED + PRESSED");
	case 7: rtrn LLC_CXS("SELECTED + DISABLED");
	}
}

sttc ::llc::vcst_t sampleGUIThemeName(::llc::u0_t theme) {
	switch(theme % THEME_COUNT) {
	default:
	case 0: rtrn LLC_CXS("AZURE");
	case 1: rtrn LLC_CXS("AMBER");
	case 2: rtrn LLC_CXS("MONO");
	}
}

sttc ::llc::ALIGN sampleGUIControlAlignment(::llc::u0_t alignment) nxpt {
	switch(alignment % ALIGNMENT_COUNT) {
	default:
	case 0: rtrn ::llc::ALIGN_TOP_LEFT;
	case 1: rtrn ::llc::ALIGN_CENTER_TOP;
	case 2: rtrn ::llc::ALIGN_TOP_RIGHT;
	case 3: rtrn ::llc::ALIGN_CENTER_LEFT;
	case 4: rtrn ::llc::ALIGN_CENTER;
	case 5: rtrn ::llc::ALIGN_CENTER_RIGHT;
	case 6: rtrn ::llc::ALIGN_BOTTOM_LEFT;
	case 7: rtrn ::llc::ALIGN_CENTER_BOTTOM;
	case 8: rtrn ::llc::ALIGN_BOTTOM_RIGHT;
	}
}

sttc ::llc::ALIGN sampleGUITextAlignment(::llc::u0_t alignment) nxpt {
	switch(alignment % ALIGNMENT_COUNT) {
	default:
	case 0: rtrn ::llc::ALIGN_BOTTOM_RIGHT;
	case 1: rtrn ::llc::ALIGN_CENTER_BOTTOM;
	case 2: rtrn ::llc::ALIGN_BOTTOM_LEFT;
	case 3: rtrn ::llc::ALIGN_CENTER_RIGHT;
	case 4: rtrn ::llc::ALIGN_CENTER;
	case 5: rtrn ::llc::ALIGN_CENTER_LEFT;
	case 6: rtrn ::llc::ALIGN_TOP_RIGHT;
	case 7: rtrn ::llc::ALIGN_CENTER_TOP;
	case 8: rtrn ::llc::ALIGN_TOP_LEFT;
	}
}

sttc ::llc::vcst_t sampleGUIAlignmentText(::llc::u0_t alignment) {
	switch(alignment % ALIGNMENT_COUNT) {
	default:
	case 0: rtrn LLC_CXS("TOP_LEFT");
	case 1: rtrn LLC_CXS("CENTER_TOP");
	case 2: rtrn LLC_CXS("TOP_RIGHT");
	case 3: rtrn LLC_CXS("CENTER_LEFT");
	case 4: rtrn LLC_CXS("CENTER");
	case 5: rtrn LLC_CXS("CENTER_RIGHT");
	case 6: rtrn LLC_CXS("BOTTOM_LEFT");
	case 7: rtrn LLC_CXS("CENTER_BOTTOM");
	case 8: rtrn LLC_CXS("BOTTOM_RIGHT");
	}
}

sttc ::llc::GUI_CONTROL_FLAG sampleGUIStateFlags(::llc::u0_t state) nxpt {
	switch(state) {
	default:
	case 0: rtrn {};
	case 1: rtrn ::llc::GUI_CONTROL_FLAG_Hovered;
	case 2: rtrn ::llc::GUI_CONTROL_FLAG_Pressed;
	case 3: rtrn ::llc::GUI_CONTROL_FLAG_Disabled;
	case 4: rtrn ::llc::GUI_CONTROL_FLAG_Selected;
	case 5: rtrn (::llc::GUI_CONTROL_FLAG)(::llc::GUI_CONTROL_FLAG_Selected | ::llc::GUI_CONTROL_FLAG_Hovered);
	case 6: rtrn (::llc::GUI_CONTROL_FLAG)(::llc::GUI_CONTROL_FLAG_Selected | ::llc::GUI_CONTROL_FLAG_Pressed);
	case 7: rtrn (::llc::GUI_CONTROL_FLAG)(::llc::GUI_CONTROL_FLAG_Selected | ::llc::GUI_CONTROL_FLAG_Disabled);
	}
}

sttc ::llc::cid_t sampleGUIControlCreate(SSampleGUI & app, ::llc::vcst_t text, cnst ::llc::rect2s1_t & area, ::llc::s1_t font = -1) {
	cnst ::llc::cid_t idControl = ::llc::controlCreate(app.GUI);
	if(idControl < 0)
		rtrn idControl;
	::llc::SControlPlacement & placement = app.GUI.Controls.Placement[idControl];
	placement.Area = area;
	placement.Margin = {2, 2, 2, 2};
	placement.Border = {3, 3, 3, 3};
	::llc::SControlDraw & draw = app.GUI.Controls.Draw[idControl];
	draw.UseNewPalettes = true;
	draw.ColorMode = ::llc::GUI_COLOR_MODE_Default;
	app.GUI.Controls.Text[idControl].Text = text;
	app.GUI.Controls.Text[idControl].FontSelected = font;
	rtrn idControl;
}

stacxpr ::llc::u0_t PALETTE_SHADE_COUNT = 16;

sttc ::llc::SColorBGRA sampleGUIThemeColor(::llc::u0_t theme) nxpt {
	stacxpr ::llc::SColorBGRA colors[THEME_COUNT] =
		{ {248, 208,  72}
		, { 32, 176, 248}
		, {232, 232, 232}
		};
	rtrn colors[theme % THEME_COUNT];
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

sttc ::llc::err_t sampleGUIResourcesCreate(SSampleGUI & app) {
	if_true_fe(0 == app.Resources.Colors.create());
	if_fail_fe(app.Resources.Colors->Palettes.resize(THEME_COUNT * ::llc::GUI_CONTROL_PALETTE_COUNT));
	for(::llc::u0_t iTheme = 0; iTheme < THEME_COUNT; ++iTheme)
	for(::llc::u0_t iState = 0; iState < ::llc::GUI_CONTROL_PALETTE_COUNT; ++iState) {
		::llc::astatic<::llc::bgra, ::llc::UI_CONTROL_AREA_COUNT> & palette = app.Resources.Colors->Palettes[iTheme * ::llc::GUI_CONTROL_PALETTE_COUNT + iState];
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
	app.Resources.Fonts = fonts.Fonts;
	rtrn 0;
}

sttc void sampleGUIThemeApply(SSampleGUI & app) nxpt {
	for(::llc::u2_t iControl = 0; iControl < app.GUI.Controls.Draw.size(); ++iControl) {
		cnst ::llc::cid_t idControl = (::llc::cid_t)iControl;
		if(::llc::controlInvalid(app.GUI, idControl))
			continue;
		::llc::SControlDraw & draw = app.GUI.Controls.Draw[iControl];
		draw.UseNewPalettes = true;
		for(::llc::u0_t iState = 0; iState < ::llc::GUI_CONTROL_PALETTE_COUNT; ++iState)
			draw.Palettes[iState] = (::llc::s1_t)(app.Theme * ::llc::GUI_CONTROL_PALETTE_COUNT + iState);
	}
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
	if(state.IsSelected()) if_fail_fe(::sampleGUIStateFlagAppend(text, LLC_CXS("SELECTED")));
	if(state.IsHovered ()) if_fail_fe(::sampleGUIStateFlagAppend(text, LLC_CXS("HOVERED")));
	if(state.IsPressed ()) if_fail_fe(::sampleGUIStateFlagAppend(text, LLC_CXS("PRESSED")));
	if(state.IsFocused ()) if_fail_fe(::sampleGUIStateFlagAppend(text, LLC_CXS("FOCUSED")));
	if(disabled          ) if_fail_fe(::sampleGUIStateFlagAppend(text, LLC_CXS("DISABLED")));
	if(state.IsAction  ()) if_fail_fe(::sampleGUIStateFlagAppend(text, LLC_CXS("ACTION")));
	if(0 == text.size())
		if_fail_fe(text.append_string(LLC_CXS("NORMAL")));
	rtrn text.size();
}

sttc ::llc::err_t sampleGUIControlTextSetOwned(SSampleGUI & app, ::llc::cid_t idControl, ::llc::astchar<512> & storage, ::llc::vcst_t text) {
	if(app.GUI.Controls.Text[idControl].Text == text)
		rtrn 0;
	if_true_fe(text.size() >= storage.size());
	for(::llc::u2_t iCharacter = 0; iCharacter < text.size(); ++iCharacter)
		storage.Storage[iCharacter] = text[iCharacter];
	storage.Storage[text.size()] = 0;
	app.GUI.Controls.Text[idControl].Text = {storage.Storage, text.size()};
	app.GUI.Controls.SetUpdated(idControl, false);
	rtrn 0;
}

sttc ::llc::vcst_t sampleGUIControlName(cnst SSampleGUI & app, ::llc::cid_t idControl) {
	if(idControl == app.ControlTitle    ) rtrn LLC_CXS("TITLE");
	if(idControl == app.ControlMode     ) rtrn LLC_CXS("MODE");
	if(idControl == app.ControlLive     ) rtrn LLC_CXS("LIVE CONTROL");
	if(idControl == app.ControlInspector) rtrn LLC_CXS("GLOBAL INSPECTOR");
	if(idControl == app.ControlHelp     ) rtrn LLC_CXS("HELP");
	if(idControl == app.ControlAlignmentTitle) rtrn LLC_CXS("ALIGNMENT TITLE");
	if(idControl == app.ControlAlignmentRoot ) rtrn LLC_CXS("ALIGNMENT STAGE");
	for(::llc::u0_t iTheme = 0; iTheme < THEME_COUNT; ++iTheme)
		if(idControl == app.ControlsTheme[iTheme])
			rtrn ::sampleGUIThemeText(iTheme);
	for(::llc::u0_t iState = 0; iState < STATE_COUNT; ++iState)
		if(idControl == app.ControlsState[iState])
			rtrn ::sampleGUIStateText(iState);
	for(::llc::u0_t iAlignment = 0; iAlignment < ALIGNMENT_COUNT; ++iAlignment)
		if(idControl == app.ControlsAlignment[iAlignment])
			rtrn ::sampleGUIAlignmentText(iAlignment);
	rtrn LLC_CXS("CONTROL");
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
	cnst ::llc::GUI_CONTROL_PALETTE palette = ::llc::controlPaletteIndex(disabled, state);
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
		cnst ::llc::GUI_CONTROL_PALETTE palette = ::llc::controlPaletteIndex(disabled, state);
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

sttc ::llc::err_t sampleGUIMetricsUpdate(SSampleGUI & app, bool forceUpdate) {
	cnst ::llc::FGUITextMeasure measure = [&app](::llc::s1_t font, ::llc::vcst_t text, ::llc::n2u1_t & size) { rtrn ::llc::guiMeasureTextRaster(app.Resources, font, text, size); };
	rtrn ::llc::guiUpdateMetrics(app.GUI, app.Window.Size, measure, forceUpdate);
}

::llc::err_t sampleGUISetup(SSampleGUI & app) {
	app.GUI.SelectedFont = 2;
	app.GUI.ColorModeDefault = ::llc::GUI_COLOR_MODE_3D;
	if_fail_fe(::sampleGUIResourcesCreate(app));

	app.ControlTitle = ::sampleGUIControlCreate(app, LLC_CXS("LLC GUI CONTROL PALETTES AND STATES"), {{24, 18}, {650, 42}}, 3);
	if_true_fe(app.ControlTitle < 0);
	app.ControlMode = ::sampleGUIControlCreate(app, LLC_CXS("Mode: 3D"), {{742, 18}, {184, 42}}, 2);
	if_true_fe(app.ControlMode < 0);
	for(::llc::u0_t iTheme = 0; iTheme < THEME_COUNT; ++iTheme) {
		app.ControlsTheme[iTheme] = ::sampleGUIControlCreate(app, ::sampleGUIThemeText(iTheme), {{(::llc::s1_t)(24 + 226 * iTheme), 76}, {206, 40}}, 2);
		if_true_fe(app.ControlsTheme[iTheme] < 0);
	}

	for(::llc::u0_t iState = 0; iState < STATE_COUNT; ++iState) {
		cnst ::llc::s1_t x = (::llc::s1_t)(24 + 226 * (iState % 4));
		cnst ::llc::s1_t y = (::llc::s1_t)(140 + 88 * (iState / 4));
		app.ControlsState[iState] = ::sampleGUIControlCreate(app, ::sampleGUIStateText(iState), {{x, y}, {206, 68}}, 2);
		if_true_fe(app.ControlsState[iState] < 0);
	}
	app.ControlLive = ::sampleGUIControlCreate(app, LLC_CXS("LIVE"), {{24, 338}, {884, 64}}, 2);
	if_true_fe(app.ControlLive < 0);
	app.ControlInspector = ::sampleGUIControlCreate(app, LLC_CXS("GUI | HOVER:NONE"), {{24, 416}, {884, 56}}, 1);
	if_true_fe(app.ControlInspector < 0);
	app.ControlHelp = ::sampleGUIControlCreate(app, LLC_CXS("Each palette derives every state from one 16-shade tone. Mode switches the same geometry between flat and 3D shading. ESC exits."), {{24, 486}, {884, 36}}, 1);
	if_true_fe(app.ControlHelp < 0);
	app.ControlAlignmentTitle = ::sampleGUIControlCreate(app, LLC_CXS("CONTROL ALIGNMENT / OPPOSITE TEXT ALIGNMENT"), {{24, 18}, {332, 42}}, 1);
	if_true_fe(app.ControlAlignmentTitle < 0);
	app.GUI.Controls.Placement[app.ControlAlignmentTitle].Align = ::llc::ALIGN_TOP_RIGHT;
	app.ControlAlignmentRoot = ::sampleGUIControlCreate(app, LLC_CXS(""), {{24, 76}, {332, 446}});
	if_true_fe(app.ControlAlignmentRoot < 0);
	app.GUI.Controls.Placement[app.ControlAlignmentRoot].Align = ::llc::ALIGN_TOP_RIGHT;
	for(::llc::u0_t iAlignment = 0; iAlignment < ALIGNMENT_COUNT; ++iAlignment) {
		app.ControlsAlignment[iAlignment] = ::sampleGUIControlCreate(app, ::sampleGUIAlignmentText(iAlignment), {{0, 0}, {108, 58}}, 1);
		if_true_fe(app.ControlsAlignment[iAlignment] < 0);
		app.GUI.Controls.Placement[app.ControlsAlignment[iAlignment]].Align = ::sampleGUIControlAlignment(iAlignment);
		app.GUI.Controls.Text[app.ControlsAlignment[iAlignment]].Align = ::sampleGUITextAlignment(iAlignment);
		if_fail_fe(::llc::controlSetParent(app.GUI, app.ControlsAlignment[iAlignment], app.ControlAlignmentRoot));
	}

	::sampleGUIThemeApply(app);
	::sampleGUIStateCardsApply(app);
	if_fail_fe(::sampleGUILiveTextUpdate(app));
	if_fail_fe(::sampleGUIInspectorTextUpdate(app, ::llc::CID_INVALID));
	if_fail_fe(app.Target.resize(app.Window.Size));
	rtrn ::sampleGUIMetricsUpdate(app, true);
}

::llc::err_t sampleGUIUpdate(SSampleGUI & app, bool systemRequestedExit) {
	if(systemRequestedExit || app.Window.Closed || app.Window.Input.KeyDown(0x1B))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if(app.Target.metrics() != app.Window.Size.u2()) {
		if_fail_fe(app.Target.resize(app.Window.Size));
		if_fail_fe(::sampleGUIMetricsUpdate(app, true));
	}
	if_fail_fe(::llc::guiProcessInput(app.GUI, app.Window.Input));
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

::llc::err_t sampleGUIDraw(SSampleGUI & app) {
	if_fail_fe(app.Target.View.fill(::llc::SColorBGRA{16, 14, 12}));
	rtrn ::llc::guiDraw(app.GUI, app.Resources, app.Target.View);
}
