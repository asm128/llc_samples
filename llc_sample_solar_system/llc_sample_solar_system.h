#include "llc_game.h"
#include "llc_engine_planetary_system.h"
#include "llc_gui.h"
#include "llc_window.h"

#ifndef LLC_SAMPLE_SOLAR_SYSTEM_H_23627
#define LLC_SAMPLE_SOLAR_SYSTEM_H_23627

stct SSampleSolarSystem {
	::llc::SWindow					& Window;
	::llc::SGUI						& GUI;
	::llc::SEngine					Engine			= {};
	::llc::SEnginePlanetarySystem	SolarSystem		= {};
	::llc::SCamera					Camera			= {};
	::llc::f3_t						CameraAngle		= 0;
	::llc::f3_t						CameraDistance	= 24;
	::llc::u2_t						SelectedBody	= 0;
	::llc::u2_t						SelectedPlanet	= 0;
	::llc::f3_t						TimeScale		= 1;
	::llc::f3_t						Time			= 0;
	bool							CameraOrbit		= true;
	bool							Paused			= false;
	::llc::cid_t				Performance		= ::llc::CID_INVALID;
	::llc::astchar<96>			PerformanceText	= {};

	SSampleSolarSystem(::llc::SWindow & window, ::llc::SGUI & gui) : Window(window), GUI(gui) {}
};

::llc::err_t	sampleSolarSystemSetup	(SSampleSolarSystem & app);
::llc::err_t	sampleSolarSystemUpdate	(SSampleSolarSystem & app, bool systemRequestedExit, ::llc::f3_t secondsLastFrame);
::llc::err_t	sampleSolarSystemCamera	(SSampleSolarSystem & app);
::llc::err_t	sampleSolarSystemDraw	(SSampleSolarSystem & app, ::llc::SWindow::TOffscreen & backBuffer);

#endif // LLC_SAMPLE_SOLAR_SYSTEM_H_23627
