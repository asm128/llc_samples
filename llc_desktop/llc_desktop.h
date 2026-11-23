#include "llc_engine.h"
#include "llc_window.h"
#include "llc_gui_composite.h"
#include "llc_path.h"

#ifndef LLC_DESKTOP_H_23627
#define LLC_DESKTOP_H_23627

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
	::llc::aobj<::llc::vcst_t> FileLines		= {};
	::llc::aobj<::llc::SGUIViewportTextRow> TextRows = {};
	::llc::acid			FileControls		= {};
	::llc::u2_t			FileOffset			= 0;
	::llc::u2_t			FileColumnOffset	= 0;
	::llc::u2_t			FileLineWidthMax	= 0;
	::llc::u2_t			VisibleColumns		= 0;
	bool					WordWrap		= false;
	bool					SelectAll		= false;
	::llc::astatic<::llc::SViewport, SAMPLE_GUI_VIEWPORT_COUNT>
						Viewports		= {};
	::llc::astatic<::llc::astcid<::llc::GUI_VIEWPORT_ACTION_COUNT>, SAMPLE_GUI_VIEWPORT_COUNT>
						ViewportActions = {};
	::llc::astatic<::llc::n2s1_t, SAMPLE_GUI_VIEWPORT_COUNT>
						UnfoldedSize		= {};
	::llc::astatic<bool, SAMPLE_GUI_VIEWPORT_COUNT>
						Folded			= {};
};

::llc::err_t	sampleGUIViewportSetup				(SSampleGUIViewport & app);
::llc::err_t	sampleGUIViewportUpdate				(SSampleGUIViewport & app, bool systemRequestedExit, double secondsLastFrame);
::llc::err_t	sampleGUIViewportDraw				(SSampleGUIViewport & app, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::u2_t frameIndex, ::llc::g8bgra target);

#endif // LLC_DESKTOP_H_23627
