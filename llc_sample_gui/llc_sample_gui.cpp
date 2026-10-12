#include "llc_sample_gui.h"
#include "llc_runtime.h"

stacxpr ::llc::u0_t THEME_COUNT = 3;
stacxpr ::llc::u0_t STATE_COUNT = 8;

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

sttc ::llc::SColorBGRA sampleGUIBaseColor(::llc::u0_t theme, ::llc::u0_t state) nxpt {
	stacxpr ::llc::SColorBGRA colors[THEME_COUNT][::llc::GUI_CONTROL_PALETTE_COUNT] =
		{ { { 72,  54,  34}, { 54,  54,  54}, {112,  82,  36}, { 42,  96, 150}, { 48, 110,  42}, { 58,  70,  58}, { 72, 154,  58}, { 36, 100,  32}, {130,  38, 144}, { 32, 100, 170} }
		, { { 28,  70, 120}, { 50,  50,  50}, { 32, 112, 186}, { 24,  58, 112}, { 34,  96, 150}, { 54,  62,  70}, { 40, 132, 210}, { 24,  72, 124}, {120,  34, 170}, { 24,  76, 122} }
		, { { 58,  58,  58}, { 42,  42,  42}, { 92,  92,  92}, { 34,  34,  34}, {116, 116, 116}, { 56,  56,  56}, {154, 154, 154}, { 80,  80,  80}, {190, 190, 190}, { 24,  24,  24} }
		};
	rtrn colors[theme % THEME_COUNT][state % ::llc::GUI_CONTROL_PALETTE_COUNT];
}

sttc ::llc::err_t sampleGUIResourcesCreate(SSampleGUI & app) {
	if_true_fe(0 == app.Resources.Colors.create());
	if_fail_fe(app.Resources.Colors->Palettes.resize(THEME_COUNT * ::llc::GUI_CONTROL_PALETTE_COUNT));
	for(::llc::u0_t iTheme = 0; iTheme < THEME_COUNT; ++iTheme)
	for(::llc::u0_t iState = 0; iState < ::llc::GUI_CONTROL_PALETTE_COUNT; ++iState) {
		::llc::astatic<::llc::bgra, ::llc::UI_CONTROL_AREA_COUNT> & palette = app.Resources.Colors->Palettes[iTheme * ::llc::GUI_CONTROL_PALETTE_COUNT + iState];
		cnst ::llc::SColorBGRA base = ::sampleGUIBaseColor(iTheme, iState);
		for(::llc::u2_t iArea = 0; iArea < palette.size(); ++iArea)
			palette[iArea] = base * 0.35;
		palette[::llc::UI_CONTROL_AREA_BACKGROUND] = base * 0.18;
		palette[::llc::UI_CONTROL_AREA_CLIENT] = base;
		palette[::llc::UI_CONTROL_AREA_BORDER_LEFT] = base * 1.20;
		palette[::llc::UI_CONTROL_AREA_BORDER_TOP] = base * 1.20;
		palette[::llc::UI_CONTROL_AREA_BORDER_RIGHT] = base * 0.70;
		palette[::llc::UI_CONTROL_AREA_BORDER_BOTTOM] = base * 0.70;
		palette[::llc::UI_CONTROL_AREA_TEXT_BACKGROUND] = base * 0.45;
		palette[::llc::UI_CONTROL_AREA_TEXT_FACE] = iTheme == 2 && iState >= 4 ? ::llc::SColorBGRA{20, 20, 20} : ::llc::SColorBGRA{242, 246, 250};
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

sttc ::llc::err_t sampleGUIMetricsUpdate(SSampleGUI & app, bool forceUpdate) {
	cnst ::llc::FGUITextMeasure measure = [&app](::llc::s1_t font, ::llc::vcst_t text, ::llc::n2u1_t & size) { rtrn ::llc::guiMeasureTextRaster(app.Resources, font, text, size); };
	rtrn ::llc::guiUpdateMetrics(app.GUI, app.Window.Size, measure, forceUpdate);
}

::llc::err_t sampleGUISetup(SSampleGUI & app) {
	app.GUI.SelectedFont = 2;
	app.GUI.ColorModeDefault = ::llc::GUI_COLOR_MODE_3D;
	if_fail_fe(::sampleGUIResourcesCreate(app));

	cnst ::llc::cid_t idTitle = ::sampleGUIControlCreate(app, LLC_CXS("LLC GUI CONTROL PALETTES AND STATES"), {{24, 18}, {650, 42}}, 3);
	if_true_fe(idTitle < 0);
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
	app.ControlLive = ::sampleGUIControlCreate(app, LLC_CXS("LIVE CONTROL - HOVER, HOLD, RELEASE; CLICK TO TOGGLE SELECTED"), {{24, 338}, {884, 96}}, 2);
	if_true_fe(app.ControlLive < 0);
	cnst ::llc::cid_t idHelp = ::sampleGUIControlCreate(app, LLC_CXS("Palette buttons recolor every control. Mode switches the same geometry between flat and 3D shading. ESC exits."), {{24, 458}, {884, 48}}, 2);
	if_true_fe(idHelp < 0);

	::sampleGUIThemeApply(app);
	::sampleGUIStateCardsApply(app);
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
	if_fail_fe(::llc::controlTextSet(app.GUI, app.ControlMode, app.GUI.ColorModeDefault == ::llc::GUI_COLOR_MODE_3D ? LLC_CXS("Mode: 3D") : LLC_CXS("Mode: Flat")));
	::sampleGUIThemeApply(app);
	::sampleGUIStateCardsApply(app);
	rtrn 0;
}

::llc::err_t sampleGUIDraw(SSampleGUI & app) {
	if_fail_fe(app.Target.View.fill(::llc::SColorBGRA{16, 14, 12}));
	rtrn ::llc::guiDraw(app.GUI, app.Resources, app.Target.View);
}
