#include "llc_engine_window.h"
#include "llc_img_color.h"

#ifndef LLC_SAMPLE_WINDOW_H_23627
#define LLC_SAMPLE_WINDOW_H_23627

stct SSampleWindow {
	::llc::SEngineWindow	Window		= {};
	::llc::img8bgra		Target		= {};
	::llc::u2_t			FrameIndex	= 0;
};

::llc::err_t	sampleWindowSetup	(SSampleWindow& app);
::llc::err_t	sampleWindowUpdate	(SSampleWindow& app, bool systemRequestedExit);
::llc::err_t	sampleWindowDraw		(SSampleWindow& app);

#endif // LLC_SAMPLE_WINDOW_H_23627
