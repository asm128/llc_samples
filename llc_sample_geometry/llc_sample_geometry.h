#include "llc_window.h"
#include "llc_gui_viewport.h"
#include "llc_geometry.h"

#ifndef LLC_SAMPLE_GEOMETRY_H_23627
#define LLC_SAMPLE_GEOMETRY_H_23627

stxp ::llc::u0_t SAMPLE_GUI_VIEWPORT_COUNT = 16;

stct SSampleGUIScale {
	::llc::cid_t			IdRoot			= ::llc::CID_INVALID;
	::llc::cid_t			IdThumb			= ::llc::CID_INVALID;
	double				Value			= 1.0;
	::llc::astchar<32>		Text			= {};
};

stct SSampleGeometry {
	::llc::SWindow		& Window;
	::llc::SGUI			& GUI;
	::llc::cid_t			Desktop			= ::llc::CID_INVALID;
	::llc::cid_t			Help			= ::llc::CID_INVALID;
	::llc::cid_t			Performance		= ::llc::CID_INVALID;
	::llc::cid_t			Exit			= ::llc::CID_INVALID;
	SSampleGUIScale			GUIScale		= {};
	::llc::astchar<96>		PerformanceText		= {};
	::llc::astatic<::llc::SViewport, SAMPLE_GUI_VIEWPORT_COUNT>
						Viewports		= {};
	::llc::astatic<::llc::n2s1_t, SAMPLE_GUI_VIEWPORT_COUNT>
						UnfoldedSize		= {};
	::llc::astatic<bool, SAMPLE_GUI_VIEWPORT_COUNT>
						Folded			= {};
	::llc::astatic<::llc::rect2s1_t, SAMPLE_GUI_VIEWPORT_COUNT>
						RestoreArea		= {};
	::llc::s1_t		MaximizedViewport = -1;
	::llc::astatic<::llc::SGeometryBuffers, SAMPLE_GUI_VIEWPORT_COUNT>
						Geometry		= {};
	::llc::f2_t		Angle			= {};

	SSampleGeometry(::llc::SWindow & window, ::llc::SGUI & gui) : Window(window), GUI(gui) {}
};

::llc::err_t	sampleGeometrySetup				(SSampleGeometry & app);
::llc::err_t	sampleGeometryUpdate				(SSampleGeometry & app, bool systemRequestedExit, double secondsLastFrame, double secondsUpdate, double secondsDraw);
::llc::err_t	sampleGeometryDraw				(SSampleGeometry & app, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::g8bgra target);

#endif // LLC_SAMPLE_GEOMETRY_H_23627
