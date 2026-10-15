#include "llc_sample_solar_system.h"
#include "llc_grid_copy.h"
#include "llc_runtime.h"
#include "llc_system_key.h"

stxp ::llc::vcst_t SOLAR_SYSTEM_DATA_FOLDER	= LLC_CXS("../llc_samples_data/llc_solar_system");
stxp ::llc::vcst_t SOLAR_SYSTEM_JSON			= LLC_CXS("llc_solar_system.json");
stxp ::llc::vcst_t SOLAR_SYSTEM_BACKGROUND		= LLC_CXS("black.png");

::llc::err_t sampleSolarSystemSetup(SSampleSolarSystem & app) {
	if_null_ve(::llc::OS_NO_MEMORY, app.Engine.Scene.create());
	if_null_ve(::llc::OS_NO_MEMORY, app.Engine.Scene->Graphics.create());
	llc_necs(::llc::planetarySystemSetup(app.SolarSystem, app.Engine, SOLAR_SYSTEM_JSON, SOLAR_SYSTEM_DATA_FOLDER, SOLAR_SYSTEM_BACKGROUND));
	const int32_t				iEarth					= app.SolarSystem.PlanetarySystem.Body.Keys.find("Earth");
	if(iEarth >= 0) {
		app.SelectedBody			= iEarth;
		app.SelectedPlanet		= iEarth;
	}
	return 0;
}

static ::llc::err_t selectBody(SSampleSolarSystem & app, uint32_t iBody) {
	if(iBody >= app.SolarSystem.PlanetarySystem.Body.size())
		return 0;
	app.SelectedBody			= iBody;
	const ::llc::eid_t		iEntity					= app.SolarSystem.EntityMap.Bodies[iBody];
	const uint32_t				iNode					= app.Engine.GetRenderNode(iEntity);
	const ::llc::n3f2_t		bodySize				= app.Engine.Scene->RenderNodes.BaseTransforms[iNode].NodeSize;
	app.CameraDistance		= ::llc::max(.01f, ::llc::max(bodySize.x, ::llc::max(bodySize.y, bodySize.z)) * 3);
	if(app.SolarSystem.PlanetarySystem.Type[iBody] == ::llc::CELESTIAL_BODY_Planet)
		app.SelectedPlanet		= iBody;
	return 0;
}

static ::llc::err_t selectBodyType(SSampleSolarSystem & app, ::llc::CELESTIAL_BODY type) {
	if(type == ::llc::CELESTIAL_BODY_Planet && app.SelectedPlanet < app.SolarSystem.PlanetarySystem.Type.size())
		return ::selectBody(app, app.SelectedPlanet);
	for(uint32_t iBody = 0; iBody < app.SolarSystem.PlanetarySystem.Type.size(); ++iBody)
		if(app.SolarSystem.PlanetarySystem.Type[iBody] == type)
			return ::selectBody(app, iBody);
	return 0;
}

::llc::err_t sampleSolarSystemUpdate(SSampleSolarSystem & app, bool systemRequestedExit, double secondsLastFrame, double secondsUpdate, double secondsDraw) {
	if(systemRequestedExit || app.Window.Input.KeyDown(::llc::SYSTEM_KEY_Escape))
		return ::llc::APPLICATION_STATE_EXIT;
	for(uint32_t iBody = 0, iPlanet = 0; iBody < app.SolarSystem.PlanetarySystem.Type.size(); ++iBody)
		if(app.SolarSystem.PlanetarySystem.Type[iBody] == ::llc::CELESTIAL_BODY_Planet && iPlanet < 9 && app.Window.Input.KeyDown((::llc::u0_t)('1' + iPlanet++)))
			llc_necs(::selectBody(app, iBody));
	if(app.Window.Input.KeyDown(::llc::SYSTEM_KEY_Tab)) {
		const bool				reverse					= app.Window.Input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Shift];
		const ::llc::CELESTIAL_BODY selectedType		= app.SolarSystem.PlanetarySystem.Type[app.SelectedBody];
		const ::llc::CELESTIAL_BODY nextType			= reverse
			? (selectedType == ::llc::CELESTIAL_BODY_Planet ? ::llc::CELESTIAL_BODY_Star : selectedType == ::llc::CELESTIAL_BODY_Star ? ::llc::CELESTIAL_BODY_Moon : ::llc::CELESTIAL_BODY_Planet)
			: (selectedType == ::llc::CELESTIAL_BODY_Planet ? ::llc::CELESTIAL_BODY_Moon : selectedType == ::llc::CELESTIAL_BODY_Moon ? ::llc::CELESTIAL_BODY_Star : ::llc::CELESTIAL_BODY_Planet);
		llc_necs(::selectBodyType(app, nextType));
	}
	if(app.Window.Input.KeyDown('P'))
		app.Paused				= !app.Paused;
	if(app.Window.Input.KeyDown('R'))
		app.TimeScale			= 1;
	if(app.Window.Input.KeyDown('0'))
		app.TimeScale			= .1f;
	if(app.Window.Input.KeyDown(::llc::SYSTEM_KEY_Add))
		app.TimeScale			*= 10;
	if(app.Window.Input.KeyDown(::llc::SYSTEM_KEY_Subtract))
		app.TimeScale			*= .1f;
	const bool				zoomFast					= app.Window.Input.KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Shift];
	const float				zoomDirection				= (app.Window.Input.KeyboardCurrent.KeyState['W'] ? 1.0f : 0.0f) - (app.Window.Input.KeyboardCurrent.KeyState['S'] ? 1.0f : 0.0f);
	const float				zoomSteps					= app.Window.Input.MouseCurrent.Deltas.z / 120.0f + zoomDirection * (float)secondsLastFrame * (zoomFast ? 40.0f : 4.0f);
	app.CameraDistance		*= (float)::pow(.85, zoomSteps);
	if(app.CameraDistance < .001f)
		app.CameraDistance = .001f;
	else if(app.CameraDistance > 200)
		app.CameraDistance = 200;
	if(secondsLastFrame > 0)
		llc_necs(::llc::sprintf_s(app.PerformanceText.Storage, "FPS %.2f   Frame %.2f ms   Update %.2f ms   Draw %.2f ms   Zoom %.2f", 1.0 / secondsLastFrame, secondsLastFrame * 1000.0, secondsUpdate * 1000.0, secondsDraw * 1000.0, app.CameraDistance));
	app.CameraAngle += (::llc::f3_t)(secondsLastFrame * .05);
	const double				secondsSimulation		= app.Paused ? 0 : secondsLastFrame * app.TimeScale;
	app.Time += (::llc::f3_t)secondsSimulation;
	return app.Engine.Update(secondsSimulation);
}

::llc::err_t sampleSolarSystemDraw(SSampleSolarSystem & app, ::llc::img8bgra (&)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::g8bgra target) {
	llc_necs(app.Color.resize(target.metrics()));
	llc_necs(app.Depth.resize(target.metrics()));
	app.Color.View.fill({2, 3, 8, 255});
	app.Depth.View.fill((::llc::u2_t)-1);
	::llc::n3f2_t			cameraPosition				= {};
	::llc::n3f2_t			cameraTarget				= {};
	llc_necs(::sampleSolarSystemCamera(app, cameraPosition, cameraTarget));
	llc_necs(::llc::drawScene(app.Color.View, app.Depth.View, app.Engine.Scene->RenderCache, *app.Engine.Scene, cameraPosition, cameraTarget, {0, 1, 0}, {.00001f, 1000.0f}, app.Time));
	return ::llc::grid_copy(target, app.Color.View);
}

::llc::err_t sampleSolarSystemCamera(const SSampleSolarSystem & app, ::llc::n3f2_t & cameraPosition, ::llc::n3f2_t & cameraTarget) {
	cameraTarget				= {};
	if(app.SelectedBody < app.SolarSystem.EntityMap.Bodies.size()) {
		const ::llc::eid_t		iEntity					= app.SolarSystem.EntityMap.Bodies[app.SelectedBody];
		const uint32_t				iNode					= app.Engine.GetRenderNode(iEntity);
		cameraTarget				= app.Engine.Scene->RenderNodes.Transforms[iNode].Model.GetTranslation();
	}
	cameraPosition			= cameraTarget + ::llc::n3f2_t{(::llc::f2_t)(::llc::sin(app.CameraAngle) * app.CameraDistance), (::llc::f2_t)(app.CameraDistance * .5), (::llc::f2_t)(::llc::cos(app.CameraAngle) * -app.CameraDistance)};
	return 0;
}
