#include "llc_sample_gui_components.h"

#include "llc_sample_gui_resources.h"
#include "llc_apod_serialize.h"
#include "llc_cstring.h"
#include "llc_runtime.h"
#include "llc_system_key.h"

#include <limits.h>

sttc ::llc::err_t sampleGUIComponentsDoubleClick(cnst ::llc::TQueueSystemEvent & events, bool & doubleClick) {
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

sttc bool sampleGUIComponentsPressed(cnst SSampleGUIComponents & app, cnst ::llc::SViewport & viewport) nxpt {
	rtrn app.GUI.Controls.IsPressed(viewport.IdRoot)
		|| app.GUI.Controls.IsPressed(viewport.IdTitle)
		|| app.GUI.Controls.IsPressed(viewport.IdClient)
		;
}

sttc ::llc::err_t sampleGUIComponentsActivate(SSampleGUIComponents & app, ::llc::u0_t iViewport) {
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

sttc void sampleGUIComponentsImageUpdate(::llc::img8bgra & image, ::llc::u0_t theme, ::llc::u2_t frameIndex) nxpt {
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

sttc ::llc::err_t sampleGUIComponentsCreate(SSampleGUIComponents & app, ::llc::u0_t iViewport, ::llc::vcst_t title, cnst ::llc::rect2s1_t & area) {
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

sttc ::llc::err_t sampleGUIComponentsPerformanceUpdate(SSampleGUIComponents & app, double secondsLastFrame) {
	if(secondsLastFrame <= 0)
		rtrn 0;
	cnst ::llc::err_t textLength = ::llc::sprintf_s(app.PerformanceText.Storage, "FPS %.2f    Frame %.2f ms", 1.0 / secondsLastFrame, secondsLastFrame * 1000.0);
	if_fail_fe(textLength);
	app.GUI.Controls.Text[app.Performance].Text = {app.PerformanceText.Storage, (::llc::u2_t)textLength};
	app.GUI.Controls.SetUpdated(app.Performance, false);
	rtrn 0;
}

sttc ::llc::cid_t sampleGUIComponentsControlCreate(SSampleGUIComponents & app, ::llc::vcst_t text, cnst ::llc::rect2s1_t & area) {
	cnst ::llc::cid_t idControl = ::llc::controlCreateChild(app.GUI, app.ComponentsPanel);
	if(idControl < 0)
		rtrn idControl;
	::llc::SControlPlacement & placement = app.GUI.Controls.Placement[idControl];
	placement.Area = area;
	placement.Margin = {1, 1, 1, 1};
	placement.Border = {2, 2, 2, 2};
	app.GUI.Controls.Text[idControl].Align = ::llc::ALIGN_CENTER;
	app.GUI.Controls.Text[idControl].FontSelected = 2;
	if(::llc::failed(::llc::controlTextSet(app.GUI, idControl, text))) {
		::llc::controlDelete(app.GUI, idControl);
		rtrn -1;
	}
	rtrn idControl;
}

sttc ::llc::err_t sampleGUIComponentsKeyboardAppend(SSampleGUIComponents & app) {
	if_fail_fe(app.ControlsKeyboard.push_back(app.CheckBox.IdControl));
	if_fail_fe(app.ControlsKeyboard.push_back(app.SliderHorizontal.IdRoot));
	if_fail_fe(app.ControlsKeyboard.push_back(app.SliderVertical.IdRoot));
	if_fail_fe(app.ControlsKeyboard.push_back(app.ControlEdit));
	for(::llc::u2_t iCharacter = 0; iCharacter < app.InputBox.VirtualKeyboard.Characters.size(); ++iCharacter)
		if_fail_fe(app.ControlsKeyboard.push_back(app.InputBox.VirtualKeyboard.Characters[iCharacter].IdControl));
	for(::llc::u2_t iCommand = 0; iCommand < app.InputBox.VirtualKeyboard.Commands.size(); ++iCommand)
		if_fail_fe(app.ControlsKeyboard.push_back(app.InputBox.VirtualKeyboard.Commands[iCommand].IdControl));
	rtrn app.ControlsKeyboard.push_back(app.ControlExit);
}

sttc void sampleGUIComponentsThemeApply(SSampleGUIComponents & app) nxpt {
	::sampleGUIControlThemeApply(app.GUI, app.CheckBox.IdControl, 0);
	::sampleGUIControlThemeApply(app.GUI, app.SliderHorizontal.IdRoot, 1);
	::sampleGUIControlThemeApply(app.GUI, app.SliderHorizontal.IdThumb, 1);
	::sampleGUIControlThemeApply(app.GUI, app.SliderVertical.IdRoot, 2);
	::sampleGUIControlThemeApply(app.GUI, app.SliderVertical.IdThumb, 2);
	::sampleGUIControlThemeApply(app.GUI, app.ControlEdit, 0);
	::sampleGUIControlThemeApply(app.GUI, app.ControlExit, 1);
	::sampleGUIControlThemeApply(app.GUI, app.InputBox.IdRoot, 2);
	::sampleGUIControlThemeApply(app.GUI, app.InputBox.IdText, 2);
	::sampleGUIControlThemeApply(app.GUI, app.InputBox.VirtualKeyboard.IdRoot, 2);
	for(::llc::u2_t iCharacter = 0; iCharacter < app.InputBox.VirtualKeyboard.Characters.size(); ++iCharacter)
		::sampleGUIControlThemeApply(app.GUI, app.InputBox.VirtualKeyboard.Characters[iCharacter].IdControl, 2);
	for(::llc::u2_t iCommand = 0; iCommand < app.InputBox.VirtualKeyboard.Commands.size(); ++iCommand)
		::sampleGUIControlThemeApply(app.GUI, app.InputBox.VirtualKeyboard.Commands[iCommand].IdControl, 2);
}

sttc ::llc::err_t sampleGUIComponentsControlsSetup(SSampleGUIComponents & app) {
	app.ComponentsPanel = ::llc::controlCreateChild(app.GUI, app.Desktop);
	if_true_fe(app.ComponentsPanel < 0);
	::llc::SControlPlacement & panelPlacement = app.GUI.Controls.Placement[app.ComponentsPanel];
	panelPlacement.Area = {{18, 18}, {500, 112}};
	panelPlacement.Align = ::llc::ALIGN_BOTTOM_LEFT;
	panelPlacement.Margin = {};
	panelPlacement.Border = {};
	app.GUI.Controls.Draw[app.ComponentsPanel].NoBorder = true;
	app.GUI.Controls.Draw[app.ComponentsPanel].NoClient = true;

	::llc::SCheckBoxCreate checkBoxSetup = {};
	checkBoxSetup.Parent = app.ComponentsPanel;
	checkBoxSetup.Area = {{0, 0}, {32, 32}};
	checkBoxSetup.Text = LLC_CXS("X");
	if_fail_fe(::llc::checkBoxCreate(app.CheckBox, app.GUI, checkBoxSetup));
	cnst ::llc::cid_t idCheckBoxLabel = ::sampleGUIComponentsControlCreate(app, LLC_CXS("CHECK BOX | ENTER / SPACE"), {{38, 0}, {212, 32}});
	if_true_fe(idCheckBoxLabel < 0);
	app.GUI.Controls.Draw[idCheckBoxLabel].NoBorder = true;
	app.GUI.Controls.Draw[idCheckBoxLabel].NoClient = true;
	app.GUI.Controls.Text[idCheckBoxLabel].Align = ::llc::ALIGN_CENTER_LEFT;
	::sampleGUIControlThemeApply(app.GUI, idCheckBoxLabel, 0);

	::llc::SSliderCreate horizontalSetup = {};
	horizontalSetup.Parent = app.ComponentsPanel;
	horizontalSetup.Area = {{0, 40}, {250, 30}};
	horizontalSetup.Value = 35;
	if_fail_fe(::llc::sliderCreate(app.SliderHorizontal, app.GUI, horizontalSetup));
	app.SliderHorizontal.TextFormat = [](::llc::astchar<64> & output, ::llc::s3_t value, ::llc::minmaxs3_c &) { rtrn ::llc::sprintf_s(output.Storage, "HORIZONTAL %" LLC_FMT_S3, value); };
	if_fail_fe(::llc::sliderSetValue(app.SliderHorizontal, app.GUI, app.SliderHorizontal.ValueCurrent));

	::llc::SSliderCreate verticalSetup = {};
	verticalSetup.Parent = app.ComponentsPanel;
	verticalSetup.Area = {{260, 0}, {40, 108}};
	verticalSetup.Value = 65;
	verticalSetup.Vertical = true;
	if_fail_fe(::llc::sliderCreate(app.SliderVertical, app.GUI, verticalSetup));
	if_fail_fe(::llc::controlTextSet(app.GUI, app.SliderVertical.IdRoot, LLC_CXS("V")));

	app.ControlEdit = ::sampleGUIComponentsControlCreate(app, LLC_CXS("EDIT TEXT / CP437 KEYBOARD"), {{310, 0}, {190, 32}});
	if_true_fe(app.ControlEdit < 0);
	app.ControlExit = ::sampleGUIComponentsControlCreate(app, LLC_CXS("EXIT"), {{310, 40}, {190, 32}});
	if_true_fe(app.ControlExit < 0);

	if_fail_fe(::llc::inputBoxCreate(app.InputBox, app.GUI, app.Desktop));
	app.GUI.Controls.Placement[app.InputBox.IdRoot].Align = ::llc::ALIGN_CENTER_BOTTOM;
	app.GUI.Controls.Placement[app.InputBox.IdRoot].Area.Offset.y = 70;
	app.GUI.Controls.Text[app.InputBox.IdText].FontSelected = 2;
	if_fail_fe(::sampleGUIComponentsKeyboardAppend(app));
	::sampleGUIComponentsThemeApply(app);
	rtrn 0;
}

::llc::err_t sampleGUIComponentsSetup(SSampleGUIComponents & app) {
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
	app.GUI.Controls.Text[app.Help].Text = LLC_CXS("DRAG / FOLD VIEWPORTS | TAB / SHIFT+TAB FOCUS | ENTER / SPACE ACTIVATE | ARROWS / PAGE / HOME / END SLIDE | ESC CLOSES / EXITS");
	app.GUI.Controls.Text[app.Help].Align = ::llc::ALIGN_CENTER_LEFT;
	app.GUI.Controls.Text[app.Help].FontSelected = 1;
	::sampleGUIControlThemeApply(app.GUI, app.Help, 2);
	if_fail_fe(::sampleGUIComponentsCreate(app, 0, LLC_CXS("AZURE CPU SURFACE"), {{80, 92}, {420, 250}}));
	if_fail_fe(::sampleGUIComponentsCreate(app, 1, LLC_CXS("AMBER CPU SURFACE"), {{408, 184}, {430, 280}}));
	if_fail_fe(::sampleGUIComponentsCreate(app, 2, LLC_CXS("MONO CPU SURFACE"), {{754, 104}, {360, 230}}));
	if_fail_fe(::sampleGUIComponentsActivate(app, 2));
	if_fail_fe(::sampleGUIComponentsControlsSetup(app));
	app.Performance = ::llc::controlCreate(app.GUI);
	if_true_fe(app.Performance < 0);
	app.GUI.Controls.Placement[app.Performance].Area = {{0, 14}, {360, 36}};
	app.GUI.Controls.Placement[app.Performance].Align = ::llc::ALIGN_CENTER_BOTTOM;
	app.GUI.Controls.Placement[app.Performance].Border = {1, 1, 1, 1};
	app.GUI.Controls.Placement[app.Performance].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[app.Performance].Text = LLC_CXS("FPS --    Frame -- ms");
	app.GUI.Controls.Text[app.Performance].FontSelected = 2;
	::sampleGUIControlThemeApply(app.GUI, app.Performance, 2);
	rtrn 0;
}

::llc::err_t sampleGUIComponentsRenderResourcesSetup(cnst SSampleGUIComponents & app, ::llc::SGUIRenderResources & resources, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_COMPONENTS_COUNT]) {
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

::llc::err_t sampleGUIComponentsUpdate(SSampleGUIComponents & app, bool systemRequestedExit, double secondsLastFrame) {
	if(systemRequestedExit || app.Window.Closed)
		rtrn ::llc::APPLICATION_STATE_EXIT;
	cnst bool editingAtFrameStart = app.InputBox.Editing;
	if_fail_fe(::llc::guiProcessInput(app.GUI, app.Window.Input, app.ControlsKeyboard, app.Window.EventQueue));
	if_fail_fe(::llc::checkBoxUpdate(app.CheckBox, app.GUI));
	if_fail_fe(::llc::sliderUpdate(app.SliderHorizontal, app.GUI, app.Window.Input));
	if_fail_fe(::llc::sliderUpdate(app.SliderVertical, app.GUI, app.Window.Input));
	if(app.GUI.Controls.IsAction(app.ControlEdit))
		app.InputBox.Edit(app.GUI, true);
	if(editingAtFrameStart) {
		cnst ::llc::err_t inputResult = app.InputBox.Update(app.GUI, app.InputBox.VirtualKeyboard, app.Window.EventQueue, app.ControlsKeyboard);
		if_fail_fe(inputResult);
		if(INT_MAX == inputResult)
			app.InputBox.Edit(app.GUI, false);
	}
	else if(app.Window.Input.KeyDown(::llc::SYSTEM_KEY_Escape))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if(app.GUI.Controls.IsAction(app.ControlExit))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if_fail_fe(::sampleGUIComponentsPerformanceUpdate(app, secondsLastFrame));
	bool doubleClick = false;
	if_fail_fe(::sampleGUIComponentsDoubleClick(app.Window.EventQueue, doubleClick));
	::llc::u2_t hoveredViewport = app.Viewports.size();
	for(::llc::u0_t iViewport = 0; iViewport < app.Viewports.size(); ++iViewport) {
		::llc::SViewport & viewport = app.Viewports[iViewport];
		if(::sampleGUIComponentsPressed(app, viewport))
			if_fail_fe(::sampleGUIComponentsActivate(app, iViewport));
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

::llc::err_t sampleGUIComponentsRenderEvents(SSampleGUIComponents & app, ::llc::TGUIRenderEventQueue & output, cnst ::llc::n2u1_t & targetSize, cnst ::llc::FGUITextMeasure & textMeasure) {
	rtrn ::llc::guiRenderEvents(app.GUI, output, targetSize, textMeasure);
}

::llc::err_t sampleGUIComponentsDraw(cnst ::llc::TGUIRenderEventQueue & events, ::llc::SGUIRenderResources & resources, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_COMPONENTS_COUNT], ::llc::u2_t frameIndex, ::llc::g8bgra target) {
	if_fail_fe(target.fill(::llc::SColorBGRA{16, 14, 12}));
	for(::llc::u0_t iViewport = 0; iViewport < SAMPLE_GUI_COMPONENTS_COUNT; ++iViewport)
		::sampleGUIComponentsImageUpdate(clientImages[iViewport], iViewport, frameIndex);
	rtrn ::llc::guiDraw(events, resources, target);
}
