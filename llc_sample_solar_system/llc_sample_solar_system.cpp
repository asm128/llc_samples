#include "llc_sample_solar_system.h"
#include "llc_app_impl.h"
#include "llc_framework.h"
#include "llc_runtime.h"
#include "llc_system_key.h"

#include <mutex>

#if defined(LLC_WINDOWS)
#	include "llc_engine_d3d_app.h"
#endif

stxp ::llc::vcst_t SOLAR_SYSTEM_DATA_FOLDER = "../llc_samples_data/llc_solar_system";
stxp ::llc::vcst_t SOLAR_SYSTEM_JSON = "llc_solar_system.json";
stacxpr ::llc::bgra SAMPLE_CLEAR_COLOR = {2, 3, 8, 255};

::llc::err_t sampleSolarSystemSetup(SSampleSolarSystem & app) {
	llc_necs(::llc::planetarySystemSetup(app.SolarSystem, app.Engine, SOLAR_SYSTEM_JSON, SOLAR_SYSTEM_DATA_FOLDER));
	app.Camera.Up = {0, 1, 0};
	app.Camera.NearFar = {.00001f, 1000.0f};
	cnst ::llc::s2_t iEarth = app.SolarSystem.PlanetarySystem.Body.Keys.find("Earth");
	if(iEarth >= 0) {
		app.SelectedBody = iEarth;
		app.SelectedPlanet = iEarth;
	}
	llc_necs(app.Performance = ::llc::controlCreate(app.GUI));
	app.GUI.Controls.Placement[app.Performance].Area = {{0, 12}, {560, 30}};
	app.GUI.Controls.Placement[app.Performance].Align = ::llc::ALIGN_CENTER_TOP;
	app.GUI.Controls.Placement[app.Performance].Border = {1, 1, 1, 1};
	app.GUI.Controls.Placement[app.Performance].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[app.Performance].Text = "CPU | -- FPS | -- ms avg | F3";
	rtrn ::llc::guiUpdateMetrics(app.GUI, app.Window.Size, false);
}

sttc ::llc::err_t sampleSolarSystemPerformanceUpdate(SSampleSolarSystem & app, ::llc::vcst_t backend, const ::llc::SFrameMeter & frameMeter) {
	cnst ::llc::err_t length = ::llc::sprintf_s(app.PerformanceText.Storage, "%.*s | %u FPS | %.2f ms avg | %ux%u | F3", LLC_VIEW_ARGS(backend), frameMeter.FramesPerSecond, frameMeter.AverageFrameTime * 1000, app.Window.Size.x, app.Window.Size.y);
	if_fail_fe(length);
	rtrn ::llc::controlTextSet(app.GUI, app.Performance, {app.PerformanceText.Storage, (::llc::u2_t)length});
}

sttc ::llc::err_t selectBody(SSampleSolarSystem & app, ::llc::u2_t iBody) {
	if(iBody >= app.SolarSystem.PlanetarySystem.Body.size())
		rtrn 0;
	app.SelectedBody = iBody;
	cnst ::llc::eid_t iEntity = app.SolarSystem.EntityMap.Bodies[iBody];
	cnst ::llc::u2_t iNode = app.Engine.GetRenderNode(iEntity);
	cnst ::llc::n3f2_t bodySize = app.Engine.Scene->RenderNodes.BaseTransforms[iNode].NodeSize;
	app.CameraDistance = ::llc::max(.01f, ::llc::max(bodySize.x, ::llc::max(bodySize.y, bodySize.z)) * 3);
	if(app.SolarSystem.PlanetarySystem.Type[iBody] == ::llc::CELESTIAL_BODY_Planet)
		app.SelectedPlanet = iBody;
	rtrn 0;
}

sttc ::llc::err_t selectBodyType(SSampleSolarSystem & app, ::llc::CELESTIAL_BODY type) {
	if(type == ::llc::CELESTIAL_BODY_Planet && app.SelectedPlanet < app.SolarSystem.PlanetarySystem.Type.size())
		rtrn ::selectBody(app, app.SelectedPlanet);
	for(::llc::u2_t iBody = 0; iBody < app.SolarSystem.PlanetarySystem.Type.size(); ++iBody)
		if(app.SolarSystem.PlanetarySystem.Type[iBody] == type)
			rtrn ::selectBody(app, iBody);
	rtrn 0;
}

::llc::err_t sampleSolarSystemUpdate(SSampleSolarSystem & app, bool systemRequestedExit, ::llc::f3_t secondsLastFrame) {
	if(systemRequestedExit || app.Window.Closed || app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Escape))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	for(::llc::u2_t iBody = 0, iPlanet = 0; iBody < app.SolarSystem.PlanetarySystem.Type.size(); ++iBody)
		if(app.SolarSystem.PlanetarySystem.Type[iBody] == ::llc::CELESTIAL_BODY_Planet && iPlanet < 9 && app.Window.Input->KeyDown((::llc::u0_t)('1' + iPlanet++)))
			llc_necs(::selectBody(app, iBody));
	if(app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Tab) && app.SelectedBody < app.SolarSystem.PlanetarySystem.Type.size()) {
		cnst bool reverse = app.Window.Input->KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Shift];
		cnst ::llc::CELESTIAL_BODY selectedType = app.SolarSystem.PlanetarySystem.Type[app.SelectedBody];
		cnst ::llc::CELESTIAL_BODY nextType = reverse
			? (selectedType == ::llc::CELESTIAL_BODY_Planet ? ::llc::CELESTIAL_BODY_Star : selectedType == ::llc::CELESTIAL_BODY_Star ? ::llc::CELESTIAL_BODY_Moon : ::llc::CELESTIAL_BODY_Planet)
			: (selectedType == ::llc::CELESTIAL_BODY_Planet ? ::llc::CELESTIAL_BODY_Moon : selectedType == ::llc::CELESTIAL_BODY_Moon ? ::llc::CELESTIAL_BODY_Star : ::llc::CELESTIAL_BODY_Planet);
		llc_necs(::selectBodyType(app, nextType));
	}
	if(app.Window.Input->KeyDown('P'))
		app.Paused = !app.Paused;
	if(app.Window.Input->KeyDown('R'))
		app.TimeScale = 1;
	if(app.Window.Input->KeyDown('0'))
		app.TimeScale = .1f;
	if(app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Add))
		app.TimeScale *= 10;
	if(app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Subtract))
		app.TimeScale *= .1f;
	if(app.Window.Input->KeyDown('T'))
		app.CameraOrbit = !app.CameraOrbit;
	cnst ::llc::SInput & input = *app.Window.Input;
	cnst bool moveFast = input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Shift];
	if(app.CameraOrbit) {
		cnst ::llc::f2_t zoomDirection = (input.KeyboardCurrent.KeyState['W'] ? 1.0f : 0.0f) - (input.KeyboardCurrent.KeyState['S'] ? 1.0f : 0.0f);
		cnst ::llc::f2_t zoomSteps = input.MouseCurrent.Deltas.z / 120.0f + zoomDirection * (::llc::f2_t)secondsLastFrame * (moveFast ? 40.0f : 4.0f);
		app.CameraDistance *= (::llc::f2_t)::pow(.85, zoomSteps);
		app.CameraDistance = ::llc::min((::llc::f3_t)200.0, ::llc::max((::llc::f3_t).001, app.CameraDistance));
		app.CameraAngle += (::llc::f3_t)(secondsLastFrame * .05);
	}
	else {
		cnst ::llc::f2_t moveScale = (::llc::f2_t)secondsLastFrame * (moveFast ? 40.0f : 4.0f);
		cnst ::llc::f2_t moveForward = (input.KeyboardCurrent.KeyState['W'] ? 1.0f : 0.0f) - (input.KeyboardCurrent.KeyState['S'] ? 1.0f : 0.0f);
		cnst ::llc::f2_t moveRight = (input.KeyboardCurrent.KeyState['D'] ? 1.0f : 0.0f) - (input.KeyboardCurrent.KeyState['A'] ? 1.0f : 0.0f);
		cnst ::llc::f2_t moveUp = ((input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Space] || input.KeyboardCurrent.KeyState['Q']) ? 1.0f : 0.0f)
			- ((input.KeyboardCurrent.KeyState['C'] || input.KeyboardCurrent.KeyState['Z']) ? 1.0f : 0.0f);
		::llc::n3f2_t cameraDelta = {};
		cameraDelta.AddScaled(app.Camera.Front, moveForward * moveScale);
		cameraDelta.AddScaled(app.Camera.Right, moveRight * moveScale);
		cameraDelta.AddScaled(app.Camera.Up, moveUp * moveScale);
		app.Camera.Position += cameraDelta;
		app.Camera.Target += cameraDelta;
	}
	cnst ::llc::f3_t secondsSimulation = app.Paused ? 0 : secondsLastFrame * app.TimeScale;
	app.Time += secondsSimulation;
	rtrn app.Engine.Update(secondsSimulation);
}

::llc::err_t sampleSolarSystemCamera(SSampleSolarSystem & app) {
	if(app.CameraOrbit) {
		app.Camera.Target = {};
		if(app.SelectedBody < app.SolarSystem.EntityMap.Bodies.size()) {
			cnst ::llc::eid_t iEntity = app.SolarSystem.EntityMap.Bodies[app.SelectedBody];
			cnst ::llc::u2_t iNode = app.Engine.GetRenderNode(iEntity);
			app.Camera.Target = app.Engine.Scene->RenderNodes.Transforms[iNode].Model.GetTranslation();
		}
		app.Camera.Position = app.Camera.Target + ::llc::n3f2_t
			{ (::llc::f2_t)(::llc::sin(app.CameraAngle) * app.CameraDistance)
			, (::llc::f2_t)(app.CameraDistance * .5)
			, (::llc::f2_t)(::llc::cos(app.CameraAngle) * -app.CameraDistance)
			};
	}
	app.Camera.Front = (app.Camera.Target - app.Camera.Position).Normalized();
	app.Camera.Up = {0, 1, 0};
	app.Camera.Right = app.Camera.Up.Cross(app.Camera.Front).Normalized();
	rtrn 0;
}

::llc::err_t sampleSolarSystemDraw(SSampleSolarSystem & app, ::llc::SWindow::TOffscreen & backBuffer) {
	if_fail_fe(::llc::guiUpdateMetrics(app.GUI, backBuffer.Color.metrics().u1(), app.GUI.LastSize != backBuffer.Color.metrics().u1() || app.GUI.LastZoom != app.GUI.Zoom));
	backBuffer.Color.View.fill(SAMPLE_CLEAR_COLOR);
	backBuffer.DepthStencil.View.fill((::llc::u2_t)-1);
	llc_necs(::sampleSolarSystemCamera(app));
	llc_necs(::llc::drawScene(backBuffer.Color.View, backBuffer.DepthStencil.View, app.Engine.Scene->RenderCache, *app.Engine.Scene, app.Camera.Position, app.Camera.Target, app.Camera.Up, app.Camera.NearFar, {}, app.Time));
	rtrn ::llc::guiDraw(app.GUI, backBuffer.Color.View);
}

#define LLC_SAMPLE_SOLAR_SYSTEM_TITLE "LLC solar system"

stct SApplication {
	::llc::SFramework	Framework;
	SSampleSolarSystem	Sample;
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
sttc ::llc::err_t sampleSolarSystemRendererSet(SApplication & app, bool useD3D) {
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
		if_fail_fe(::sampleSolarSystemDraw(app.Sample, *window.BackBuffer));
	}
	app.UseD3D = useD3D;
	::SetWindowTextA(window.PlatformDetail.WindowHandle, useD3D
		? LLC_SAMPLE_SOLAR_SYSTEM_TITLE " - D3D11 | F3: HUD | F4/F1: CPU | Esc: exit"
		: LLC_SAMPLE_SOLAR_SYSTEM_TITLE " - CPU | F3: HUD | F4/F2: D3D11 | Esc: exit");
	rtrn 0;
}
#endif

::llc::err_t setup(SApplication & app) {
	::llc::SWindow & window = app.Framework.RootWindow;
	window.Size = {1600, 960};
	if_null_ve(::llc::OS_NO_MEMORY, window.BackBuffer.create());
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	if_fail_fe(::llc::mainWindowCreate(window, app.Framework.RuntimeValues.PlatformDetail, window.Input));
	if_fail_fe(::sampleSolarSystemSetup(app.Sample));
#if defined(LLC_WINDOWS)
	::SetWindowTextA(window.PlatformDetail.WindowHandle, LLC_SAMPLE_SOLAR_SYSTEM_TITLE " - CPU | F3: HUD | F4/F2: D3D11 | Esc: exit");
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

sttc ::llc::err_t sampleSolarSystemDrawFrame(SApplication & app);

::llc::err_t update(SApplication & app, bool systemRequestedExit) {
	::std::lock_guard lock(app.Lock);
	if(systemRequestedExit)
		rtrn ::llc::APPLICATION_STATE_EXIT;
	cnst ::llc::err_t result = ::llc::updateFramework(app.Framework);
	if(result)
		rtrn result;
	::llc::SWindow & window = app.Framework.RootWindow;
#if defined(LLC_WINDOWS)
	if(!window.NoDraw && window.Size.x && window.Size.y) {
		if(window.Input->KeyDown(::llc::SYSTEM_KEY_F4))
			if_fail_fe(::sampleSolarSystemRendererSet(app, !app.UseD3D));
		if(window.Input->KeyDown(::llc::SYSTEM_KEY_F1) && app.UseD3D)
			if_fail_fe(::sampleSolarSystemRendererSet(app, false));
		if(window.Input->KeyDown(::llc::SYSTEM_KEY_F2) && !app.UseD3D)
			if_fail_fe(::sampleSolarSystemRendererSet(app, true));
	}
#endif
	if(window.Input->KeyDown(::llc::SYSTEM_KEY_F3)) {
		app.ShowPerformance = !app.ShowPerformance;
		app.Sample.GUI.Controls.SetHidden(app.Sample.Performance, !app.ShowPerformance);
	}
	cnst ::llc::err_t resultSample = ::sampleSolarSystemUpdate(app.Sample, false, app.Framework.FrameInfo.Seconds.LastFrame);
	cnst ::llc::vcst_t backend =
#if defined(LLC_WINDOWS)
		app.UseD3D ? ::llc::vcst_t{"D3D11"} : ::llc::vcst_t{"CPU"};
#else
		"CPU";
#endif
	if_fail_fe(::sampleSolarSystemPerformanceUpdate(app.Sample, backend, app.Framework.FrameInfo.FrameMeter));
#if defined(LLC_WINDOWS) && defined(LLC_AVOID_LOCAL_APPLICATION_MODEL_EXECUTABLE_RUNTIME)
	if(app.D3D)
		rtrn ::sampleSolarSystemDrawFrame(app);
#endif
	::llc::sleep(1);
	rtrn resultSample;
}

#if defined(LLC_WINDOWS)
sttc ::llc::err_t sampleSolarSystemDrawD3D(SApplication & app) {
	::llc::SD3DApplication & d3d = *app.D3D;
	SSampleSolarSystem & sample = app.Sample;
	::llc::SGUI & gui = sample.GUI;
	if_fail_fe(::sampleSolarSystemCamera(sample));
	if_fail_fe(::llc::guiUpdateMetrics(gui, sample.Window.Size, gui.LastSize != sample.Window.Size || gui.LastZoom != gui.Zoom));
	auto & resources = *d3d.DeviceResources;
	auto context = resources.GetD3DDeviceContext();
	ID3D11RenderTargetView * target = resources.GetBackBufferRenderTargetView();
	context->OMSetRenderTargets(1, &target, resources.GetDepthStencilView());
	cnst ::llc::rgbaf clear = SAMPLE_CLEAR_COLOR;
	context->ClearRenderTargetView(target, &clear.r);
	context->ClearDepthStencilView(resources.GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	cnst D3D11_VIEWPORT viewport = resources.GetScreenViewport();
	context->RSSetViewports(1, &viewport);
	if_fail_fe(::llc::d3dDrawEngineScene(d3d.Scene, *sample.Engine.Scene, sample.Window.Size.u1(), {}, sample.Camera.Position, sample.Camera.Target, sample.Camera.NearFar, sample.Time));
	rtrn ::llc::d3dGUIDraw(resources, d3d.GUIStuff, gui, sample.Window.Size);
}
#endif

sttc ::llc::err_t sampleSolarSystemDrawFrame(SApplication & app) {
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
		if_fail_fe(::sampleSolarSystemDrawD3D(app));
		rtrn d3d.DeviceResources->Present();
	}
#endif
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	if_fail_fe(::sampleSolarSystemDraw(app.Sample, *window.BackBuffer));
#if defined(LLC_WINDOWS)
	if(app.D3D) {
		::llc::SD3DApplication & d3d = *app.D3D;
		if(d3d.GUIStuff.RenderTarget.metrics() != window.Size.u2())
			if_fail_fe(d3d.SetWindowSize(window.Size));
		memcpy(d3d.GUIStuff.RenderTarget.begin(), window.BackBuffer->Color.begin(), window.BackBuffer->Color.byte_count());
		for(auto & pixel : d3d.GUIStuff.RenderTarget.Texels)
			pixel.a = 255;
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
	rtrn ::sampleSolarSystemDrawFrame(app);
}

LLC_DEFINE_APPLICATION_ENTRY_POINT(SApplication, LLC_SAMPLE_SOLAR_SYSTEM_TITLE)
