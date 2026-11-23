#include "llc_engine.h"
#include "llc_window.h"
#include "llc_gui_viewport.h"
#include "llc_path.h"
#include "llc_ide_common.h"
#include "llc_gui_inputbox.h"
#include "llc_json.h"

#ifndef LLC_IDE_H_23627
#define LLC_IDE_H_23627

stxp ::llc::u0_t SAMPLE_GUI_VIEWPORT_COUNT = 3;

stct SSampleGUIViewport {
	::llc::SEngine		Engine			= {};
	::llc::SWindow		& Window;
	::llc::SGUI			& GUI;

	SSampleGUIViewport(::llc::SWindow & window, ::llc::SGUI & gui) : Window(window), GUI(gui) {}
	::llc::cid_t			Desktop			= ::llc::CID_INVALID;
	::llc::cid_t			Help			= ::llc::CID_INVALID;
	::llc::cid_t			Performance		= ::llc::CID_INVALID;
	::llc::cid_t			Exit			= ::llc::CID_INVALID;
	::llc::astchar<96>		PerformanceText		= {};
	::llc::string			FolderPath			= {};
	::llc::SPathTree		FolderEntries		= {};
	::llc::acid			FolderControls		= {};
	::llc::u2_t			FolderOffset		= 0;
	::llc::string			OpenFile			= {};
	::llc::string			FileBytes			= {};
	::llc::string			FileDisplay		= {};
	SFileEditor				Editor			= {};
	::llc::SVirtualKeyboard	Keyboard		= {};
	::llc::SJSONFile		JSONDocument	= {};
	::llc::err_t			JSONParseResult = 0;
	::llc::aobj<::llc::vcst_t> FileLines		= {};
	::llc::acid			FileControls		= {};
	::llc::u2_t			FileOffset			= 0;
	::llc::astatic<::llc::SViewport, SAMPLE_GUI_VIEWPORT_COUNT>
						Viewports		= {};
};

::llc::err_t	sampleGUIViewportSetup				(SSampleGUIViewport & app);
::llc::err_t	sampleGUIViewportUpdate				(SSampleGUIViewport & app, bool systemRequestedExit, double secondsLastFrame);
::llc::err_t	sampleGUIViewportDraw				(SSampleGUIViewport & app, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::u2_t frameIndex, ::llc::g8bgra target);

#endif // LLC_IDE_H_23627
