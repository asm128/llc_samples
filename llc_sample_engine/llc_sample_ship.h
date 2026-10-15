#include "llc_engine.h"

#ifndef LLC_SAMPLE_SHIP_H_23627
#define LLC_SAMPLE_SHIP_H_23627

enum SHIP_SAMPLE_GEOMETRY : ::llc::u0_t
	{ SHIP_SAMPLE_GEOMETRY_Gun
	, SHIP_SAMPLE_GEOMETRY_Wafer
	, SHIP_SAMPLE_GEOMETRY_Cannon
	, SHIP_SAMPLE_GEOMETRY_Shotgun
	, SHIP_SAMPLE_GEOMETRY_WaferShotgun
	, SHIP_SAMPLE_GEOMETRY_Engine
	, SHIP_SAMPLE_GEOMETRY_COUNT
	};

stct SGalaxyHellShipConstants {
	::llc::f3_t			RootRotationSpeed	= .35f;
	::llc::f3_t			ModuleOrbitalPeriod	= 6;
	::llc::f3_t			ModuleOrbitalRadius	= 3;
	::llc::f3_t			ModuleRotationPeriod	= 1;
	::llc::rgbaf			AmbientColor		= ::llc::WHITE * .15f;
	::llc::rgbaf			DiffuseColor		= ::llc::WHITE;
	::llc::u0_t			TextureSeed		= 2;
};

stct SGalaxyHellShip {
	::llc::SEngine			Engine			= {};
	SGalaxyHellShipConstants	Constants		= {};
	::llc::lmpod<::llc::u0_t, ::llc::u2_t>
					GeometryCache		= {};
	::llc::eid_t			ShipCore		= ::llc::EID_INVALID;
	::llc::astatic<::llc::eid_t, 6>
					ShipModules		= {};
	::llc::f3_t			RootAngle		= 0;
	::llc::f3_t			ModuleTime		= 0;
};

::llc::err_t	galaxyHellShipSetup	(SGalaxyHellShip & ship);
::llc::err_t	galaxyHellShipUpdate	(SGalaxyHellShip & ship, double secondsLastFrame);

#endif // LLC_SAMPLE_SHIP_H_23627
