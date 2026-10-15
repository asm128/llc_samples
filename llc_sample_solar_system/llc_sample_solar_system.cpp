#include "llc_sample_solar_system.h"
#include "llc_grid_copy.h"
#include "llc_runtime.h"
#include "llc_system_key.h"

stxp ::llc::vcst_t SOLAR_SYSTEM_DATA_FOLDER	= LLC_CXS("../llc_samples_data/llc_solar_system");
stxp ::llc::vcst_t SOLAR_SYSTEM_JSON			= LLC_CXS("llc_solar_system.json");

stxp ::llc::vcst_t SOLAR_SYSTEM_TEXTURES[] =
	{ LLC_CXS("sun_color.png")
	, LLC_CXS("mercury_color.png")
	, LLC_CXS("venus_color.png")
	, LLC_CXS("earth_color.png")
	, LLC_CXS("moon_color.png")
	, LLC_CXS("mars_color.png")
	, LLC_CXS("jupiter_color.png")
	, {}
	, LLC_CXS("uranus_color.png")
	, LLC_CXS("neptune_color.png")
	, LLC_CXS("pluto_color.png")
	, LLC_CXS("Io_color.png")
	, LLC_CXS("Ganymede_color.png")
	};

sttc ::llc::err_t solarSystemTextureSet(::llc::SEngine & engine, ::llc::eid_t entity, ::llc::vcst_t fileName) {
	if(0 == fileName.size())
		return 0;
	::llc::s2_t				iImage;
	llc_necs(iImage = engine.CreateImageFromFile(SOLAR_SYSTEM_DATA_FOLDER, fileName));
	cnst ::llc::u2_t		iRenderNode			= engine.GetRenderNode(entity);
	::llc::SSkin			& skin					= *engine.Scene->Graphics->Skins[engine.Scene->RenderNodes[iRenderNode].Skin];
	llc_necs(skin.Textures.insert(0, iImage));
	return 0;
}

::llc::err_t sampleSolarSystemSetup(SSampleSolarSystem & app) {
	if_null_ve(::llc::OS_NO_MEMORY, app.Engine.Scene.create());
	if_null_ve(::llc::OS_NO_MEMORY, app.Engine.Scene->Graphics.create());
	llc_necs(::llc::planetarySystemSetup(app.SolarSystem, app.Engine, SOLAR_SYSTEM_JSON));
	cnst ::llc::u2_t countTextures = ::llc::min(app.SolarSystem.EntityMap.Bodies.size(), (::llc::u2_t)::llc::size(SOLAR_SYSTEM_TEXTURES));
	for(::llc::u2_t iBody = 0; iBody < countTextures; ++iBody)
		llc_necs(::solarSystemTextureSet(app.Engine, app.SolarSystem.EntityMap.Bodies[iBody], SOLAR_SYSTEM_TEXTURES[iBody]));
	return 0;
}

::llc::err_t sampleSolarSystemUpdate(SSampleSolarSystem & app, bool systemRequestedExit, double secondsLastFrame, double, double) {
	if(systemRequestedExit || app.Window.Input.KeyDown(::llc::SYSTEM_KEY_Escape))
		return ::llc::APPLICATION_STATE_EXIT;
	app.CameraAngle += (::llc::f3_t)(secondsLastFrame * .05);
	app.Time += (::llc::f3_t)secondsLastFrame;
	return app.Engine.Update(secondsLastFrame);
}

::llc::err_t sampleSolarSystemDraw(SSampleSolarSystem & app, ::llc::img8bgra (&)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::g8bgra target) {
	llc_necs(app.Color.resize(target.metrics()));
	llc_necs(app.Depth.resize(target.metrics()));
	app.Color.View.fill({2, 3, 8, 255});
	app.Depth.View.fill((::llc::u2_t)-1);
	cnst ::llc::n3f2_t cameraPosition = {(::llc::f2_t)(::llc::sin(app.CameraAngle) * 24), 12, (::llc::f2_t)(::llc::cos(app.CameraAngle) * -24)};
	llc_necs(::llc::drawScene(app.Color.View, app.Depth.View, app.Engine.Scene->RenderCache, *app.Engine.Scene, cameraPosition, {}, {0, 1, 0}, {.01f, 1000.0f}, app.Time));
	return ::llc::grid_copy(target, app.Color.View);
}
