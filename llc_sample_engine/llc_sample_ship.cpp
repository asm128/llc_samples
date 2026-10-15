#include "llc_sample_ship.h"
#include "llc_ascii_color.h"

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

sttc ::llc::err_t shipTextureSet(SGalaxyHellShip & ship, ::llc::eid_t entity, ::llc::u0_t seed) {
	cnst ::llc::u2_t renderNode = ship.Engine.GetRenderNode(entity);
	::llc::SSkin & skin = *ship.Engine.Scene->Graphics->Skins[ship.Engine.Scene->RenderNodes[renderNode].Skin];
	::llc::SSurface & surface = *ship.Engine.Scene->Graphics->Surfaces[skin.Textures[0]];
	surface.Desc.Dimensions = {32, 5};
	llc_necs(surface.Data.resize(surface.Desc.Dimensions.Area() * szof(::llc::bgra)));
	::llc::g8bgra image = {(::llc::bgra*)surface.Data.begin(), {(::llc::u2_t)surface.Desc.Dimensions.x, (::llc::u2_t)surface.Desc.Dimensions.y}};
	for(::llc::u2_t y = 0; y < image.metrics().y; ++y) {
		cnst ::llc::bgra color = ::llc::ASCII_PALETTE[(seed + y * 3) % ::llc::size(::llc::ASCII_PALETTE)];
		for(::llc::u2_t x = 0; x < image.metrics().x; ++x)
			image[y][x] = color * ((y % 2) ? (::llc::f2_t)::llc::max(.25, ::llc::sin(x * (1.0 / image.metrics().x * ::llc::math_pi))) : 1.0f);
	}
	skin.Material.Color.Diffuse = ship.Constants.DiffuseColor;
	skin.Material.Color.Ambient = ship.Constants.AmbientColor;
	return 0;
}

sttc ::llc::err_t shipModuleCreate(SGalaxyHellShip & ship, ::llc::u0_t type, ::llc::u0_t index) {
	::llc::eid_t entity;
	llc_necs(entity = ship.Engine.CreateEntityFromGeometry("Galaxy Hell ship module", {1.5f, 1.5f, 1.5f}, true, type, ship.GeometryCache, [type](::llc::SGeometryBuffers & geometry) { return ::shipGeometryBuild(geometry, type); }));
	cnst ::llc::f3_t angle = (::llc::f3_t)(::llc::math_2pi / ship.ShipModules.size() * index);
	ship.Engine.SetPosition(entity, {(::llc::f2_t)(::llc::cos(angle) * ship.Constants.ModuleOrbitalRadius), 0, (::llc::f2_t)(::llc::sin(angle) * ship.Constants.ModuleOrbitalRadius)});
	ship.Engine.SetOrientation(entity, ::llc::quatf2_t{}.MakeFromEuler(0, -angle, 0).Normalize());
	llc_necs(ship.Engine.SetParent(entity, ship.ShipCore));
	llc_necs(::shipTextureSet(ship, entity, ship.Constants.TextureSeed + index * 2));
	ship.ShipModules[index] = entity;
	return 0;
}

::llc::err_t galaxyHellShipSetup(SGalaxyHellShip & ship) {
	if_null_ve(::llc::OS_NO_MEMORY, ship.Engine.Scene.create());
	if_null_ve(::llc::OS_NO_MEMORY, ship.Engine.Scene->Graphics.create());
	::llc::SParamsSphere core = {}; core.CellCount = {16, 16}; core.Radius = .75f;
	llc_necs(ship.ShipCore = ship.Engine.CreateSphere(core, "Galaxy Hell ship core"));
	llc_necs(::shipTextureSet(ship, ship.ShipCore, ship.Constants.TextureSeed + 9));
	for(::llc::u0_t iModule = 0; iModule < ship.ShipModules.size(); ++iModule)
		llc_necs(::shipModuleCreate(ship, iModule, iModule));
	return 0;
}

::llc::err_t galaxyHellShipUpdate(SGalaxyHellShip & ship, double secondsLastFrame) {
	ship.RootAngle += (::llc::f3_t)(secondsLastFrame * ship.Constants.RootRotationSpeed);
	ship.ModuleTime += (::llc::f3_t)secondsLastFrame;
	ship.Engine.SetOrientation(ship.ShipCore, ::llc::quatf2_t{}.MakeFromEuler(.25f, ship.RootAngle, 0).Normalize());
	for(::llc::u0_t iModule = 0; iModule < ship.ShipModules.size(); ++iModule) {
		cnst ::llc::f3_t phase = (::llc::f3_t)(::llc::math_2pi / ship.ShipModules.size() * iModule);
		cnst ::llc::f3_t orbit = phase + ship.ModuleTime * (::llc::f3_t)::llc::math_2pi / ship.Constants.ModuleOrbitalPeriod;
		cnst ::llc::f3_t spin = ship.ModuleTime * (::llc::f3_t)::llc::math_2pi / ship.Constants.ModuleRotationPeriod;
		ship.Engine.SetPosition(ship.ShipModules[iModule], {(::llc::f2_t)(::llc::cos(orbit) * ship.Constants.ModuleOrbitalRadius), 0, (::llc::f2_t)(::llc::sin(orbit) * ship.Constants.ModuleOrbitalRadius)});
		ship.Engine.SetOrientation(ship.ShipModules[iModule], ::llc::quatf2_t{}.MakeFromEuler(0, spin - orbit, 0).Normalize());
	}
	return ship.Engine.Update(secondsLastFrame);
}
