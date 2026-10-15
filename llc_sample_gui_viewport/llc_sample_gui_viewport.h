#include "llc_engine_window.h"
#include "llc_gui_viewport.h"
#include "../llc_sample_engine/llc_sample_engine.h"

#ifndef LLC_SAMPLE_GUI_VIEWPORT_H_23627
#define LLC_SAMPLE_GUI_VIEWPORT_H_23627

stxp ::llc::u0_t SAMPLE_GUI_VIEWPORT_COUNT = 3;

stct SSampleGUIViewport {
	::llc::SEngineWindow		Window			= {};
	SSampleEngine			Ship			= {};
	::llc::SGUI				GUI				= {};
	::llc::cid_t			Desktop			= ::llc::CID_INVALID;
	::llc::cid_t			Help			= ::llc::CID_INVALID;
	::llc::cid_t			Performance		= ::llc::CID_INVALID;
	::llc::cid_t			Exit			= ::llc::CID_INVALID;
	::llc::astchar<96>		PerformanceText		= {};
	::llc::astatic<::llc::SViewport, SAMPLE_GUI_VIEWPORT_COUNT>
						Viewports		= {};
	::llc::astatic<::llc::n2s1_t, SAMPLE_GUI_VIEWPORT_COUNT>
						UnfoldedSize		= {};
	::llc::astatic<bool, SAMPLE_GUI_VIEWPORT_COUNT>
						Folded			= {};
};

::llc::err_t	sampleGUIViewportSetup				(SSampleGUIViewport & app);
::llc::err_t	sampleGUIViewportUpdate				(SSampleGUIViewport & app, bool systemRequestedExit, double secondsLastFrame);
::llc::err_t	sampleGUIViewportDraw				(SSampleGUIViewport & app, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::u2_t frameIndex, ::llc::g8bgra target);

#endif // LLC_SAMPLE_GUI_VIEWPORT_H_23627
