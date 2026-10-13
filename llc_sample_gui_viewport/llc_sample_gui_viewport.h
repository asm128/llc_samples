#include "llc_engine_window.h"
#include "llc_gui_render.h"
#include "llc_gui_viewport.h"

#ifndef LLC_SAMPLE_GUI_VIEWPORT_H_23627
#define LLC_SAMPLE_GUI_VIEWPORT_H_23627

stxp ::llc::u0_t SAMPLE_GUI_VIEWPORT_COUNT = 3;

stct SSampleGUIViewport {
	::llc::SEngineWindow		Window			= {};
	::llc::img8bgra			Target			= {};
	::llc::SGUI				GUI				= {};
	::llc::SGUIRenderResources	Resources		= {};
	::llc::cid_t			Desktop			= ::llc::CID_INVALID;
	::llc::cid_t			Help			= ::llc::CID_INVALID;
	::llc::astatic<::llc::SViewport, SAMPLE_GUI_VIEWPORT_COUNT>
						Viewports		= {};
	::llc::img8bgra			ClientImages[SAMPLE_GUI_VIEWPORT_COUNT] = {};
	::llc::u2_t			FrameIndex		= 0;
};

::llc::err_t	sampleGUIViewportSetup	(SSampleGUIViewport & app);
::llc::err_t	sampleGUIViewportUpdate	(SSampleGUIViewport & app, bool systemRequestedExit);
::llc::err_t	sampleGUIViewportDraw	(SSampleGUIViewport & app);

#endif // LLC_SAMPLE_GUI_VIEWPORT_H_23627
