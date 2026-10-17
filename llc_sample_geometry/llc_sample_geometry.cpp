#include "llc_sample_geometry.h"

#include "llc_app_impl.h"
#include "llc_apod_serialize.h"
#include "llc_chrono.h"
#include "llc_cstring.h"
#include "llc_framework.h"
#include "llc_runtime.h"
#include "llc_sync.h"
#include "llc_system_key.h"
#include "llc_geometry_lh.h"
#include "llc_raster_lh.h"

stacxpr ::llc::u0_t SAMPLE_GUI_THEME_COUNT = 3;
stacxpr ::llc::u0_t PALETTE_SHADE_COUNT = 16;

sttc ::llc::SColorBGRA sampleGUIThemeColor(::llc::u0_t theme) nxpt {
	stacxpr ::llc::SColorBGRA colors[SAMPLE_GUI_THEME_COUNT] =
		{ {248, 208,  72}
		, { 32, 176, 248}
		, {112, 112, 112}
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

sttc ::llc::err_t sampleGeometryDoubleClick(cnst ::llc::TQueueSystemEvent & events, bool & doubleClick) {
	doubleClick = false;
	for(::llc::u2_t iEvent = 0; iEvent < events.size(); ++iEvent) {
		cnst ::llc::pobj<::llc::SEventSystem> & eventSystem = events[iEvent];
		if(0 == eventSystem.get_ref() || eventSystem->Type != ::llc::SYSTEM_EVENT_Mouse)
			continue;
		::llc::SEView<::llc::EVENT_MOUSE> eventMouse = {};
		if_fail_fe(eventSystem->ExtractChild(eventMouse));
		if(eventMouse.Type != ::llc::EVENT_MOUSE_DoubleClick)
			continue;
		::llc::vcu0_t input = eventMouse.Data;
		::llc::u0_t button = 0;
		if_fail_fe(::llc::loadPOD(input, button));
		if_true_fef(input.size(), "Unexpected mouse double-click payload tail:%u.", input.size());
		if(0 == button)
			doubleClick = true;
	}
	rtrn 0;
}

sttc bool sampleGeometryPressed(cnst SSampleGeometry & app, cnst ::llc::SViewport & viewport) nxpt {
	rtrn app.GUI.Controls.IsPressed(viewport.IdControl)
		|| app.GUI.Controls.IsPressed(viewport.IdControls[::llc::VIEWPORT_CONTROL_TITLE])
		|| app.GUI.Controls.IsPressed(viewport.IdControls[::llc::VIEWPORT_CONTROL_TARGET])
		;
}

sttc ::llc::err_t sampleGeometryActivate(SSampleGeometry & app, ::llc::u0_t iViewport) {
	if_true_fe(iViewport >= app.Viewports.size());
	cnst ::llc::cid_t idRoot = app.Viewports[iViewport].IdControl;
	::llc::acid & desktopChildren = app.GUI.Controls.Children[app.Desktop];
	if(0 == desktopChildren.size() || desktopChildren[desktopChildren.size() - 1] != idRoot) {
		cnst ::llc::err_t iChild = desktopChildren.find(idRoot);
		if(iChild >= 0)
			if_fail_fe(desktopChildren.remove((::llc::u2_t)iChild));
		if_fail_fe(desktopChildren.push_back(idRoot));
	}
	for(::llc::u0_t iOther = 0; iOther < app.Viewports.size(); ++iOther) {
		cnst bool active = iOther == iViewport;
		app.GUI.Controls.SetSelected(app.Viewports[iOther].IdControl, active);
		app.GUI.Controls.SetSelected(app.Viewports[iOther].IdControls[::llc::VIEWPORT_CONTROL_TITLE], active);
	}
	rtrn 0;
}

sttc ::llc::n2s1_t sampleGeometryProject(cnst ::llc::n3f2_t & source, cnst ::llc::n3f2_t & center, ::llc::f2_t radius, ::llc::n2u2_t metrics, ::llc::f2_t angle) nxpt {
	cnst ::llc::f2_t sinY = (::llc::f2_t)::sin(angle);
	cnst ::llc::f2_t cosY = (::llc::f2_t)::cos(angle);
	cnst ::llc::f2_t sinX = (::llc::f2_t)::sin(angle * .63f);
	cnst ::llc::f2_t cosX = (::llc::f2_t)::cos(angle * .63f);
	cnst ::llc::n3f2_t centered = source - center;
	cnst ::llc::f2_t x = centered.x * cosY - centered.z * sinY;
	cnst ::llc::f2_t z = centered.x * sinY + centered.z * cosY;
	cnst ::llc::f2_t y = centered.y * cosX - z * sinX;
	cnst ::llc::f2_t scale = (::llc::f2_t)::llc::min(metrics.x, metrics.y) * .42f / radius;
	rtrn {(::llc::s1_t)(metrics.x * .5f + x * scale), (::llc::s1_t)(metrics.y * .5f - y * scale)};
}

sttc void sampleGeometryImageUpdate(::llc::img8bgra & image, cnst ::llc::SGeometryBuffers & geometry, ::llc::u0_t theme, ::llc::f2_t angle) nxpt {
	cnst ::llc::SColorBGRA tone = ::sampleGUIThemeColor(theme);
	cnst ::llc::bgra background = ::sampleGUIShade(tone, 14);
	cnst ::llc::bgra wire = ::sampleGUIShade(tone, 1);
	image.View.fill(background);
	if(0 == geometry.Positions.size())
		rtrn;
	::llc::n3f2_t minimum = geometry.Positions[0];
	::llc::n3f2_t maximum = geometry.Positions[0];
	for(cnst ::llc::n3f2_t & position : geometry.Positions) {
		minimum.x = ::llc::min(minimum.x, position.x);
		minimum.y = ::llc::min(minimum.y, position.y);
		minimum.z = ::llc::min(minimum.z, position.z);
		maximum.x = ::llc::max(maximum.x, position.x);
		maximum.y = ::llc::max(maximum.y, position.y);
		maximum.z = ::llc::max(maximum.z, position.z);
	}
	cnst ::llc::n3f2_t center = (minimum + maximum) * .5f;
	::llc::f2_t radius = 0;
	for(cnst ::llc::n3f2_t & position : geometry.Positions)
		radius = ::llc::max(radius, (::llc::f2_t)(position - center).Length());
	radius = ::llc::max(radius, .001f);
	for(::llc::u2_t iIndex = 0; iIndex + 2 < geometry.PositionIndices.size(); iIndex += 3) {
		::llc::n2s1_t projected[3] = {};
		for(::llc::u0_t iVertex = 0; iVertex < 3; ++iVertex) {
			cnst ::llc::u2_t index = geometry.PositionIndices[iIndex + iVertex];
			if(index >= geometry.Positions.size())
				continue;
			projected[iVertex] = ::sampleGeometryProject(geometry.Positions[index], center, radius, image.metrics(), angle);
		}
		::llc::drawLine(image.View, {projected[0], projected[1]}, wire);
		::llc::drawLine(image.View, {projected[1], projected[2]}, wire);
		::llc::drawLine(image.View, {projected[2], projected[0]}, wire);
	}
}

sttc ::llc::err_t sampleGeometryCreate(SSampleGeometry & app, ::llc::u0_t iViewport, ::llc::vcst_t title, cnst ::llc::rect2s1_t & area) {
	if_true_fe(iViewport >= app.Viewports.size());
	::llc::SViewport & viewport = app.Viewports[iViewport];
	if_fail_fe(::llc::viewportInitialize(app.GUI, viewport));
	if_fail_fe(::llc::controlSetParent(app.GUI, viewport.IdControl, app.Desktop));
	::llc::SControlPlacement & root = app.GUI.Controls.Placement[viewport.IdControl];
	root.Area = area;
	root.Border = {3, 3, 3, 3};
	root.Margin = {1, 1, 1, 1};
	cnst ::llc::cid_t idTitle = viewport.IdControls[::llc::VIEWPORT_CONTROL_TITLE];
	cnst ::llc::cid_t idClient = viewport.IdControls[::llc::VIEWPORT_CONTROL_TARGET];
	app.GUI.Controls.Text[idTitle].Text = title;
	app.GUI.Controls.Placement[idTitle].Border = {2, 2, 2, 2};
	app.GUI.Controls.Placement[idClient].Area.Size = {(::llc::s1_t)(area.Size.x - 8), (::llc::s1_t)(area.Size.y - 32)};
	app.GUI.Controls.Placement[idClient].Border = {1, 1, 1, 1};
	::sampleGUIControlThemeApply(app.GUI, viewport.IdControl, iViewport);
	for(::llc::u0_t iControl = 0; iControl < viewport.IdControls.size(); ++iControl)
		::sampleGUIControlThemeApply(app.GUI, viewport.IdControls[iControl], iViewport);
	app.UnfoldedSize[iViewport] = area.Size;
	rtrn ::llc::controlMetricsInvalidate(app.GUI, viewport.IdControl);
}

sttc ::llc::err_t sampleGeometryAppend(::llc::SGeometryBuffers & target, cnst ::llc::SGeometryQuads & source) {
	for(cnst ::llc::tri3f2_t & triangle : source.Triangles) {
		cnst ::llc::u2_t baseVertex = target.Positions.size();
		llc_necs(target.Positions.append({triangle.A, triangle.B, triangle.C}));
		llc_necs(target.PositionIndices.append({baseVertex, baseVertex + 1, baseVertex + 2}));
	}
	rtrn 0;
}

sttc ::llc::err_t sampleGeometryFold(SSampleGeometry & app, ::llc::u0_t iViewport) {
	if_true_fe(iViewport >= app.Viewports.size());
	::llc::SViewport & viewport = app.Viewports[iViewport];
	::llc::SControlPlacement & root = app.GUI.Controls.Placement[viewport.IdControl];
	cnst ::llc::cid_t idClient = viewport.IdControls[::llc::VIEWPORT_CONTROL_TARGET];
	if(app.Folded[iViewport]) {
		root.Area.Size = app.UnfoldedSize[iViewport];
		app.GUI.Controls.SetHidden(idClient, false);
	}
	else {
		app.UnfoldedSize[iViewport] = root.Area.Size;
		root.Area.Size.y = 30;
		app.GUI.Controls.SetHidden(idClient, true);
	}
	app.Folded[iViewport] = false == app.Folded[iViewport];
	rtrn ::llc::controlMetricsInvalidate(app.GUI, viewport.IdControl);
}

sttc ::llc::err_t sampleGeometryMaximize(SSampleGeometry & app, ::llc::u0_t iViewport) {
	if_true_fe(iViewport >= app.Viewports.size());
	cnst bool restore = app.MaximizedViewport == (::llc::s1_t)iViewport;
	if(app.MaximizedViewport >= 0) {
		cnst ::llc::u0_t iMaximized = (::llc::u0_t)app.MaximizedViewport;
		::llc::SViewport & maximized = app.Viewports[iMaximized];
		app.GUI.Controls.Placement[maximized.IdControl].Area = app.RestoreArea[iMaximized];
		app.GUI.Controls.Placement[maximized.IdControls[::llc::VIEWPORT_CONTROL_TARGET]].Area.Size = app.RestoreArea[iMaximized].Size - ::llc::n2s1_t{8, 32};
	}
	for(::llc::u0_t iOther = 0; iOther < app.Viewports.size(); ++iOther)
		app.GUI.Controls.SetHidden(app.Viewports[iOther].IdControl, false);
	app.MaximizedViewport = -1;
	if(false == restore) {
		if(app.Folded[iViewport])
			if_fail_fe(::sampleGeometryFold(app, iViewport));
		::llc::SViewport & viewport = app.Viewports[iViewport];
		app.RestoreArea[iViewport] = app.GUI.Controls.Placement[viewport.IdControl].Area;
		for(::llc::u0_t iOther = 0; iOther < app.Viewports.size(); ++iOther)
			if(iOther != iViewport)
				app.GUI.Controls.SetHidden(app.Viewports[iOther].IdControl, true);
		::llc::rect2s1_t & area = app.GUI.Controls.Placement[viewport.IdControl].Area;
		area = {{18, 76}, {1564, 800}};
		app.GUI.Controls.Placement[viewport.IdControls[::llc::VIEWPORT_CONTROL_TARGET]].Area.Size = area.Size - ::llc::n2s1_t{8, 32};
		app.MaximizedViewport = (::llc::s1_t)iViewport;
	}
	rtrn ::llc::controlMetricsInvalidate(app.GUI, app.Desktop);
}

sttc ::llc::err_t sampleGeometryPerformanceUpdate(SSampleGeometry & app, double secondsLastFrame, double secondsUpdate, double secondsDraw) {
	if(secondsLastFrame <= 0)
		rtrn 0;
	cnst ::llc::err_t textLength = ::llc::sprintf_s(app.PerformanceText.Storage, "FPS %.2f   Frame %.2f ms   Update %.2f ms   Draw %.2f ms", 1.0 / secondsLastFrame, secondsLastFrame * 1000.0, secondsUpdate * 1000.0, secondsDraw * 1000.0);
	if_fail_fe(textLength);
	app.GUI.Controls.Text[app.Performance].Text = {app.PerformanceText.Storage, (::llc::u2_t)textLength};
	app.GUI.Controls.SetUpdated(app.Performance, false);
	rtrn 0;
}

sttc ::llc::err_t sampleGeometryScaleSet(SSampleGeometry & app, double value) {
	SSampleGUIScale & slider = app.GUIScale;
	slider.Value = ::llc::max(.5, ::llc::min(2.0, value));
	app.GUI.Zoom.DPI = {slider.Value, slider.Value};
	::llc::SControlPlacement & root = app.GUI.Controls.Placement[slider.IdRoot];
	::llc::SControlPlacement & thumb = app.GUI.Controls.Placement[slider.IdThumb];
	cnst ::llc::n2u1_t ncSpacing = ::llc::controlNCSpacing(root);
	cnst ::llc::s2_t trackLength = ::llc::max((::llc::s2_t)0, (::llc::s2_t)(root.Area.Size.x - ncSpacing.x - thumb.Area.Size.x));
	thumb.Area.Offset.x = (::llc::s1_t)(trackLength * ((slider.Value - .5) / 1.5));
	cnst ::llc::err_t textLength = ::llc::sprintf_s(slider.Text.Storage, "%.0f%%", slider.Value * 100.0);
	if_fail_fe(textLength);
	if_fail_fe(::llc::controlTextSet(app.GUI, slider.IdThumb, {slider.Text.Storage, (::llc::u2_t)textLength}));
	rtrn ::llc::controlMetricsInvalidate(app.GUI, app.Desktop);
}

sttc ::llc::err_t sampleGeometryScaleCreate(SSampleGeometry & app) {
	SSampleGUIScale & slider = app.GUIScale;
	slider.IdRoot = ::llc::controlCreate(app.GUI);
	if_true_fe(slider.IdRoot < 0);
	slider.IdThumb = ::llc::controlCreateChild(app.GUI, slider.IdRoot);
	if_true_fe(slider.IdThumb < 0);
	::llc::SControlPlacement & root = app.GUI.Controls.Placement[slider.IdRoot];
	root.Area = {{18, 14}, {320, 36}};
	root.Align = ::llc::ALIGN_BOTTOM_LEFT;
	root.Border = {2, 2, 2, 2};
	root.Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[slider.IdRoot].Text = LLC_CXS("GUI DPI");
	app.GUI.Controls.Text[slider.IdRoot].Align = ::llc::ALIGN_CENTER;
	::sampleGUIControlThemeApply(app.GUI, slider.IdRoot, 0);
	::llc::SControlPlacement & thumb = app.GUI.Controls.Placement[slider.IdThumb];
	thumb.Area.Size = {56, 28};
	thumb.Border = {1, 1, 1, 1};
	app.GUI.Controls.Text[slider.IdThumb].Align = ::llc::ALIGN_CENTER;
	app.GUI.Controls.Constraints[slider.IdThumb].AttachSizeToControl.y = slider.IdThumb;
	::sampleGUIControlThemeApply(app.GUI, slider.IdThumb, 2);
	rtrn ::sampleGeometryScaleSet(app, 1.0);
}

sttc ::llc::err_t sampleGeometryScaleUpdate(SSampleGeometry & app) {
	SSampleGUIScale & slider = app.GUIScale;
	double value = slider.Value;
	cnst bool focused = app.GUI.Controls.IsFocused(slider.IdRoot) || app.GUI.Controls.IsFocused(slider.IdThumb);
	cnst bool hovered = app.GUI.Controls.IsHovered(slider.IdRoot) || app.GUI.Controls.IsHovered(slider.IdThumb);
	if(focused) {
		if(app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Home)) value = .5;
		else if(app.Window.Input->KeyDown(::llc::SYSTEM_KEY_End)) value = 2.0;
		else if(app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Right)) value += .01;
		else if(app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Left)) value -= .01;
	}
	if((focused || hovered) && app.Window.Input->MouseCurrent.Deltas.z)
		value += 0 < app.Window.Input->MouseCurrent.Deltas.z ? .05 : -.05;
	if(app.GUI.Controls.IsPressed(slider.IdThumb) && app.Window.Input->MouseCurrent.Deltas.x)
		value += app.Window.Input->MouseCurrent.Deltas.x * .005;
	else if(app.Window.Input->ButtonDown(0) && app.GUI.Controls.IsPressed(slider.IdRoot)) {
		cnst ::llc::SControlArea & thumbArea = app.GUI.Controls.Area[slider.IdThumb];
		value += app.Window.Input->MouseCurrent.Position.x < thumbArea.Total.Global.Offset.x + thumbArea.Total.Global.Size.x / 2 ? -.05 : .05;
	}
	rtrn value != slider.Value ? ::sampleGeometryScaleSet(app, value) : 0;
}

::llc::err_t sampleGeometrySetup(SSampleGeometry & app) {
	app.GUI.SelectedFont = 8;
	app.GUI.ColorModeDefault = ::llc::GUI_COLOR_MODE_3D;
	if_fail_fe(::sampleGUIResourcesCreate(app.GUI));
	app.Desktop = ::llc::controlCreate(app.GUI);
	if_true_fe(app.Desktop < 0);
	app.GUI.Controls.Draw[app.Desktop].NoBorder = true;
	app.GUI.Controls.Draw[app.Desktop].NoClient = true;
	app.GUI.Controls.Constraints[app.Desktop].AttachSizeToControl = {app.Desktop, app.Desktop};
	::sampleGUIControlThemeApply(app.GUI, app.Desktop, 2);
	app.Help = ::llc::controlCreateChild(app.GUI, app.Desktop);
	if_true_fe(app.Help < 0);
	app.GUI.Controls.Placement[app.Help].Area = {{18, 18}, {1500, 42}};
	app.GUI.Controls.Placement[app.Help].Border = {2, 2, 2, 2};
	app.GUI.Controls.Placement[app.Help].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[app.Help].Text = LLC_CXS("DRAG ANY TITLE | DOUBLE-CLICK MAXIMIZES | SHIFT+DOUBLE-CLICK FOLDS | TAB MOVES FOCUS | ESC OR EXIT CLOSES");
	app.GUI.Controls.Text[app.Help].Align = ::llc::ALIGN_CENTER;
	::sampleGUIControlThemeApply(app.GUI, app.Help, 2);
	stacxpr ::llc::vcst_t titles[SAMPLE_GUI_VIEWPORT_COUNT] =
		{ LLC_CXS("BUFFER BOX")
		, LLC_CXS("BUFFER SPHERE")
		, LLC_CXS("BUFFER DISC")
		, LLC_CXS("BUFFER CYLINDER")
		, LLC_CXS("BUFFER GRID")
		, LLC_CXS("BUFFER RING FLAT")
		, LLC_CXS("BUFFER TUBE")
		, LLC_CXS("BUFFER HELIX")
		, LLC_CXS("LH CUBE")
		, LLC_CXS("LH SPHERE")
		, LLC_CXS("LH CYLINDER")
		, LLC_CXS("LH HELIX HALF")
		, LLC_CXS("LH HELIX")
		, LLC_CXS("LH TENDER")
		, LLC_CXS("LH FIGURE 0")
		, LLC_CXS("LH GRID")
		};
	for(::llc::u0_t iViewport = 0; iViewport < SAMPLE_GUI_VIEWPORT_COUNT; ++iViewport) {
		cnst ::llc::s1_t column = iViewport % 4;
		cnst ::llc::s1_t row = iViewport / 4;
		if_fail_fe(::sampleGeometryCreate(app, iViewport, titles[iViewport], {{(::llc::s1_t)(20 + column * 382), (::llc::s1_t)(76 + row * 202)}, {362, 184}}));
	}
	::llc::SParamsBox			paramsBox		= {};
	::llc::SParamsSphere		paramsSphere	= {};
	::llc::SParamsDisc			paramsDisc		= {};
	::llc::SParamsCylinderWall	paramsCylinder	= {};
	::llc::SParamsGrid			paramsGrid		= {};
	::llc::SParamsRingSide		paramsRing		= {};
	::llc::SParamsTube			paramsTube		= {};
	paramsSphere.CellCount	= {16, 12};
	paramsDisc.Slices		= 24;
	paramsCylinder.CellCount= {20, 2};
	paramsGrid.CellCount	= {8, 8};
	paramsRing.Slices		= 24;
	paramsTube.CellCount	= {20, 8};
	paramsTube.RadiusYMin	= {.25f, .5f};
	paramsTube.RadiusYMax	= {.25f, .5f};
	if_fail_fe(::llc::geometryBuildBox(app.Geometry[0], paramsBox));
	if_fail_fe(::llc::geometryBuildSphere(app.Geometry[1], paramsSphere));
	if_fail_fe(::llc::geometryBuildDisc(app.Geometry[2], paramsDisc));
	if_fail_fe(::llc::geometryBuildCylinder(app.Geometry[3], paramsCylinder));
	if_fail_fe(::llc::geometryBuildGrid(app.Geometry[4], paramsGrid));
	if_fail_fe(::llc::geometryBuildRingFlat(app.Geometry[5], paramsRing));
	if_fail_fe(::llc::geometryBuildRing(app.Geometry[6], paramsTube));
	::llc::SParamsHelix paramsHelix = {};
	paramsHelix.CellCount = {18, 8};
	if_fail_fe(::llc::geometryBuildHelix(app.Geometry[7], paramsHelix));
	::llc::SGeometryQuads geometryLH[8] = {};
	if_fail_fe(::llc::geometryBuildCube(geometryLH[0], {1, 1, 1}));
	if_fail_fe(::llc::geometryBuildSphere(geometryLH[1], 8U, 8U, .5f, {}));
	if_fail_fe(::llc::geometryBuildCylinder(geometryLH[2], 1U, 8U, .35f, .25f, {0, 1, 0}, {1, 1, 1}));
	if_fail_fe(::llc::geometryBuildHelixHalf(geometryLH[3], 8U, 8U, .15f, {0, 1.1f, 0}, {1, 1, 1}));
	if_fail_fe(::llc::geometryBuildHelix(geometryLH[4], 8U, 8U, .15f, {0, 1.1f, 0}, {1, 1, 1}));
	if_fail_fe(::llc::geometryBuildTender(geometryLH[5], 8U, 8U, .15f, {0, 1.1f, 0}, {1, 1, 1}));
	if_fail_fe(::llc::geometryBuildFigure0(geometryLH[6], 8U, 8U, .15f, {0, 1.1f, 0}));
	if_fail_fe(::llc::geometryBuildGrid(geometryLH[7], {8, 8}, {4, 4}, {1, 1, 1}));
	for(::llc::u0_t iGeometry = 0; iGeometry < 8; ++iGeometry)
		if_fail_fe(::sampleGeometryAppend(app.Geometry[8 + iGeometry], geometryLH[iGeometry]));
	if_fail_fe(::sampleGeometryActivate(app, 2));
	app.Performance = ::llc::controlCreate(app.GUI);
	if_true_fe(app.Performance < 0);
	app.GUI.Controls.Placement[app.Performance].Area = {{0, 14}, {640, 36}};
	app.GUI.Controls.Placement[app.Performance].Align = ::llc::ALIGN_CENTER_BOTTOM;
	app.GUI.Controls.Placement[app.Performance].Border = {1, 1, 1, 1};
	app.GUI.Controls.Placement[app.Performance].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[app.Performance].Text = LLC_CXS("FPS --    Frame -- ms");
	::sampleGUIControlThemeApply(app.GUI, app.Performance, 2);
	if_fail_fe(::sampleGeometryScaleCreate(app));
	app.Exit = ::llc::controlCreate(app.GUI);
	if_true_fe(app.Exit < 0);
	app.GUI.Controls.Placement[app.Exit].Area = {{18, 18}, {96, 36}};
	app.GUI.Controls.Placement[app.Exit].Align = ::llc::ALIGN_BOTTOM_RIGHT;
	app.GUI.Controls.Placement[app.Exit].Border = {2, 2, 2, 2};
	app.GUI.Controls.Placement[app.Exit].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[app.Exit].Text = LLC_CXS("EXIT");
	::sampleGUIControlThemeApply(app.GUI, app.Exit, 1);
	rtrn 0;
}

::llc::err_t sampleGeometryUpdate(SSampleGeometry & app, bool systemRequestedExit, double secondsLastFrame, double secondsUpdate, double secondsDraw) {
	if(systemRequestedExit || app.Window.Closed || app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Escape))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if(app.GUI.Controls.IsAction(app.Exit))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if_fail_fe(::sampleGeometryScaleUpdate(app));
	if_fail_fe(::sampleGeometryPerformanceUpdate(app, secondsLastFrame, secondsUpdate, secondsDraw));
	app.Angle += (::llc::f2_t)(secondsLastFrame * .75);
	bool doubleClick = false;
	if_fail_fe(::sampleGeometryDoubleClick(app.Window.EventQueue, doubleClick));
	::llc::u2_t hoveredViewport = app.Viewports.size();
	for(::llc::u0_t iViewport = 0; iViewport < app.Viewports.size(); ++iViewport) {
		::llc::SViewport & viewport = app.Viewports[iViewport];
		cnst ::llc::cid_t idTitle = viewport.IdControls[::llc::VIEWPORT_CONTROL_TITLE];
		cnst ::llc::cid_t idClose = viewport.IdControls[::llc::VIEWPORT_CONTROL_CLOSE];
		cnst ::llc::cid_t idResize = viewport.IdControls[::llc::VIEWPORT_CONTROL_RESIZE_BOTTOM_RIGHT];
		if(::sampleGeometryPressed(app, viewport))
			if_fail_fe(::sampleGeometryActivate(app, iViewport));
		if(app.GUI.Controls.IsAction(idClose)) {
			if(app.MaximizedViewport == (::llc::s1_t)iViewport)
				if_fail_fe(::sampleGeometryMaximize(app, iViewport));
			app.GUI.Controls.SetHidden(viewport.IdControl, true);
		}
		if(app.MaximizedViewport < 0 && app.GUI.Controls.IsPressed(idTitle) && (app.Window.Input->MouseCurrent.Deltas.x || app.Window.Input->MouseCurrent.Deltas.y)) {
			cnst ::llc::n2s1_t delta = {(::llc::s1_t)app.Window.Input->MouseCurrent.Deltas.x, (::llc::s1_t)app.Window.Input->MouseCurrent.Deltas.y};
			app.GUI.Controls.Placement[viewport.IdControl].Area.Offset += delta;
			if_fail_fe(::llc::controlMetricsInvalidate(app.GUI, viewport.IdControl));
		}
		if(app.MaximizedViewport < 0 && app.GUI.Controls.IsPressed(idResize) && (app.Window.Input->MouseCurrent.Deltas.x || app.Window.Input->MouseCurrent.Deltas.y)) {
			cnst ::llc::n2s1_t delta = {(::llc::s1_t)app.Window.Input->MouseCurrent.Deltas.x, (::llc::s1_t)app.Window.Input->MouseCurrent.Deltas.y};
			app.GUI.Controls.Placement[viewport.IdControl].Area.Size += delta;
			app.GUI.Controls.Placement[viewport.IdControls[::llc::VIEWPORT_CONTROL_TARGET]].Area.Size += delta;
			app.UnfoldedSize[iViewport] = app.GUI.Controls.Placement[viewport.IdControl].Area.Size;
			if_fail_fe(::llc::controlMetricsInvalidate(app.GUI, viewport.IdControl));
		}
		if(app.GUI.Controls.IsHovered(idTitle))
			hoveredViewport = iViewport;
	}
	if(doubleClick && hoveredViewport < app.Viewports.size()) {
		if(app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Shift)) {
			if_fail_fe(::sampleGeometryFold(app, (::llc::u0_t)hoveredViewport));
		}
		else {
			if_fail_fe(::sampleGeometryMaximize(app, (::llc::u0_t)hoveredViewport));
		}
	}
	rtrn 0;
}

::llc::err_t sampleGeometryDraw(SSampleGeometry & app, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::g8bgra target) {
	if_fail_fe(target.fill(::llc::SColorBGRA{16, 14, 12}));
	for(::llc::u0_t iViewport = 0; iViewport < SAMPLE_GUI_VIEWPORT_COUNT; ++iViewport) {
		cnst ::llc::cid_t idClient = app.Viewports[iViewport].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
		cnst ::llc::n2u2_t imageSize = app.GUI.Controls.Area[idClient].Client.Global.Size.u2();
		if(0 == imageSize.x || 0 == imageSize.y)
			continue;
		if(clientImages[iViewport].metrics() != imageSize)
			if_fail_fe(clientImages[iViewport].resize(imageSize));
		::sampleGeometryImageUpdate(clientImages[iViewport], app.Geometry[iViewport], iViewport, app.Angle + iViewport * .17f);
		app.GUI.Controls.Images[idClient].Image = clientImages[iViewport].View;
		app.GUI.Controls.Images[idClient].ImageAlign = ::llc::ALIGN_CENTER;
	}
	rtrn ::llc::guiDraw(app.GUI, target);
}

#define LLC_SAMPLE_GEOMETRY_TITLE "LLC geometry wireframe"

stct SApplication {
	::llc::SFramework	Framework;
	SSampleGeometry		Sample;
	::llc::img8bgra	ClientImages[SAMPLE_GUI_VIEWPORT_COUNT] = {};
	vltl ::llc::refcount_t LastUpdateMicroseconds = 0;
	vltl ::llc::refcount_t LastDrawMicroseconds = 0;

	SApplication(::llc::SRuntimeValues & runtimeValues)
		: Framework(runtimeValues, {}), Sample(Framework.RootWindow, *Framework.GUI) {}
};

::llc::err_t setup(SApplication & app) {
	::llc::SWindow & window = app.Framework.RootWindow;
	window.Size = {1600, 960};
	if_null_ve(::llc::OS_NO_MEMORY, window.BackBuffer.create());
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	if_fail_fe(::llc::mainWindowCreate(window, app.Framework.RuntimeValues.PlatformDetail, window.Input));
	rtrn ::sampleGeometrySetup(app.Sample);
}

::llc::err_t cleanup(SApplication & app) {
	rtrn ::llc::mainWindowDestroy(app.Framework.RootWindow);
}

::llc::err_t update(SApplication & app, bool systemRequestedExit) {
	if(systemRequestedExit)
		rtrn ::llc::APPLICATION_STATE_EXIT;
	cnst ::llc::s3_t timeStart = ::llc::timeCurrentInUs();
	cnst ::llc::err_t result = ::llc::updateFramework(app.Framework);
	if(result)
		rtrn result;
	cnst double secondsUpdate = llc_sync_compare_exchange(app.LastUpdateMicroseconds, 0, 0) * .000001;
	cnst double secondsDraw = llc_sync_compare_exchange(app.LastDrawMicroseconds, 0, 0) * .000001;
	cnst ::llc::err_t resultSample = ::sampleGeometryUpdate(app.Sample, false, app.Framework.FrameInfo.Seconds.LastFrame, secondsUpdate, secondsDraw);
	llc_sync_exchange(app.LastUpdateMicroseconds, (::llc::refcount_t)(::llc::timeCurrentInUs() - timeStart));
	::llc::sleep(1);
	rtrn resultSample;
}

::llc::err_t draw(SApplication & app) {
	::llc::SWindow & window = app.Framework.RootWindow;
	if(window.NoDraw) {
		::llc::sleep(8);
		rtrn 0;
	}
	cnst ::llc::s3_t timeStart = ::llc::timeCurrentInUs();
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	cnst ::llc::err_t result = ::sampleGeometryDraw(app.Sample, app.ClientImages, window.BackBuffer->Color.View);
	llc_sync_exchange(app.LastDrawMicroseconds, (::llc::refcount_t)(::llc::timeCurrentInUs() - timeStart));
	rtrn result;
}

LLC_DEFINE_APPLICATION_ENTRY_POINT(SApplication, LLC_SAMPLE_GEOMETRY_TITLE)
