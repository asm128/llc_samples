#include "llc_sample_geometry.h"

#include "llc_app_impl.h"
#include "llc_apod_serialize.h"
#include "llc_framework.h"
#include "llc_engine_shader.h"
#include "llc_runtime.h"
#include "llc_system_key.h"

#include <mutex>

#if defined(LLC_WINDOWS)
#	include "llc_engine_d3d_app.h"
#endif

stacxpr ::llc::bgra SAMPLE_CLEAR_COLOR = {24, 20, 16, 255};


sttc ::llc::err_t sampleGeometryViewportSize(SSampleGeometry & app, ::llc::u0_t iViewport) {
	::llc::SViewport & viewport = app.Viewports[iViewport];
	::llc::SControlPlacement & root = app.GUI.Controls.Placement[viewport.IdControl];
	app.GUI.Controls.Placement[viewport.IdControls[::llc::VIEWPORT_CONTROL_TARGET]].Area.Size = root.Area.Size - ::llc::n2s1_t{8, 32};
	rtrn ::llc::controlMetricsInvalidate(app.GUI, viewport.IdControl);
}

sttc ::llc::err_t sampleGeometryLayout(SSampleGeometry & app) {
	cnst ::llc::n2f3_t scale = app.GUI.Zoom.DPI * app.GUI.Zoom.ZoomLevel;
	cnst ::llc::n2s1_t cell = {(::llc::s1_t)::llc::max(180., (app.Window.Size.x / scale.x - 40) / 4), (::llc::s1_t)::llc::max(140., (app.Window.Size.y / scale.y - 140) / 2)};
	app.Maximized = -1;
	for(::llc::u0_t i = 0; i < SAMPLE_GEOMETRY_COUNT; ++i) {
		::llc::SViewport & viewport = app.Viewports[i];
		if(viewport.Folded)
			if_fail_fe(::llc::viewportFold(app.GUI, viewport));
		app.GUI.Controls.SetHidden(viewport.IdControl, false);
		app.GUI.Controls.Placement[viewport.IdControl].Area = {{(::llc::s1_t)(20 + i % 4 * cell.x), (::llc::s1_t)(76 + i / 4 * cell.y)}, cell - ::llc::n2s1_t{16, 16}};
		if_fail_fe(::sampleGeometryViewportSize(app, i));
	}
	rtrn 0;
}

sttc ::llc::err_t sampleGeometryMaximizedSize(SSampleGeometry & app) {
	if(app.Maximized < 0)
		rtrn 0;
	cnst ::llc::n2f3_t scale = app.GUI.Zoom.DPI * app.GUI.Zoom.ZoomLevel;
	app.GUI.Controls.Placement[app.Viewports[app.Maximized].IdControl].Area = {{20, 76}
		, {(::llc::s1_t)::llc::max(160., app.Window.Size.x / scale.x - 40), (::llc::s1_t)::llc::max(100., app.Window.Size.y / scale.y - 140)}};
	rtrn ::sampleGeometryViewportSize(app, (::llc::u0_t)app.Maximized);
}

sttc ::llc::err_t sampleGeometryMaximize(SSampleGeometry & app, ::llc::u0_t iViewport) {
	cnst bool restore = app.Maximized == iViewport;
	if(app.Maximized >= 0) {
		app.GUI.Controls.Placement[app.Viewports[app.Maximized].IdControl].Area = app.RestoreArea;
		if_fail_fe(::sampleGeometryViewportSize(app, (::llc::u0_t)app.Maximized));
		app.Maximized = -1;
	}
	if(!restore) {
		::llc::SViewport & viewport = app.Viewports[iViewport];
		if(viewport.Folded)
			if_fail_fe(::llc::viewportFold(app.GUI, viewport));
		app.RestoreArea = app.GUI.Controls.Placement[viewport.IdControl].Area;
		app.Maximized = iViewport;
		if_fail_fe(::llc::viewportActivate(app.GUI, app.Desktop, app.Viewports, iViewport));
	}
	rtrn ::sampleGeometryMaximizedSize(app);
}

sttc ::llc::err_t sampleGeometryScaleSet(SSampleGeometry & app, double value) {
	::llc::SGUI & gui = app.GUI;
	gui.Zoom.ZoomLevel = ::llc::clamped(value, .5, 2.);
	cnst ::llc::SControlPlacement & root = gui.Controls.Placement[app.Scale];
	::llc::SControlPlacement & thumb = gui.Controls.Placement[app.ScaleThumb];
	cnst double travel = root.Area.Size.x - ::llc::controlNCSpacing(root).x - thumb.Area.Size.x;
	thumb.Area.Offset.x = (::llc::s1_t)(travel * (gui.Zoom.ZoomLevel - .5) / 1.5);
	cnst ::llc::err_t length = ::llc::sprintf_s(app.ScaleText.Storage, "%.0f%%", gui.Zoom.ZoomLevel * 100);
	if_fail_fe(length);
	if_fail_fe(::llc::controlTextSet(gui, app.ScaleThumb, {app.ScaleText.Storage, (::llc::u2_t)length}));
	if_fail_fe(::sampleGeometryMaximizedSize(app));
	rtrn ::llc::guiUpdateMetrics(gui, app.Window.Size, true);
}

sttc ::llc::err_t sampleGeometryGUISetup(SSampleGeometry & app) {
	::llc::SGUI & gui = app.GUI;
	gui.SelectedFont = 8;
	if_fail_fe(app.Desktop = ::llc::controlCreate(gui));
	gui.Controls.Placement[app.Desktop].Border = gui.Controls.Placement[app.Desktop].Margin = {};
	gui.Controls.Draw[app.Desktop].NoBorder = gui.Controls.Draw[app.Desktop].NoClient = true;
	gui.Controls.Constraints[app.Desktop].AttachSizeToControl = {app.Desktop, app.Desktop};
	// Keep palette additions local; the GUI defaults are shared by other consumers.
	::llc::pobj<::llc::SGUIColors> colors;
	if_null_ve(::llc::OS_NO_MEMORY, colors.create(*gui.Colors));
	if_null_ve(::llc::OS_NO_MEMORY, colors->Palette.create(*gui.Colors->Palette));
	if_null_ve(::llc::OS_NO_MEMORY, colors->ControlThemes.create(*gui.Colors->ControlThemes));
	cnst ::llc::err_t transparent = colors->Palette->push_back({0, 0, 0, 0});
	if_fail_fe(transparent);
	::llc::SControlTheme clientTheme = (*colors->ControlThemes)[gui.ThemeDefault];
	for(auto & combo : clientTheme.ColorCombos)
		combo[::llc::UI_CONTROL_AREA_CLIENT] = (::llc::u2_t)transparent;
	cnst ::llc::err_t theme = colors->ControlThemes->push_back(clientTheme);
	if_fail_fe(theme);
	app.ClientTheme = (::llc::s1_t)(theme + 1);
	gui.Colors = colors;

	stacxpr ::llc::vcst_t titles[SAMPLE_GEOMETRY_COUNT] = {"BOX", "SPHERE", "DISC", "CYLINDER", "GRID", "RING FLAT", "TUBE", "HELIX"};
	stacxpr ::llc::s1_t themes[] = {::llc::ASCII_COLOR_CYAN * 16 + 9, ::llc::ASCII_COLOR_YELLOW * 16 + 9, ::llc::ASCII_COLOR_LIGHTGREY * 16 + 9};
	for(::llc::u0_t i = 0; i < SAMPLE_GEOMETRY_COUNT; ++i) {
		::llc::SViewport & viewport = app.Viewports[i];
		if_fail_fe(::llc::viewportInitialize(gui, viewport));
		if_fail_fe(::llc::controlSetParent(gui, viewport.IdControl, app.Desktop));
		gui.Controls.Placement[viewport.IdControl].Border = {3, 3, 3, 3};
		gui.Controls.Draw[viewport.IdControl].ColorTheme = themes[i % 3];
		for(cnst ::llc::cid_t id : viewport.IdControls)
			gui.Controls.Draw[id].ColorTheme = themes[i % 3];
		if_fail_fe(::llc::controlTextSet(gui, viewport.IdControls[::llc::VIEWPORT_CONTROL_TITLE], titles[i]));
		cnst ::llc::cid_t client = viewport.IdControls[::llc::VIEWPORT_CONTROL_TARGET];
		gui.Controls.Draw[client].ColorTheme = app.ClientTheme;
		gui.Controls.Draw[client].NoBorder = true;
		gui.Controls.Images[client].ImageAlign = ::llc::ALIGN_CENTER;
	}
	::llc::cid_t help;
	if_fail_fe(help = ::llc::controlCreate(gui));
	gui.Controls.Placement[help].Area = {{20, 18}, {1500, 36}};
	if_fail_fe(::llc::controlTextSet(gui, help, "DRAG TITLE | RESIZE EDGES | DOUBLE-CLICK MAXIMIZES | R: RESET | F3: HUD | F4 OR C/D: RENDERER | ESC: EXIT"));
	if_fail_fe(app.Scale = ::llc::controlCreate(gui));
	gui.Controls.Placement[app.Scale].Area = {{20, 16}, {280, 32}};
	gui.Controls.Placement[app.Scale].Align = ::llc::ALIGN_BOTTOM_LEFT;
	if_fail_fe(::llc::controlTextSet(gui, app.Scale, "UI SCALE"));
	if_fail_fe(app.ScaleThumb = ::llc::controlCreateChild(gui, app.Scale));
	gui.Controls.Placement[app.ScaleThumb].Area.Size = {60, 28};
	gui.Controls.Constraints[app.ScaleThumb].AttachSizeToControl.y = app.ScaleThumb;
	if_fail_fe(app.Performance = ::llc::controlCreate(gui));
	gui.Controls.Placement[app.Performance].Area = {{20, 16}, {560, 32}};
	gui.Controls.Placement[app.Performance].Align = ::llc::ALIGN_BOTTOM_RIGHT;
	if_fail_fe(::sampleGeometryLayout(app));
	rtrn ::sampleGeometryScaleSet(app, 1);
}

sttc ::llc::err_t sampleGeometryGUIUpdate(SSampleGeometry & app, ::llc::vcst_t backend, const ::llc::SFrameMeter & frameMeter) {
	::llc::SGUI & gui = app.GUI;
	cnst ::llc::SInput & input = *app.Window.Input;
	if(input.KeyDown('R')) {
		if_fail_fe(::sampleGeometryLayout(app));
		app.DragControl = ::llc::CID_INVALID;
	}
	double zoom = gui.Zoom.ZoomLevel;
	cnst bool scaleFocused = gui.Controls.IsFocused(app.Scale) || gui.Controls.IsFocused(app.ScaleThumb);
	if(scaleFocused) {
		if(input.KeyDown(::llc::SYSTEM_KEY_Home)) zoom = .5;
		if(input.KeyDown(::llc::SYSTEM_KEY_End)) zoom = 2;
		if(input.KeyDown(::llc::SYSTEM_KEY_Left)) zoom -= .05;
		if(input.KeyDown(::llc::SYSTEM_KEY_Right)) zoom += .05;
	}
	if(scaleFocused || gui.Controls.IsHovered(app.Scale) || gui.Controls.IsHovered(app.ScaleThumb))
		zoom += input.MouseCurrent.Deltas.z > 0 ? .05 : input.MouseCurrent.Deltas.z < 0 ? -.05 : 0;
	if(!input.MouseCurrent.ButtonState[0])
		app.DragControl = ::llc::CID_INVALID;
	if(input.ButtonDown(0)) {
		app.DragOrigin = input.MouseCurrent.Position;
		app.DragScale = gui.Zoom.DPI * gui.Zoom.ZoomLevel;
		app.DragZoom = zoom;
		if(gui.Controls.IsPressed(app.ScaleThumb) || gui.Controls.IsPressed(app.Scale)) {
			gui.Controls.SetFocused(app.ScaleThumb, true);
			app.DragControl = app.ScaleThumb;
			if(gui.Controls.IsPressed(app.Scale)) {
				cnst auto & area = gui.Controls.Area[app.Scale].Client.Global;
				cnst double thumb = gui.Controls.Area[app.ScaleThumb].Total.Global.Size.x;
				app.DragZoom = zoom = .5 + 1.5 * ::llc::clamped((input.MouseCurrent.Position.x - area.Offset.x - thumb / 2) / ::llc::max(1., area.Size.x - thumb), 0., 1.);
			}
		}
	}
	if(app.DragControl == app.ScaleThumb) {
		cnst auto & root = gui.Controls.Placement[app.Scale];
		cnst double travel = (root.Area.Size.x - ::llc::controlNCSpacing(root).x - gui.Controls.Placement[app.ScaleThumb].Area.Size.x) * app.DragScale.x;
		zoom = app.DragZoom + 1.5 * (input.MouseCurrent.Position.x - app.DragOrigin.x) / ::llc::max(1., travel);
	}
	if(zoom != gui.Zoom.ZoomLevel)
		if_fail_fe(::sampleGeometryScaleSet(app, zoom));

	bool doubleClick = false;
	for(cnst auto & event : app.Window.EventQueue) {
		if(!event || event->Type != ::llc::SYSTEM_EVENT_Mouse)
			continue;
		::llc::SEView<::llc::EVENT_MOUSE> mouse = {};
		if_fail_fe(event->ExtractChild(mouse));
		if(mouse.Type == ::llc::EVENT_MOUSE_DoubleClick) {
			::llc::vcu0_t data = mouse.Data;
			::llc::u0_t button = 0;
			if_fail_fe(::llc::loadPOD(data, button));
			doubleClick |= button == 0;
		}
	}
	for(::llc::u0_t i = 0; i < SAMPLE_GEOMETRY_COUNT; ++i) {
		::llc::SViewport & viewport = app.Viewports[i];
		if(::llc::controlHidden(gui, viewport.IdControl))
			continue;
		cnst ::llc::cid_t title = viewport.IdControls[::llc::VIEWPORT_CONTROL_TITLE];
		if(::llc::viewportIsPressed(gui, viewport))
			if_fail_fe(::llc::viewportActivate(gui, app.Desktop, app.Viewports, i));
		if(gui.Controls.IsAction(viewport.IdControls[::llc::VIEWPORT_CONTROL_CLOSE])) {
			if(app.Maximized == i)
				if_fail_fe(::sampleGeometryMaximize(app, i));
			gui.Controls.SetHidden(viewport.IdControl, true);
			continue;
		}
		if(doubleClick && gui.Controls.IsHovered(title)) {
			app.DragControl = ::llc::CID_INVALID;
			if(input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Shift]) {
				if(app.Maximized == i)
					if_fail_fe(::sampleGeometryMaximize(app, i));
				if_fail_fe(::llc::viewportFold(gui, viewport));
			}
			else
				if_fail_fe(::sampleGeometryMaximize(app, i));
		}
		if(app.Maximized == i)
			continue;
		::llc::rect2s1_t & area = gui.Controls.Placement[viewport.IdControl].Area;
		for(::llc::u0_t handle = 0; handle <= ::llc::VIEWPORT_CONTROL_TITLE; ++handle) {
			cnst ::llc::cid_t id = viewport.IdControls[handle];
			if(viewport.Folded && handle != ::llc::VIEWPORT_CONTROL_TITLE)
				continue;
			if(!doubleClick && input.ButtonDown(0) && gui.Controls.IsPressed(id)) {
				app.DragControl = id;
				app.DragArea = area;
				if_fail_fe(::llc::viewportActivate(gui, app.Desktop, app.Viewports, i));
			}
			if(app.DragControl != id)
				continue;
			cnst ::llc::n2s1_t delta = { (::llc::s1_t)((input.MouseCurrent.Position.x - app.DragOrigin.x) / app.DragScale.x), (::llc::s1_t)((input.MouseCurrent.Position.y - app.DragOrigin.y) / app.DragScale.y)};
			area = app.DragArea;
			if(handle == ::llc::VIEWPORT_CONTROL_TITLE)
				area.Offset += delta;
			else {
				stacxpr ::llc::u0_t edges[] = {1, 2, 4, 8, 3, 6, 9, 12};
				cnst ::llc::u0_t edge = edges[handle];
				if(edge & 1) { area.Size.x = ::llc::max<::llc::s1_t>(160, app.DragArea.Size.x - delta.x); area.Offset.x += app.DragArea.Size.x - area.Size.x; }
				if(edge & 2) { area.Size.y = ::llc::max<::llc::s1_t>(100, app.DragArea.Size.y - delta.y); area.Offset.y += app.DragArea.Size.y - area.Size.y; }
				if(edge & 4) area.Size.x = ::llc::max<::llc::s1_t>(160, app.DragArea.Size.x + delta.x);
				if(edge & 8) area.Size.y = ::llc::max<::llc::s1_t>(100, app.DragArea.Size.y + delta.y);
			}
			if_fail_fe(viewport.Folded ? ::llc::controlMetricsInvalidate(gui, viewport.IdControl) : ::sampleGeometryViewportSize(app, i));
		}
	}
	if_fail_fe(::sampleGeometryMaximizedSize(app));
	cnst ::llc::err_t length = ::llc::sprintf_s(app.PerformanceText.Storage, "%.*s | %u FPS | %.2f ms avg | %ux%u | F3", LLC_VIEW_ARGS(backend), frameMeter.FramesPerSecond, frameMeter.AverageFrameTime * 1000, app.Window.Size.x, app.Window.Size.y);
	if_fail_fe(length);
	if_fail_fe(::llc::controlTextSet(gui, app.Performance, {app.PerformanceText.Storage, (::llc::u2_t)length}));
	rtrn ::llc::guiUpdateMetrics(gui, app.Window.Size, gui.LastSize != app.Window.Size || gui.LastZoom != gui.Zoom);
}
::llc::err_t sampleGeometrySetup(SSampleGeometry & app) {
	::llc::SParamsBox			paramsBox		= {};
	::llc::SParamsSphere			paramsSphere	= {};
	::llc::SParamsDisc			paramsDisc		= {};
	::llc::SParamsCylinderWall	paramsCylinder	= {};
	::llc::SParamsGrid			paramsGrid		= {};
	::llc::SParamsRingSide		paramsRing		= {};
	::llc::SParamsTube			paramsTube		= {};
	::llc::SParamsHelix			paramsHelix		= {};
	paramsSphere.CellCount	= {16, 12};
	paramsDisc.Slices		= 24;
	paramsCylinder.CellCount	= {20, 2};
	paramsGrid.CellCount		= {8, 8};
	paramsRing.Slices		= 24;
	paramsTube.CellCount		= {20, 8};
	paramsTube.RadiusYMin	= {.25f, .5f};
	paramsTube.RadiusYMax	= {.25f, .5f};
	paramsHelix.CellCount	= {18, 8};

	::llc::SEngine & engine = app.Engine;
	::llc::astatic<::llc::err_t, 8> entities = {};
	if_fail_fe(entities[0] = engine.CreateBox		(paramsBox));
	if_fail_fe(entities[1] = engine.CreateSphere	(paramsSphere));
	if_fail_fe(entities[2] = engine.CreateDisc		(paramsDisc));
	if_fail_fe(entities[3] = engine.CreateCylinder	(paramsCylinder));
	if_fail_fe(entities[4] = engine.CreateGrid		(paramsGrid));
	if_fail_fe(entities[5] = engine.CreateRingFlat	(paramsRing));
	if_fail_fe(entities[6] = engine.CreateTube		(paramsTube));
	if_fail_fe(entities[7] = engine.CreateHelix	(paramsHelix));

	for(::llc::u2_t iEntity = 0; iEntity < entities.size(); ++iEntity) {
		cnst ::llc::eid_t entity = (::llc::eid_t)entities[iEntity];
		app.Entities[iEntity] = entity;
		if_fail_fe(engine.SetShader(entity, ::llc::psTerrestrial, "psTerrestrial"));
		if_fail_fe(engine.SetPosition(entity, {((::llc::f2_t)(iEntity % 4) - 1.5f) * 2, 0, iEntity < 4 ? -1.5f : 1.5f}));
		if_fail_fe(engine.SetHidden(entity, true));
		if_fail_fe(engine.SetRotation(entity, {0, .6f, 0}));
	}
	if_fail_fe(::sampleGeometryGUISetup(app));
	rtrn engine.Update(0);
}

::llc::err_t sampleGeometryDraw(SSampleGeometry & app, ::llc::SWindow::TOffscreen & target) {
	::llc::SGUI & gui = app.GUI;
	if_fail_fe(::llc::guiUpdateMetrics(gui, target.Color.metrics().u1(), gui.LastSize != target.Color.metrics().u1() || gui.LastZoom != gui.Zoom));
	if_fail_fe(target.Color.View.fill(SAMPLE_CLEAR_COLOR));
	::llc::SEngineScene & scene = *app.Engine.Scene;
	for(::llc::u0_t i = 0; i < SAMPLE_GEOMETRY_COUNT; ++i) {
		cnst ::llc::cid_t client = app.Viewports[i].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
		if(::llc::controlHidden(gui, client))
			continue;
		cnst ::llc::n2s1_t size = gui.Controls.Area[client].Client.Global.Size;
		if(size.x <= 0 || size.y <= 0)
			continue;
		auto & image = app.Targets[i];
		if_fail_fe(image.resize(size.u2()));
		if_fail_fe(image.Color.View.fill(SAMPLE_CLEAR_COLOR));
		if_fail_fe(image.DepthStencil.View.fill((::llc::u2_t)-1));
		::llc::n3f2_t position;
		if_fail_fe(app.Engine.GetPosition(app.Entities[i], position));
		if_fail_fe(app.Engine.SetHidden(app.Entities[i], false));
		cnst ::llc::err_t result = ::llc::drawScene(image.Color.View, image.DepthStencil.View, scene.RenderCache, scene, app.Camera.Position + position, app.Camera.Target + position, {0, 1, 0}, app.NearFar, position + app.LightOffset);
		if_fail_fe(app.Engine.SetHidden(app.Entities[i], true));
		if_fail_fe(result);
		gui.Controls.Images[client].Image = image.Color.View;
	}
	rtrn ::llc::guiDraw(gui, target.Color.View);
}
#define LLC_SAMPLE_GEOMETRY_TITLE "LLC engine geometry"

stct SApplication {
	::llc::SFramework	Framework;
	SSampleGeometry		Sample;
	::std::mutex		Lock;
#if defined(LLC_WINDOWS)
	::llc::pobj<::llc::SD3DApplication> D3D;
	bool				UseD3D			= false;
	bool				COMInitialized	= false;
#endif
	bool				ShowPerformance	= true;

	SApplication(::llc::SRuntimeValues & runtimeValues)
		: Framework(runtimeValues, {}), Sample(Framework.RootWindow, *Framework.GUI) {
		if_fail_e(::llc::rasterFontDefaults(Sample.Engine.Scene->Graphics->Fonts));
	}
};

#if defined(LLC_WINDOWS)
sttc ::llc::err_t sampleGeometryRendererSet(SApplication & app, bool useD3D) {
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
	else {
		if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
		if_fail_fe(::sampleGeometryDraw(app.Sample, *window.BackBuffer));
	}
	app.UseD3D = useD3D;
	::SetWindowTextA(window.PlatformDetail.WindowHandle, useD3D
		? LLC_SAMPLE_GEOMETRY_TITLE " - D3D11 | F3: HUD | F4/C: software | Esc: exit"
		: LLC_SAMPLE_GEOMETRY_TITLE " - CPU | F3: HUD | F4/D: D3D11 | Esc: exit");
	rtrn 0;
}
#endif

::llc::err_t setup(SApplication & app) {
	::llc::SWindow & window = app.Framework.RootWindow;
	window.Size = {1600, 960};
	if_fail_fe(::sampleGeometrySetup(app.Sample));
	if_null_ve(::llc::OS_NO_MEMORY, window.BackBuffer.create());
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	if_fail_fe(::sampleGeometryDraw(app.Sample, *window.BackBuffer));
	if_fail_fe(::llc::mainWindowCreate(window, app.Framework.RuntimeValues.PlatformDetail, window.Input));
#if defined(LLC_WINDOWS)
	::SetWindowTextA(window.PlatformDetail.WindowHandle, LLC_SAMPLE_GEOMETRY_TITLE " - CPU | F3: HUD | F4/D: D3D11 | Esc: exit");
#endif
	rtrn 0;
}

::llc::err_t cleanup(SApplication & app) {
	::std::lock_guard lock(app.Lock);
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

sttc ::llc::err_t sampleGeometryDrawFrame(SApplication & app);

::llc::err_t update(SApplication & app, bool systemRequestedExit) {
	::std::lock_guard lock(app.Lock);
	if(systemRequestedExit)
		rtrn ::llc::APPLICATION_STATE_EXIT;
	cnst ::llc::err_t result = ::llc::updateFramework(app.Framework);
	if(result)
		rtrn result;
	::llc::SWindow & window = app.Framework.RootWindow;
	if(window.Closed || window.Input->KeyDown(::llc::SYSTEM_KEY_Escape))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if(!window.NoDraw && window.Size.x && window.Size.y) {
		if(window.Input->KeyDown(::llc::SYSTEM_KEY_F3)) {
			app.ShowPerformance = !app.ShowPerformance;
			app.Sample.GUI.Controls.SetHidden(app.Sample.Performance, !app.ShowPerformance);
		}
#if defined(LLC_WINDOWS)
		if(window.Input->KeyDown(::llc::SYSTEM_KEY_F4))
			if_fail_fe(::sampleGeometryRendererSet(app, !app.UseD3D));
		if(window.Input->KeyDown('C') && app.UseD3D)
			if_fail_fe(::sampleGeometryRendererSet(app, false));
		if(window.Input->KeyDown('D') && !app.UseD3D)
			if_fail_fe(::sampleGeometryRendererSet(app, true));
#endif
		cnst ::llc::vcst_t backend =
#if defined(LLC_WINDOWS)
			app.UseD3D ? ::llc::vcst_t{"D3D11"} : ::llc::vcst_t{"CPU"};
#else
			"CPU";
#endif
		if_fail_fe(::sampleGeometryGUIUpdate(app.Sample, backend, app.Framework.FrameInfo.FrameMeter));
		if_fail_fe(app.Sample.Engine.Update(app.Framework.FrameInfo.Seconds.LastFrame));
	}
#if defined(LLC_WINDOWS) && defined(LLC_AVOID_LOCAL_APPLICATION_MODEL_EXECUTABLE_RUNTIME)
	// lrt draws on a worker; keep the D3D presenter and its window/COM lifetime on the update thread.
	if(app.D3D)
		rtrn ::sampleGeometryDrawFrame(app);
#endif
	::llc::sleep(1);
	rtrn 0;
}

#if defined(LLC_WINDOWS)
sttc ::llc::err_t sampleGeometryDrawD3D(SApplication & app) {
	::llc::SD3DApplication & d3d = *app.D3D;
	SSampleGeometry & sample = app.Sample;
	::llc::SGUI & gui = sample.GUI;
	if_fail_fe(::llc::guiUpdateMetrics(gui, sample.Window.Size, gui.LastSize != sample.Window.Size || gui.LastZoom != gui.Zoom));
	auto & resources = *d3d.DeviceResources;
	auto context = resources.GetD3DDeviceContext();
	ID3D11RenderTargetView * target = resources.GetBackBufferRenderTargetView();
	context->OMSetRenderTargets(1, &target, resources.GetDepthStencilView());
	cnst ::llc::rgbaf clear = SAMPLE_CLEAR_COLOR;
	context->ClearRenderTargetView(target, &clear.r);
	for(cnst ::llc::cid_t root : gui.Controls.Children[sample.Desktop])
	for(::llc::u0_t i = 0; i < SAMPLE_GEOMETRY_COUNT; ++i) {
		if(sample.Viewports[i].IdControl != root)
			continue;
		cnst ::llc::cid_t client = sample.Viewports[i].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
		gui.Controls.Images[client].Image = {};
		if(::llc::controlHidden(gui, client))
			continue;
		cnst ::llc::rect2s1_t area = gui.Controls.Area[client].Client.Global;
		if(area.Size.x <= 0 || area.Size.y <= 0)
			continue;
		cnst D3D11_VIEWPORT viewport = {(float)area.Offset.x, (float)area.Offset.y, (float)area.Size.x, (float)area.Size.y, 0, 1};
		cnst D3D11_RECT rect = {area.Offset.x, area.Offset.y, area.Offset.x + area.Size.x, area.Offset.y + area.Size.y};
		context->ClearView(target, &clear.r, &rect, 1);
		context->ClearDepthStencilView(resources.GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
		context->RSSetViewports(1, &viewport);
		::llc::n3f2_t position;
		if_fail_fe(sample.Engine.GetPosition(sample.Entities[i], position));
		if_fail_fe(sample.Engine.SetHidden(sample.Entities[i], false));
		cnst ::llc::err_t result = ::llc::d3dDrawEngineScene(d3d.Scene, *sample.Engine.Scene, area.Size.u1(), position + sample.LightOffset, sample.Camera.Position + position, sample.Camera.Target + position, sample.NearFar);
		if_fail_fe(sample.Engine.SetHidden(sample.Entities[i], true));
		if_fail_fe(result);
	}
	cnst D3D11_VIEWPORT screen = resources.GetScreenViewport();
	context->RSSetViewports(1, &screen);
	rtrn ::llc::d3dGUIDraw(resources, d3d.GUIStuff, gui, sample.Window.Size);
}
#endif
sttc ::llc::err_t sampleGeometryDrawFrame(SApplication & app) {
	::llc::SWindow & window = app.Framework.RootWindow;
	if(window.NoDraw || 0 == window.Size.x || 0 == window.Size.y) {
		::llc::sleep(8);
		rtrn 0;
	}
#if defined(LLC_WINDOWS)
	if(app.UseD3D) {
		::llc::SD3DApplication & d3d = *app.D3D;
		if(d3d.GUIStuff.RenderTarget.metrics() != window.Size.u2())
			if_fail_fe(d3d.SetWindowSize(window.Size));
		d3d.Scene.ConstantBufferScene.Time = (::llc::f2_t)app.Framework.FrameInfo.Seconds.Total;
		if_fail_fe(::sampleGeometryDrawD3D(app));
		rtrn d3d.DeviceResources->Present();
	}
#endif
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	if_fail_fe(::sampleGeometryDraw(app.Sample, *window.BackBuffer));
#if defined(LLC_WINDOWS)
	if(app.D3D) {
		::llc::SD3DApplication & d3d = *app.D3D;
		if(d3d.GUIStuff.RenderTarget.metrics() != window.Size.u2())
			if_fail_fe(d3d.SetWindowSize(window.Size));
		// A flip-model window keeps using its presenter when the scene is rendered on the CPU.
		memcpy(d3d.GUIStuff.RenderTarget.begin(), window.BackBuffer->Color.begin(), window.BackBuffer->Color.byte_count());
		for(auto & pixel : d3d.GUIStuff.RenderTarget.Texels)
			pixel.a = 255; // The completed CPU frame is opaque, as it is when presented by GDI.
		auto & resources = *d3d.DeviceResources;
		auto context = resources.GetD3DDeviceContext();
		ID3D11RenderTargetView * target = resources.GetBackBufferRenderTargetView();
		context->OMSetRenderTargets(1, &target, resources.GetDepthStencilView());
		cnst D3D11_VIEWPORT viewport = resources.GetScreenViewport();
		context->RSSetViewports(1, &viewport);
		cnst ::llc::rgbaf clear = SAMPLE_CLEAR_COLOR;
		context->ClearRenderTargetView(target, &clear.r);
		if_fail_fe(::llc::d3dGUIDraw(resources, d3d.GUIStuff));
		rtrn resources.Present();
	}
#endif

	window.Repaint = true;
	rtrn 0;
}

::llc::err_t draw(SApplication & app) {
	::std::unique_lock lock(app.Lock);
#if defined(LLC_WINDOWS) && defined(LLC_AVOID_LOCAL_APPLICATION_MODEL_EXECUTABLE_RUNTIME)
	if(app.D3D) {
		lock.unlock();
		::llc::sleep(1);
		rtrn 0;
	}
#endif
	rtrn ::sampleGeometryDrawFrame(app);
}

LLC_DEFINE_APPLICATION_ENTRY_POINT(SApplication, LLC_SAMPLE_GEOMETRY_TITLE)
