#include "llc_engine_window.h"
#include "llc_engine.h"
#include "llc_engine_scene.h"

#ifndef LLC_SAMPLE_ENGINE_H_23627
#define LLC_SAMPLE_ENGINE_H_23627

stxp ::llc::u0_t SAMPLE_GUI_VIEWPORT_COUNT = 1;

enum SHIP_SAMPLE_GEOMETRY : ::llc::u0_t
	{ SHIP_SAMPLE_GEOMETRY_Gun
	, SHIP_SAMPLE_GEOMETRY_Wafer
	, SHIP_SAMPLE_GEOMETRY_Cannon
	, SHIP_SAMPLE_GEOMETRY_Shotgun
	, SHIP_SAMPLE_GEOMETRY_WaferShotgun
	, SHIP_SAMPLE_GEOMETRY_Engine
	, SHIP_SAMPLE_GEOMETRY_COUNT
	};

stct SSampleEngine {
	::llc::SEngineWindow		Window			= {};
	::llc::SEngine			Engine			= {};
	::llc::img8bgra			Color			= {};
	::llc::img<::llc::u2_t>	Depth			= {};
	::llc::lmpod<::llc::u0_t, ::llc::u2_t>	GeometryCache	= {};
	::llc::eid_t			ShipRoot		= ::llc::EID_INVALID;
	::llc::astatic<::llc::eid_t, 6>	ShipParts		= {};
	::llc::f3_t			Angle			= 0;
};

::llc::err_t	sampleEngineSetup	(SSampleEngine & app);
::llc::err_t	sampleEngineUpdate	(SSampleEngine & app, bool systemRequestedExit, double secondsLastFrame, double secondsUpdate, double secondsDraw);
::llc::err_t	sampleEngineDraw	(SSampleEngine & app, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::g8bgra target);

#endif // LLC_SAMPLE_ENGINE_H_23627
