#include "llc_sample_gui_viewport.h"
#include "llc_mutex.h"

#if defined(LLC_WINDOWS)
#	include "llc_engine_d3d_app.h"
#endif

#include "llc_apod_serialize.h"
#include "llc_app_impl.h"
#include "llc_cstring.h"
#include "llc_framework.h"
#include "llc_runtime.h"
#include "llc_system_key.h"

stacxpr ::llc::u0_t SAMPLE_GUI_THEME_COUNT = 3;
stacxpr ::llc::u0_t PALETTE_SHADE_COUNT = 16;

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
	rtrn app.GUI.Controls.IsPressed(viewport.IdControl)
		|| app.GUI.Controls.IsPressed(viewport.IdControls[::llc::VIEWPORT_CONTROL_TITLE])
		|| app.GUI.Controls.IsPressed(viewport.IdControls[::llc::VIEWPORT_CONTROL_TARGET])
		;
}

sttc ::llc::err_t sampleGUIViewportActivate(SSampleGUIViewport & app, ::llc::u0_t iViewport) {
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
	app.GUI.Controls.Text[idTitle].FontSelected = 2;
	app.GUI.Controls.Placement[idTitle].Border = {2, 2, 2, 2};
	app.GUI.Controls.Placement[idClient].Area.Size = {(::llc::s1_t)(area.Size.x - 8), (::llc::s1_t)(area.Size.y - 32)};
	app.GUI.Controls.Placement[idClient].Border = {1, 1, 1, 1};
	::sampleGUIControlThemeApply(app.GUI, viewport.IdControl, iViewport);
	for(::llc::u0_t iControl = 0; iControl < viewport.IdControls.size(); ++iControl)
		::sampleGUIControlThemeApply(app.GUI, viewport.IdControls[iControl], iViewport);
	app.UnfoldedSize[iViewport] = area.Size;
	rtrn ::llc::controlMetricsInvalidate(app.GUI, viewport.IdControl);
}

sttc ::llc::err_t sampleGUIViewportFold(SSampleGUIViewport & app, ::llc::u0_t iViewport) {
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

sttc ::llc::err_t sampleGUIViewportPerformanceUpdate(SSampleGUIViewport & app, ::llc::vcst_t backend, const ::llc::SFrameMeter & frameMeter) {
	cnst ::llc::err_t textLength = ::llc::sprintf_s(app.PerformanceText.Storage, "%.*s | %u FPS | %.2f ms avg | %ux%u | F3", LLC_VIEW_ARGS(backend), frameMeter.FramesPerSecond, frameMeter.AverageFrameTime * 1000, app.Window.Size.x, app.Window.Size.y);
	if_fail_fe(textLength);
	app.GUI.Controls.Text[app.Performance].Text = {app.PerformanceText.Storage, (::llc::u2_t)textLength};
	app.GUI.Controls.SetUpdated(app.Performance, false);
	rtrn 0;
}

::llc::err_t sampleGUIViewportSetup(SSampleGUIViewport & app) {
	app.GUI.SelectedFont = 2;
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
	app.GUI.Controls.Placement[app.Help].Area = {{18, 18}, {1040, 42}};
	app.GUI.Controls.Placement[app.Help].Border = {2, 2, 2, 2};
	app.GUI.Controls.Placement[app.Help].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[app.Help].Text = "DRAG ANY TITLE | DOUBLE-CLICK TO FOLD | TAB MOVES FOCUS | ENTER OR SPACE ACTIVATES | ESC OR EXIT CLOSES";
	app.GUI.Controls.Text[app.Help].Align = ::llc::ALIGN_CENTER_LEFT;
	app.GUI.Controls.Text[app.Help].FontSelected = 1;
	::sampleGUIControlThemeApply(app.GUI, app.Help, 2);
	if_fail_fe(::sampleGUIViewportCreate(app, 0, "AZURE CPU SURFACE", {{80, 92}, {420, 250}}));
	if_fail_fe(::sampleGUIViewportCreate(app, 1, "AMBER CPU SURFACE", {{408, 184}, {430, 280}}));
	if_fail_fe(::sampleGUIViewportCreate(app, 2, "MONO CPU SURFACE", {{754, 104}, {360, 230}}));
	if_fail_fe(::sampleGUIViewportActivate(app, 2));
	app.Performance = ::llc::controlCreate(app.GUI);
	if_true_fe(app.Performance < 0);
	app.GUI.Controls.Placement[app.Performance].Area = {{0, 14}, {560, 36}};
	app.GUI.Controls.Placement[app.Performance].Align = ::llc::ALIGN_CENTER_BOTTOM;
	app.GUI.Controls.Placement[app.Performance].Border = {1, 1, 1, 1};
	app.GUI.Controls.Placement[app.Performance].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[app.Performance].Text = "CPU | -- FPS | -- ms avg | F3";
	app.GUI.Controls.Text[app.Performance].FontSelected = 2;
	::sampleGUIControlThemeApply(app.GUI, app.Performance, 2);
	app.Exit = ::llc::controlCreate(app.GUI);
	if_true_fe(app.Exit < 0);
	app.GUI.Controls.Placement[app.Exit].Area = {{18, 18}, {96, 36}};
	app.GUI.Controls.Placement[app.Exit].Align = ::llc::ALIGN_BOTTOM_RIGHT;
	app.GUI.Controls.Placement[app.Exit].Border = {2, 2, 2, 2};
	app.GUI.Controls.Placement[app.Exit].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[app.Exit].Text = "EXIT";
	app.GUI.Controls.Text[app.Exit].FontSelected = 2;
	::sampleGUIControlThemeApply(app.GUI, app.Exit, 1);
	rtrn ::llc::guiUpdateMetrics(app.GUI, app.Window.Size, false);
}

::llc::err_t sampleGUIViewportUpdate(SSampleGUIViewport & app, bool systemRequestedExit, double secondsLastFrame) {
	(void)secondsLastFrame;
	if(systemRequestedExit || app.Window.Closed || app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Escape))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if(app.GUI.Controls.IsAction(app.Exit))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	bool doubleClick = false;
	if_fail_fe(::sampleGUIViewportDoubleClick(app.Window.EventQueue, doubleClick));
	::llc::u2_t hoveredViewport = app.Viewports.size();
	for(::llc::u0_t iViewport = 0; iViewport < app.Viewports.size(); ++iViewport) {
		::llc::SViewport & viewport = app.Viewports[iViewport];
		cnst ::llc::cid_t idTitle = viewport.IdControls[::llc::VIEWPORT_CONTROL_TITLE];
		cnst ::llc::cid_t idClose = viewport.IdControls[::llc::VIEWPORT_CONTROL_CLOSE];
		cnst ::llc::cid_t idResize = viewport.IdControls[::llc::VIEWPORT_CONTROL_RESIZE_BOTTOM_RIGHT];
		if(::sampleGUIViewportPressed(app, viewport))
			if_fail_fe(::sampleGUIViewportActivate(app, iViewport));
		if(app.GUI.Controls.IsAction(idClose))
			app.GUI.Controls.SetHidden(viewport.IdControl, true);
		if(app.GUI.Controls.IsPressed(idTitle) && (app.Window.Input->MouseCurrent.Deltas.x || app.Window.Input->MouseCurrent.Deltas.y)) {
			cnst ::llc::n2s1_t delta = {(::llc::s1_t)app.Window.Input->MouseCurrent.Deltas.x, (::llc::s1_t)app.Window.Input->MouseCurrent.Deltas.y};
			app.GUI.Controls.Placement[viewport.IdControl].Area.Offset += delta;
			if_fail_fe(::llc::controlMetricsInvalidate(app.GUI, viewport.IdControl));
		}
		if(app.GUI.Controls.IsPressed(idResize) && (app.Window.Input->MouseCurrent.Deltas.x || app.Window.Input->MouseCurrent.Deltas.y)) {
			cnst ::llc::n2s1_t delta = {(::llc::s1_t)app.Window.Input->MouseCurrent.Deltas.x, (::llc::s1_t)app.Window.Input->MouseCurrent.Deltas.y};
			app.GUI.Controls.Placement[viewport.IdControl].Area.Size += delta;
			app.GUI.Controls.Placement[viewport.IdControls[::llc::VIEWPORT_CONTROL_TARGET]].Area.Size += delta;
			app.UnfoldedSize[iViewport] = app.GUI.Controls.Placement[viewport.IdControl].Area.Size;
			if_fail_fe(::llc::controlMetricsInvalidate(app.GUI, viewport.IdControl));
		}
		if(app.GUI.Controls.IsHovered(idTitle))
			hoveredViewport = iViewport;
	}
	if(doubleClick && hoveredViewport < app.Viewports.size())
		if_fail_fe(::sampleGUIViewportFold(app, (::llc::u0_t)hoveredViewport));
	rtrn ::llc::guiUpdateMetrics(app.GUI, app.Window.Size, false);
}

sttc ::llc::err_t sampleGUIViewportImagesUpdate(SSampleGUIViewport & app, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::u2_t frameIndex) {
	for(::llc::u0_t iViewport = 0; iViewport < SAMPLE_GUI_VIEWPORT_COUNT; ++iViewport) {
		cnst ::llc::cid_t idClient = app.Viewports[iViewport].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
		cnst ::llc::n2u2_t imageSize = app.GUI.Controls.Placement[idClient].Area.Size.u2();
		if(clientImages[iViewport].metrics() != imageSize)
			if_fail_fe(clientImages[iViewport].resize(imageSize));
		::sampleGUIViewportImageUpdate(clientImages[iViewport], iViewport, frameIndex);
		app.GUI.Controls.Images[idClient].Image = clientImages[iViewport].View;
		app.GUI.Controls.Images[idClient].ImageAlign = ::llc::ALIGN_CENTER;
	}
	rtrn 0;
}

::llc::err_t sampleGUIViewportDraw(SSampleGUIViewport & app, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::u2_t frameIndex, ::llc::g8bgra target) {
	if_fail_fe(target.fill(::llc::SColorBGRA{16, 14, 12}));
	if_fail_fe(::sampleGUIViewportImagesUpdate(app, clientImages, frameIndex));
	rtrn ::llc::guiDraw(app.GUI, target);
}

#define LLC_SAMPLE_GUI_VIEWPORT_TITLE "LLC GUI viewports"

stct SApplication {
	::llc::SFramework	Framework;
	SSampleGUIViewport	Sample;
	::llc::img8bgra	ClientImages[SAMPLE_GUI_VIEWPORT_COUNT] = {};
	::llc::u2_t		FrameIndex = 0;
	::llc::mutex		Lock;
#if defined(LLC_WINDOWS)
	::llc::pobj<::llc::SD3DApplication> D3D;
	bool				UseD3D = false;
	bool				COMInitialized = false;
#endif
	bool				ShowPerformance = true;

	SApplication(::llc::SRuntimeValues & runtimeValues)
		: Framework(runtimeValues, {}), Sample(Framework.RootWindow, *Framework.GUI) {
		if_fail_e(::llc::rasterFontDefaults(Sample.Engine.Scene->Graphics->Fonts));
	}
};

#if defined(LLC_WINDOWS)
sttc ::llc::err_t sampleGUIViewportRendererSet(SApplication & app, bool useD3D) {
	::llc::SWindow & window = app.Framework.RootWindow;
	if(useD3D) {
		if(!app.D3D) {
			cnst HRESULT result = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
			if(result != RPC_E_CHANGED_MODE) {
				if_true_fef(FAILED(result), "CoInitializeEx failed: 0x%08X.", (::llc::u2_t)result);
				app.COMInitialized = true;
			}
			if_fail_fe(app.D3D->Initialize(window.PlatformDetail.WindowHandle, app.Sample.Engine.Scene->Graphics));
			if_fail_fe(app.D3D->CreateDeviceResources(*app.Sample.Engine.Scene->Graphics));
		}
		if_fail_fe(app.D3D->SetWindowSize(window.Size));
		window.BackBuffer = {};
	}
	else
		if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	app.UseD3D = useD3D;
	::SetWindowTextA(window.PlatformDetail.WindowHandle, useD3D
		? LLC_SAMPLE_GUI_VIEWPORT_TITLE " - D3D11 | F3: HUD | F4: CPU"
		: LLC_SAMPLE_GUI_VIEWPORT_TITLE " - CPU | F3: HUD | F4: D3D11");
	rtrn 0;
}
#endif

::llc::err_t setup(SApplication & app) {
	::llc::SWindow & window = app.Framework.RootWindow;
	window.Size = {1280, 720};
	if_null_ve(::llc::OS_NO_MEMORY, window.BackBuffer.create());
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	if_fail_fe(::llc::mainWindowCreate(window, app.Framework.RuntimeValues.PlatformDetail, window.Input));
	if_fail_fe(::sampleGUIViewportSetup(app.Sample));
#if defined(LLC_WINDOWS)
	::SetWindowTextA(window.PlatformDetail.WindowHandle, LLC_SAMPLE_GUI_VIEWPORT_TITLE " - CPU | F3: HUD | F4: D3D11");
#endif
	rtrn 0;
}

::llc::err_t cleanup(SApplication & app) {
	::llc::lock_guard locked(app.Lock);
#if defined(LLC_WINDOWS)
	if(app.D3D) {
		if(app.D3D->DeviceResources && app.D3D->DeviceResources->m_swapChain)
			if_fail_e(app.D3D->Shutdown());
		app.D3D = {};
	}
	if(app.COMInitialized) {
		::CoUninitialize();
		app.COMInitialized = false;
	}
#endif
	rtrn ::llc::mainWindowDestroy(app.Framework.RootWindow);
}

sttc ::llc::err_t sampleGUIViewportDrawFrame(SApplication & app);

::llc::err_t update(SApplication & app, bool systemRequestedExit) {
	::llc::lock_guard locked(app.Lock);
	if(systemRequestedExit)
		rtrn ::llc::APPLICATION_STATE_EXIT;
	cnst ::llc::err_t result = ::llc::updateFramework(app.Framework);
	if(result)
		rtrn result;
	::llc::SWindow & window = app.Framework.RootWindow;
	if(window.Input->KeyDown(::llc::SYSTEM_KEY_F3)) {
		app.ShowPerformance = !app.ShowPerformance;
		app.Sample.GUI.Controls.SetHidden(app.Sample.Performance, !app.ShowPerformance);
	}
#if defined(LLC_WINDOWS)
	if(window.Input->KeyDown(::llc::SYSTEM_KEY_F4))
		if_fail_fe(::sampleGUIViewportRendererSet(app, !app.UseD3D));
#endif
	cnst ::llc::err_t resultSample = ::sampleGUIViewportUpdate(app.Sample, false, app.Framework.FrameInfo.Seconds.LastFrame);
	cnst ::llc::vcst_t backend =
#if defined(LLC_WINDOWS)
		app.UseD3D ? ::llc::vcst_t{"D3D11"} : ::llc::vcst_t{"CPU"};
#else
		"CPU";
#endif
	if_fail_fe(::sampleGUIViewportPerformanceUpdate(app.Sample, backend, app.Framework.FrameInfo.FrameMeter));
#if defined(LLC_WINDOWS) && defined(LLC_AVOID_LOCAL_APPLICATION_MODEL_EXECUTABLE_RUNTIME)
	if(app.D3D)
		rtrn ::sampleGUIViewportDrawFrame(app);
#endif
	::llc::sleep(1);
	rtrn resultSample;
}

sttc ::llc::err_t sampleGUIViewportDrawFrame(SApplication & app) {
	::llc::SWindow & window = app.Framework.RootWindow;
	if(window.NoDraw || 0 == window.Size.x || 0 == window.Size.y)
		rtrn 0;
#if defined(LLC_WINDOWS)
	if(app.UseD3D) {
		if(app.D3D->GUIStuff.RenderTarget.metrics() != window.Size.u2())
			if_fail_fe(app.D3D->SetWindowSize(window.Size));
		if_fail_fe(::sampleGUIViewportImagesUpdate(app.Sample, app.ClientImages, app.FrameIndex++));
		if_fail_fe(::llc::guiUpdateMetrics(app.Sample.GUI, window.Size, app.Sample.GUI.LastSize != window.Size || app.Sample.GUI.LastZoom != app.Sample.GUI.Zoom));
		::DX::D3DDeviceResources & resources = *app.D3D->DeviceResources;
		ID3D11DeviceContext * context = resources.GetD3DDeviceContext();
		cnst D3D11_VIEWPORT viewport = resources.GetScreenViewport();
		context->RSSetViewports(1, &viewport);
		ID3D11RenderTargetView * target = resources.GetBackBufferRenderTargetView();
		context->OMSetRenderTargets(1, &target, resources.GetDepthStencilView());
		cnst ::llc::rgbaf clear = ::llc::SColorBGRA{16, 14, 12};
		context->ClearRenderTargetView(target, &clear.r);
		if_fail_fe(::llc::d3dGUIDraw(resources, app.D3D->GUIStuff, app.Sample.GUI, window.Size));
		rtrn resources.Present();
	}
#endif
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	if_fail_fe(::sampleGUIViewportDraw(app.Sample, app.ClientImages, app.FrameIndex++, window.BackBuffer->Color.View));
#if defined(LLC_WINDOWS)
	if(app.D3D) {
		if(app.D3D->GUIStuff.RenderTarget.metrics() != window.Size.u2())
			if_fail_fe(app.D3D->SetWindowSize(window.Size));
		memcpy(app.D3D->GUIStuff.RenderTarget.begin(), window.BackBuffer->Color.begin(), window.BackBuffer->Color.byte_count());
		for(auto & pixel : app.D3D->GUIStuff.RenderTarget.Texels) pixel.a = 255;
		::DX::D3DDeviceResources & resources = *app.D3D->DeviceResources;
		ID3D11DeviceContext * context = resources.GetD3DDeviceContext();
		ID3D11RenderTargetView * target = resources.GetBackBufferRenderTargetView();
		context->OMSetRenderTargets(1, &target, resources.GetDepthStencilView());
		cnst D3D11_VIEWPORT viewport = resources.GetScreenViewport();
		context->RSSetViewports(1, &viewport);
		if_fail_fe(::llc::d3dGUIDraw(resources, app.D3D->GUIStuff));
		rtrn resources.Present();
	}
#endif
	window.Repaint = true;
	rtrn 0;
}

::llc::err_t draw(SApplication & app) {
#if defined(LLC_WINDOWS) && defined(LLC_AVOID_LOCAL_APPLICATION_MODEL_EXECUTABLE_RUNTIME)
	if(app.D3D) {
		::llc::sleep(1);
		rtrn 0;
	}
#endif
	::llc::lock_guard locked(app.Lock);
	rtrn ::sampleGUIViewportDrawFrame(app);
}

LLC_DEFINE_APPLICATION_ENTRY_POINT(SApplication, LLC_SAMPLE_GUI_VIEWPORT_TITLE)
