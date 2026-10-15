#include "llc_sample_engine.h"
#include "llc_ascii_color.h"
#include "llc_grid_copy.h"
#include "llc_runtime.h"
#include "llc_system_key.h"

sttc ::llc::err_t shipGeometryBuildEngine(::llc::SGeometryBuffers & geometry) {
	::llc::SParamsHelix helix = {};
	helix.CellCount = {8, 8}; helix.Radius = {.15f, .15f}; helix.Origin = {0, 1.1f, 0};
	llc_necs(::llc::geometryBuildHelix(geometry, helix));
	::llc::SParamsCylinderWall cylinder = {};
	cylinder.CellCount = {8, 1}; cylinder.Radius = {.35f, .25f}; cylinder.Height = 1; cylinder.Origin = {0, 1, 0};
	llc_necs(::llc::geometryBuildCylinder(geometry, cylinder));
	cylinder.Reverse = true;
	return ::llc::geometryBuildCylinder(geometry, cylinder);
}

sttc ::llc::err_t shipGeometryBuildWafer(::llc::SGeometryBuffers & geometry, bool shotgun) {
	llc_necs(::shipGeometryBuildEngine(geometry));
	::llc::SParamsBox box = {};
	if(shotgun) {
		box.HalfSizes = {.15f, .75f, .15f};
		stacxpr ::llc::n3f2_t origins[] = {{.25f, .75f, .25f}, {.25f, .75f, -.25f}, {-.25f, .75f, .25f}, {-.25f, .75f, -.25f}};
		for(cnst ::llc::n3f2_t & origin : origins) { box.Origin = origin; llc_necs(::llc::geometryBuildBox(geometry, box)); }
	}
	else { box.HalfSizes = {.6f, .25f, .6f}; box.Origin = box.HalfSizes; llc_necs(::llc::geometryBuildBox(geometry, box)); }
	::llc::SParamsGrid grid = {};
	grid.CellCount = {1, 1}; grid.Size = {1.5f, 1.5f}; grid.Origin = {.75f, 0, .75f};
	llc_necs(::llc::geometryBuildGrid(geometry, grid));
	grid.Reverse = true;
	return ::llc::geometryBuildGrid(geometry, grid);
}

sttc ::llc::err_t shipGeometryBuildGun(::llc::SGeometryBuffers & geometry, bool shotgun) {
	llc_necs(::shipGeometryBuildEngine(geometry));
	::llc::SParamsSphere sphere = {};
	sphere.CellCount = {8, 5}; sphere.Radius = shotgun ? .35f : .7f;
	llc_necs(::llc::geometryBuildSphere(geometry, sphere));
	::llc::SParamsHelix figure = {};
	figure.CellCount = {8, 2}; figure.Radius = {1, 1};
	llc_necs(::llc::geometryBuildFigure0(geometry, figure));
	::llc::SParamsCylinderWall barrel = {};
	barrel.CellCount = {8, 2}; barrel.Radius = shotgun ? ::llc::minmaxf2_t{.25f, .125f} : ::llc::minmaxf2_t{.125f, .1f}; barrel.Origin.z = shotgun ? .25f : 0;
	llc_necs(::llc::geometryBuildCylinder(geometry, barrel)); barrel.Reverse = true; llc_necs(::llc::geometryBuildCylinder(geometry, barrel));
	if(shotgun) { barrel.Origin.z = -.25f; barrel.Reverse = false; llc_necs(::llc::geometryBuildCylinder(geometry, barrel)); barrel.Reverse = true; llc_necs(::llc::geometryBuildCylinder(geometry, barrel)); }
	return 0;
}

sttc ::llc::err_t shipGeometryBuildCannon(::llc::SGeometryBuffers & geometry) {
	llc_necs(::shipGeometryBuildEngine(geometry));
	::llc::SParamsSphere sphere = {}; sphere.CellCount = {8, 5}; sphere.Radius = .8f;
	llc_necs(::llc::geometryBuildSphere(geometry, sphere));
	::llc::SParamsTender tender = {}; tender.Origin = {0, -.5f, 0}; tender.Scale = {.25f, .8f, .25f};
	llc_necs(::llc::geometryBuildTender(geometry, tender)); tender.Scale.x *= -1;
	return ::llc::geometryBuildTender(geometry, tender);
}

sttc ::llc::err_t shipGeometryBuild(::llc::SGeometryBuffers & geometry, ::llc::u0_t type) {
	switch(type) {
	case SHIP_SAMPLE_GEOMETRY_Gun: return ::shipGeometryBuildGun(geometry, false);
	case SHIP_SAMPLE_GEOMETRY_Wafer: return ::shipGeometryBuildWafer(geometry, false);
	case SHIP_SAMPLE_GEOMETRY_Cannon: return ::shipGeometryBuildCannon(geometry);
	case SHIP_SAMPLE_GEOMETRY_Shotgun: return ::shipGeometryBuildGun(geometry, true);
	case SHIP_SAMPLE_GEOMETRY_WaferShotgun: return ::shipGeometryBuildWafer(geometry, true);
	default: return ::shipGeometryBuildEngine(geometry);
	}
}

sttc ::llc::err_t shipTextureSet(::llc::SEngine & engine, ::llc::eid_t entity, ::llc::u0_t seed) {
	cnst ::llc::u2_t renderNode = engine.GetRenderNode(entity);
	::llc::SSkin & skin = *engine.Scene->Graphics->Skins[engine.Scene->RenderNodes[renderNode].Skin];
	::llc::SSurface & surface = *engine.Scene->Graphics->Surfaces[skin.Textures[0]];
	surface.Desc.Dimensions = {32, 5};
	llc_necs(surface.Data.resize(surface.Desc.Dimensions.Area() * szof(::llc::bgra)));
	::llc::g8bgra image = {(::llc::bgra*)surface.Data.begin(), {(::llc::u2_t)surface.Desc.Dimensions.x, (::llc::u2_t)surface.Desc.Dimensions.y}};
	for(::llc::u2_t y = 0; y < image.metrics().y; ++y) {
		cnst ::llc::bgra color = ::llc::ASCII_PALETTE[(seed + y * 3) % ::llc::size(::llc::ASCII_PALETTE)];
		for(::llc::u2_t x = 0; x < image.metrics().x; ++x)
			image[y][x] = color * ((y % 2) ? (::llc::f2_t)::llc::max(.25, ::llc::sin(x * (1.0 / image.metrics().x * ::llc::math_pi))) : 1.0f);
	}
	skin.Material.Color.Diffuse = ::llc::WHITE;
	skin.Material.Color.Ambient = ::llc::WHITE * .15f;
	return 0;
}

sttc ::llc::err_t shipPartCreate(SSampleEngine & app, ::llc::u0_t type, ::llc::u0_t index) {
	::llc::eid_t entity;
	llc_necs(entity = app.Engine.CreateEntityFromGeometry("Galaxy Hell ship part", {1.5f, 1.5f, 1.5f}, true, type, app.GeometryCache, [type](::llc::SGeometryBuffers & geometry) { return ::shipGeometryBuild(geometry, type); }));
	cnst ::llc::f3_t angle = (::llc::f3_t)(::llc::math_2pi / app.ShipParts.size() * index);
	app.Engine.SetPosition(entity, {(::llc::f2_t)(::llc::cos(angle) * 3.0f), 0, (::llc::f2_t)(::llc::sin(angle) * 3.0f)});
	app.Engine.SetOrientation(entity, ::llc::quatf2_t{}.MakeFromEuler(0, -angle, 0).Normalize());
	llc_necs(app.Engine.SetParent(entity, app.ShipRoot));
	llc_necs(::shipTextureSet(app.Engine, entity, 2 + index * 2));
	app.ShipParts[index] = entity;
	return 0;
}

::llc::err_t sampleEngineSetup(SSampleEngine & app) {
	if_null_ve(::llc::OS_NO_MEMORY, app.Engine.Scene.create());
	if_null_ve(::llc::OS_NO_MEMORY, app.Engine.Scene->Graphics.create());
	::llc::SParamsSphere core = {}; core.CellCount = {16, 16}; core.Radius = .75f;
	llc_necs(app.ShipRoot = app.Engine.CreateSphere(core, "Galaxy Hell ship core"));
	llc_necs(::shipTextureSet(app.Engine, app.ShipRoot, 11));
	for(::llc::u0_t iPart = 0; iPart < app.ShipParts.size(); ++iPart) llc_necs(::shipPartCreate(app, iPart, iPart));
	return 0;
}

::llc::err_t sampleEngineUpdate(SSampleEngine & app, bool systemRequestedExit, double secondsLastFrame, double, double) {
	if(systemRequestedExit || app.Window.Input.KeyDown(::llc::SYSTEM_KEY_Escape)) return ::llc::APPLICATION_STATE_EXIT;
	app.Angle += (::llc::f3_t)(secondsLastFrame * .35);
	app.ModuleTime += (::llc::f3_t)secondsLastFrame;
	app.Engine.SetOrientation(app.ShipRoot, ::llc::quatf2_t{}.MakeFromEuler(.25f, app.Angle, 0).Normalize());
	for(::llc::u0_t iPart = 0; iPart < app.ShipParts.size(); ++iPart) {
		cnst ::llc::f3_t phase = (::llc::f3_t)(::llc::math_2pi / app.ShipParts.size() * iPart);
		cnst ::llc::f3_t orbit = phase + app.ModuleTime * (::llc::f3_t)::llc::math_2pi / SHIP_MODULE_ORBITAL_PERIOD;
		cnst ::llc::f3_t spin = app.ModuleTime * (::llc::f3_t)::llc::math_2pi / SHIP_MODULE_ROTATION_PERIOD;
		app.Engine.SetPosition(app.ShipParts[iPart], {(::llc::f2_t)(::llc::cos(orbit) * SHIP_MODULE_ORBITAL_RADIUS), 0, (::llc::f2_t)(::llc::sin(orbit) * SHIP_MODULE_ORBITAL_RADIUS)});
		app.Engine.SetOrientation(app.ShipParts[iPart], ::llc::quatf2_t{}.MakeFromEuler(0, spin - orbit, 0).Normalize());
	}
	return app.Engine.Update(secondsLastFrame);
}

::llc::err_t sampleEngineDraw(SSampleEngine & app, ::llc::img8bgra (&)[SAMPLE_ENGINE_VIEWPORT_COUNT], ::llc::g8bgra target) {
	llc_necs(app.Color.resize(target.metrics())); llc_necs(app.Depth.resize(target.metrics()));
	app.Color.View.fill({8, 12, 16, 255}); app.Depth.View.fill((::llc::u2_t)-1);
	llc_necs(::llc::drawScene(app.Color.View, app.Depth.View, app.Engine.Scene->RenderCache, *app.Engine.Scene, {0, 5, -12}, {}, {0, 1, 0}, {.01f, 100.0f}));
	return ::llc::grid_copy(target, app.Color.View);
}
