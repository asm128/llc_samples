#include "llc_engine_window.h"
#include "llc_engine_planetary_system.h"

#ifndef LLC_SAMPLE_SOLAR_SYSTEM_H_23627
#define LLC_SAMPLE_SOLAR_SYSTEM_H_23627

stxp ::llc::u0_t SAMPLE_GUI_VIEWPORT_COUNT = 1;

stct SSampleSolarSystem {
	::llc::SEngineWindow			Window			= {};
	::llc::SEngine				Engine			= {};
	::llc::SEnginePlanetarySystem	SolarSystem		= {};
	::llc::img8bgra				Color			= {};
	::llc::img<::llc::u2_t>		Depth			= {};
	::llc::astchar<128>			PerformanceText	= {};
	::llc::f3_t					CameraAngle		= 0;
	::llc::f3_t					CameraDistance	= 24;
	::llc::u2_t					SelectedBody		= 0;
	::llc::u2_t					SelectedPlanet	= 0;
	::llc::f3_t					TimeScale		= 1;
	bool						Paused			= false;
	::llc::f3_t					Time				= 0;
};

::llc::err_t	sampleSolarSystemSetup	(SSampleSolarSystem & app);
::llc::err_t	sampleSolarSystemUpdate	(SSampleSolarSystem & app, bool systemRequestedExit, double secondsLastFrame, double secondsUpdate, double secondsDraw);
::llc::err_t	sampleSolarSystemCamera	(const SSampleSolarSystem & app, ::llc::n3f2_t & cameraPosition, ::llc::n3f2_t & cameraTarget);
::llc::err_t	sampleSolarSystemDraw	(SSampleSolarSystem & app, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::g8bgra target);

#endif // LLC_SAMPLE_SOLAR_SYSTEM_H_23627
