#include "llc_engine_window.h"
#include "llc_gui_render.h"
#include "llc_gui_viewport.h"

#ifndef LLC_SAMPLE_GUI_VIEWPORT_H_23627
#define LLC_SAMPLE_GUI_VIEWPORT_H_23627

stxp ::llc::u0_t SAMPLE_GUI_VIEWPORT_COUNT = 3;

stct SSampleGUIViewport {
	::llc::SEngineWindow		Window			= {};
	::llc::SGUI				GUI				= {};
	::llc::cid_t			Desktop			= ::llc::CID_INVALID;
	::llc::cid_t			Help			= ::llc::CID_INVALID;
	::llc::cid_t			Performance		= ::llc::CID_INVALID;
	::llc::astchar<96>		PerformanceText		= {};
	::llc::astatic<::llc::SViewport, SAMPLE_GUI_VIEWPORT_COUNT>
						Viewports		= {};
};

::llc::err_t	sampleGUIViewportSetup				(SSampleGUIViewport & app);
::llc::err_t	sampleGUIViewportRenderResourcesSetup	(cnst SSampleGUIViewport & app, ::llc::SGUIRenderResources & resources, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT]);
::llc::err_t	sampleGUIViewportUpdate				(SSampleGUIViewport & app, bool systemRequestedExit, double secondsLastFrame);
::llc::err_t	sampleGUIViewportRenderEvents		(SSampleGUIViewport & app, ::llc::TGUIRenderEventQueue & output, cnst ::llc::n2u1_t & targetSize, cnst ::llc::FGUITextMeasure & textMeasure);
::llc::err_t	sampleGUIViewportDraw				(cnst ::llc::TGUIRenderEventQueue & events, ::llc::SGUIRenderResources & resources, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::u2_t frameIndex, ::llc::g8bgra target);

#endif // LLC_SAMPLE_GUI_VIEWPORT_H_23627
