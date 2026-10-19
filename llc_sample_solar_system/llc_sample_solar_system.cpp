#include "llc_sample_solar_system.h"
#include "llc_app_impl.h"
#include "llc_framework.h"
#include "llc_runtime.h"
#include "llc_system_key.h"

stxp ::llc::vcst_t SOLAR_SYSTEM_JSON = LLC_CXS("llc_solar_system.json");

::llc::err_t sampleSolarSystemSetup(SSampleSolarSystem & app) {
	llc_necs(::llc::planetarySystemSetup(app.SolarSystem, app.Engine, SOLAR_SYSTEM_JSON));
	app.Camera.Up = {0, 1, 0};
	app.Camera.NearFar = {.00001f, 1000.0f};
	cnst ::llc::s2_t iEarth = app.SolarSystem.PlanetarySystem.Body.Keys.find("Earth");
	if(iEarth >= 0) {
		app.SelectedBody = iEarth;
		app.SelectedPlanet = iEarth;
	}
	rtrn 0;
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
	cnst bool zoomFast = app.Window.Input->KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Shift];
	cnst ::llc::f2_t zoomDirection = (app.Window.Input->KeyboardCurrent.KeyState['W'] ? 1.0f : 0.0f) - (app.Window.Input->KeyboardCurrent.KeyState['S'] ? 1.0f : 0.0f);
	cnst ::llc::f2_t zoomSteps = app.Window.Input->MouseCurrent.Deltas.z / 120.0f + zoomDirection * (::llc::f2_t)secondsLastFrame * (zoomFast ? 40.0f : 4.0f);
	app.CameraDistance *= (::llc::f2_t)::pow(.85, zoomSteps);
	app.CameraDistance = ::llc::min((::llc::f3_t)200.0, ::llc::max((::llc::f3_t).001, app.CameraDistance));
	app.CameraAngle += (::llc::f3_t)(secondsLastFrame * .05);
	cnst ::llc::f3_t secondsSimulation = app.Paused ? 0 : secondsLastFrame * app.TimeScale;
	rtrn app.Engine.Update(secondsSimulation);
}

::llc::err_t sampleSolarSystemCamera(SSampleSolarSystem & app) {
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
	rtrn 0;
}

::llc::err_t sampleSolarSystemDraw(SSampleSolarSystem & app, ::llc::SWindow::TOffscreen & backBuffer) {
	backBuffer.Color.View.fill({2, 3, 8, 255});
	backBuffer.DepthStencil.View.fill((::llc::u2_t)-1);
	llc_necs(::sampleSolarSystemCamera(app));
	rtrn ::llc::drawScene(backBuffer.Color.View, backBuffer.DepthStencil.View, app.Engine.Scene->RenderCache, *app.Engine.Scene, app.Camera.Position, app.Camera.Target, app.Camera.Up, app.Camera.NearFar);
}

#define LLC_SAMPLE_SOLAR_SYSTEM_TITLE "LLC solar system"

stct SApplication {
	::llc::SFramework	Framework;
	SSampleSolarSystem	Sample;

	SApplication(::llc::SRuntimeValues & runtimeValues)
		: Framework(runtimeValues, {}), Sample(Framework.RootWindow) {}
};

::llc::err_t setup(SApplication & app) {
	::llc::SWindow & window = app.Framework.RootWindow;
	window.Size = {1600, 960};
	if_null_ve(::llc::OS_NO_MEMORY, window.BackBuffer.create());
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	if_fail_fe(::llc::mainWindowCreate(window, app.Framework.RuntimeValues.PlatformDetail, window.Input));
	rtrn ::sampleSolarSystemSetup(app.Sample);
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
	cnst ::llc::err_t resultSample = ::sampleSolarSystemUpdate(app.Sample, false, app.Framework.FrameInfo.Seconds.LastFrame);
	::llc::sleep(1);
	rtrn resultSample;
}

::llc::err_t draw(SApplication & app) {
	::llc::SWindow & window = app.Framework.RootWindow;
	if(window.NoDraw) {
		::llc::sleep(8);
		rtrn 0;
	}
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	rtrn ::sampleSolarSystemDraw(app.Sample, *window.BackBuffer);
}

LLC_DEFINE_APPLICATION_ENTRY_POINT(SApplication, LLC_SAMPLE_SOLAR_SYSTEM_TITLE)
