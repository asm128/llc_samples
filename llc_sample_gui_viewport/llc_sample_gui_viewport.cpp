#include "llc_sample_gui_viewport.h"

#include "llc_sample_gui_resources.h"
#include "llc_apod_serialize.h"
#include "llc_cstring.h"
#include "llc_runtime.h"
#include "llc_system_key.h"

sttc ::llc::err_t sampleGUIViewportDoubleClick(cnst ::llc::TQueueSystemEvent & events, bool & doubleClick) {
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

sttc bool sampleGUIViewportPressed(cnst SSampleGUIViewport & app, cnst ::llc::SViewport & viewport) nxpt {
	rtrn app.GUI.Controls.IsPressed(viewport.IdRoot)
		|| app.GUI.Controls.IsPressed(viewport.IdTitle)
		|| app.GUI.Controls.IsPressed(viewport.IdClient)
		;
}

sttc ::llc::err_t sampleGUIViewportActivate(SSampleGUIViewport & app, ::llc::u0_t iViewport) {
	if_true_fe(iViewport >= app.Viewports.size());
	cnst ::llc::cid_t idRoot = app.Viewports[iViewport].IdRoot;
	::llc::acid & desktopChildren = app.GUI.Controls.Children[app.Desktop];
	if(0 == desktopChildren.size() || desktopChildren[desktopChildren.size() - 1] != idRoot) {
		if_fail_fe(::llc::controlSetParent(app.GUI, idRoot, ::llc::CID_INVALID));
		if_fail_fe(::llc::controlSetParent(app.GUI, idRoot, app.Desktop));
	}
	for(::llc::u0_t iOther = 0; iOther < app.Viewports.size(); ++iOther) {
		cnst bool active = iOther == iViewport;
		app.GUI.Controls.SetSelected(app.Viewports[iOther].IdRoot, active);
		app.GUI.Controls.SetSelected(app.Viewports[iOther].IdTitle, active);
	}
	rtrn 0;
}

sttc void sampleGUIViewportImageUpdate(::llc::img8bgra & image, ::llc::u0_t theme, ::llc::u2_t frameIndex) nxpt {
	cnst ::llc::SColorBGRA tone = ::sampleGUIThemeColor(theme);
	cnst ::llc::bgra background = ::sampleGUIShade(tone, 14);
	cnst ::llc::bgra grid = ::sampleGUIShade(tone, 9);
	cnst ::llc::bgra pulse = ::sampleGUIShade(tone, 2);
	for(::llc::u2_t y = 0; y < image.metrics().y; ++y)
	for(::llc::u2_t x = 0; x < image.metrics().x; ++x) {
		cnst bool onGrid = 0 == x % 32 || 0 == y % 24;
		cnst bool onPulse = ((x + y + frameIndex) % 160) < 3;
		image[y][x] = onPulse ? pulse : onGrid ? grid : background;
	}
}

sttc ::llc::err_t sampleGUIViewportCreate(SSampleGUIViewport & app, ::llc::u0_t iViewport, ::llc::vcst_t title, cnst ::llc::rect2s1_t & area) {
	if_true_fe(iViewport >= app.Viewports.size());
	::llc::SViewportCreate setup = {};
	setup.Parent = app.Desktop;
	setup.ClientSize = area.Size;
	setup.Title = title;
	if_fail_fe(::llc::viewportCreate(app.Viewports[iViewport], app.GUI, setup));
	::llc::SViewport & viewport = app.Viewports[iViewport];
	app.GUI.Controls.Placement[viewport.IdRoot].Area.Offset = area.Offset;
	app.GUI.Controls.Placement[viewport.IdRoot].Border = {3, 3, 3, 3};
	app.GUI.Controls.Placement[viewport.IdRoot].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Placement[viewport.IdTitle].Border = {2, 2, 2, 2};
	app.GUI.Controls.Placement[viewport.IdClient].Border = {1, 1, 1, 1};
	app.GUI.Controls.Text[viewport.IdTitle].FontSelected = 2;
	::sampleGUIControlThemeApply(app.GUI, viewport.IdRoot, iViewport);
	::sampleGUIControlThemeApply(app.GUI, viewport.IdTitle, iViewport);
	::sampleGUIControlThemeApply(app.GUI, viewport.IdClient, iViewport);
	rtrn ::llc::controlMetricsInvalidate(app.GUI, viewport.IdRoot);
}

sttc ::llc::err_t sampleGUIViewportPerformanceUpdate(SSampleGUIViewport & app, double secondsLastFrame) {
	if(secondsLastFrame <= 0)
		rtrn 0;
	cnst ::llc::err_t textLength = ::llc::sprintf_s(app.PerformanceText.Storage, "FPS %.2f    Frame %.2f ms", 1.0 / secondsLastFrame, secondsLastFrame * 1000.0);
	if_fail_fe(textLength);
	app.GUI.Controls.Text[app.Performance].Text = {app.PerformanceText.Storage, (::llc::u2_t)textLength};
	app.GUI.Controls.SetUpdated(app.Performance, false);
	rtrn 0;
}

::llc::err_t sampleGUIViewportSetup(SSampleGUIViewport & app) {
	app.GUI.SelectedFont = 2;
	app.GUI.ColorModeDefault = ::llc::GUI_COLOR_MODE_3D;
	app.Desktop = ::llc::controlCreate(app.GUI);
	if_true_fe(app.Desktop < 0);
	app.GUI.Controls.Draw[app.Desktop].NoBorder = true;
	app.GUI.Controls.Draw[app.Desktop].NoClient = true;
	app.GUI.Controls.Constraints[app.Desktop].AttachSizeToControl = {app.Desktop, app.Desktop};
	app.Help = ::llc::controlCreateChild(app.GUI, app.Desktop);
	if_true_fe(app.Help < 0);
	app.GUI.Controls.Placement[app.Help].Area = {{18, 18}, {1040, 42}};
	app.GUI.Controls.Placement[app.Help].Border = {2, 2, 2, 2};
	app.GUI.Controls.Placement[app.Help].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[app.Help].Text = LLC_CXS("DRAG ANY TITLE | DOUBLE-CLICK TO FOLD | TAB MOVES FOCUS | ENTER OR SPACE ACTIVATES | ESC OR EXIT CLOSES");
	app.GUI.Controls.Text[app.Help].Align = ::llc::ALIGN_CENTER_LEFT;
	app.GUI.Controls.Text[app.Help].FontSelected = 1;
	::sampleGUIControlThemeApply(app.GUI, app.Help, 2);
	if_fail_fe(::sampleGUIViewportCreate(app, 0, LLC_CXS("AZURE CPU SURFACE"), {{80, 92}, {420, 250}}));
	if_fail_fe(::sampleGUIViewportCreate(app, 1, LLC_CXS("AMBER CPU SURFACE"), {{408, 184}, {430, 280}}));
	if_fail_fe(::sampleGUIViewportCreate(app, 2, LLC_CXS("MONO CPU SURFACE"), {{754, 104}, {360, 230}}));
	if_fail_fe(::sampleGUIViewportActivate(app, 2));
	app.Performance = ::llc::controlCreate(app.GUI);
	if_true_fe(app.Performance < 0);
	app.GUI.Controls.Placement[app.Performance].Area = {{0, 14}, {360, 36}};
	app.GUI.Controls.Placement[app.Performance].Align = ::llc::ALIGN_CENTER_BOTTOM;
	app.GUI.Controls.Placement[app.Performance].Border = {1, 1, 1, 1};
	app.GUI.Controls.Placement[app.Performance].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[app.Performance].Text = LLC_CXS("FPS --    Frame -- ms");
	app.GUI.Controls.Text[app.Performance].FontSelected = 2;
	::sampleGUIControlThemeApply(app.GUI, app.Performance, 2);
	app.Exit = ::llc::controlCreate(app.GUI);
	if_true_fe(app.Exit < 0);
	app.GUI.Controls.Placement[app.Exit].Area = {{18, 18}, {96, 36}};
	app.GUI.Controls.Placement[app.Exit].Align = ::llc::ALIGN_BOTTOM_RIGHT;
	app.GUI.Controls.Placement[app.Exit].Border = {2, 2, 2, 2};
	app.GUI.Controls.Placement[app.Exit].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[app.Exit].Text = LLC_CXS("EXIT");
	app.GUI.Controls.Text[app.Exit].FontSelected = 2;
	::sampleGUIControlThemeApply(app.GUI, app.Exit, 1);
	for(::llc::u0_t iViewport = 0; iViewport < app.Viewports.size(); ++iViewport) {
		app.ControlsKeyboard[iViewport] = app.Viewports[iViewport].IdTitle;
	}
	app.ControlsKeyboard[app.Viewports.size()] = app.Exit;
	rtrn 0;
}

::llc::err_t sampleGUIViewportRenderResourcesSetup(cnst SSampleGUIViewport & app, ::llc::SGUIRenderResources & resources, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT]) {
	if_fail_fe(::sampleGUIResourcesCreate(resources));
	if_fail_fe(::llc::guiRenderResourcesSync(resources, app.GUI));
	for(::llc::u0_t iViewport = 0; iViewport < app.Viewports.size(); ++iViewport) {
		cnst ::llc::cid_t idClient = app.Viewports[iViewport].IdClient;
		if_fail_fe(clientImages[iViewport].resize(app.GUI.Controls.Placement[idClient].Area.Size));
		resources.Images[idClient].Image = clientImages[iViewport].View;
		resources.Images[idClient].ImageAlign = ::llc::ALIGN_CENTER;
	}
	rtrn 0;
}

::llc::err_t sampleGUIViewportUpdate(SSampleGUIViewport & app, bool systemRequestedExit, double secondsLastFrame) {
	if(systemRequestedExit || app.Window.Closed || app.Window.Input.KeyDown(::llc::SYSTEM_KEY_Escape))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if_fail_fe(::llc::guiProcessInput(app.GUI, app.Window.Input, app.ControlsKeyboard, app.Window.EventQueue));
	if(app.GUI.Controls.IsAction(app.Exit))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if_fail_fe(::sampleGUIViewportPerformanceUpdate(app, secondsLastFrame));
	bool doubleClick = false;
	if_fail_fe(::sampleGUIViewportDoubleClick(app.Window.EventQueue, doubleClick));
	::llc::u2_t hoveredViewport = app.Viewports.size();
	for(::llc::u0_t iViewport = 0; iViewport < app.Viewports.size(); ++iViewport) {
		::llc::SViewport & viewport = app.Viewports[iViewport];
		if(::sampleGUIViewportPressed(app, viewport) || app.GUI.Controls.IsAction(viewport.IdTitle))
			if_fail_fe(::sampleGUIViewportActivate(app, iViewport));
		if(app.GUI.Controls.IsHovered(viewport.IdTitle))
			hoveredViewport = iViewport;
		app.GUI.Controls.SetAction(viewport.IdTitle, false);
		cnst ::llc::n2s2_t mouseDeltas = {app.Window.Input.MouseCurrent.Deltas.x, app.Window.Input.MouseCurrent.Deltas.y};
		if_fail_fe(::llc::viewportUpdate(viewport, app.GUI, mouseDeltas));
	}
	if(doubleClick && hoveredViewport < app.Viewports.size())
		if_fail_fe(::llc::viewportFold(app.Viewports[hoveredViewport], app.GUI, app.Viewports[hoveredViewport].Settings.Unfolded));
	rtrn 0;
}

::llc::err_t sampleGUIViewportRenderEvents(SSampleGUIViewport & app, ::llc::TGUIRenderEventQueue & output, cnst ::llc::n2u1_t & targetSize, cnst ::llc::FGUITextMeasure & textMeasure) {
	rtrn ::llc::guiRenderEvents(app.GUI, output, targetSize, textMeasure);
}

::llc::err_t sampleGUIViewportDraw(cnst ::llc::TGUIRenderEventQueue & events, ::llc::SGUIRenderResources & resources, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::u2_t frameIndex, ::llc::g8bgra target) {
	if_fail_fe(target.fill(::llc::SColorBGRA{16, 14, 12}));
	for(::llc::u0_t iViewport = 0; iViewport < SAMPLE_GUI_VIEWPORT_COUNT; ++iViewport)
		::sampleGUIViewportImageUpdate(clientImages[iViewport], iViewport, frameIndex);
	rtrn ::llc::guiDraw(events, resources, target);
}
