#include "llc_engine.h"
#include "llc_window.h"
#include "llc_gui_viewport.h"

#ifndef LLC_SAMPLE_GEOMETRY_H_23627
#define LLC_SAMPLE_GEOMETRY_H_23627

stxp ::llc::u0_t SAMPLE_GEOMETRY_COUNT = 8;

stct SSampleGeometry {
	::llc::SEngine			Engine;
	::llc::SWindow			& Window;
	::llc::SGUI				& GUI;
	::llc::SCameraPoints		Camera			= {{0, 2.5f, -4.5f}, {0, 0, 0}};
	::llc::minmaxf2_t		NearFar			= {.1f, 100.f};
	::llc::n3f2_t			LightOffset		= {-3.f, 4.f, -4.f};
	::llc::astatic<::llc::eid_t, SAMPLE_GEOMETRY_COUNT>	Entities	= {};
	::llc::astatic<::llc::SViewport, SAMPLE_GEOMETRY_COUNT> Viewports	= {};
	::llc::SWindow::TOffscreen Targets[SAMPLE_GEOMETRY_COUNT] = {};
	::llc::cid_t			Desktop			= ::llc::CID_INVALID;
	::llc::cid_t			Scale			= ::llc::CID_INVALID;
	::llc::cid_t			ScaleThumb		= ::llc::CID_INVALID;
	::llc::cid_t			Performance		= ::llc::CID_INVALID;
	::llc::astchar<32>		ScaleText		= {};
	::llc::astchar<96>		PerformanceText	= {};
	::llc::s1_t				ClientTheme		= 0;
	::llc::s1_t				Maximized		= -1;
	::llc::rect2s1_t			RestoreArea		= {};
	::llc::cid_t			DragControl		= ::llc::CID_INVALID;
	::llc::n2s1_t			DragOrigin		= {};
	::llc::rect2s1_t			DragArea		= {};
	::llc::n2f3_t			DragScale		= {1, 1};
	double					DragZoom		= 1;

	SSampleGeometry(::llc::SWindow & window, ::llc::SGUI & gui) : Window(window), GUI(gui) {}
};

::llc::err_t	sampleGeometrySetup	(SSampleGeometry & app);
::llc::err_t	sampleGeometryDraw	(SSampleGeometry & app, ::llc::SWindow::TOffscreen & target);

#endif // LLC_SAMPLE_GEOMETRY_H_23627