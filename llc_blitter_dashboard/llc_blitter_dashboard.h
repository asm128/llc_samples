#include "llc_engine.h"
#include "llc_window.h"
#include "llc_gui_composite.h"
#include "llc_path.h"
#include "llc_json.h"
#include "llc_gui_control_list.h"

#ifndef LLC_BLITTER_DASHBOARD_H_23627
#define LLC_BLITTER_DASHBOARD_H_23627

stxp ::llc::u0_t SAMPLE_GUI_VIEWPORT_COUNT = 4;

stct SBlitterScroll {
	::llc::cid_t			IdTrack		= ::llc::CID_INVALID;
	::llc::cid_t			IdThumb		= ::llc::CID_INVALID;
	bool					Vertical	= false;
};

stct SBlitterDashboard {
	enum JSON_ENTRY_CHILD : ::llc::u0_t { JSON_ENTRY_CHILD_Row, JSON_ENTRY_CHILD_Count };
	stct SJSONEntry {
		::llc::string Label = {};
		::llc::cid_t Root = ::llc::CID_INVALID;
		::llc::u2_t Node = 0;
		::llc::s2_t Parent = -1;
		bool Expanded = false;
		bool ChildrenBuilt = false;
		bool Active = true;
	};
	::llc::SEngine		Engine			= {};
	::llc::SWindow		& Window;
	::llc::SGUI			& GUI;

	SBlitterDashboard(::llc::SWindow & window, ::llc::SGUI & gui) : Window(window), GUI(gui) {}
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
	::llc::s3_t			FileLength			= 0;
	::llc::s3_t			FileWindowStart		= 0;
	::llc::SJSONReader		JSONReader			= {};
	::llc::apobj<SJSONEntry>	JSONEntries			= {};
	::llc::cid_t			JSONPressed			= ::llc::CID_INVALID;
	::llc::string			FileDisplay		= {};
	::llc::aobj<::llc::vcst_t> FileLines		= {};
	::llc::aobj<::llc::SGUIViewportTextRow> TextRows = {};
	::llc::acid			FileControls		= {};
	::llc::u2_t			FileOffset			= 0;
	::llc::u2_t			FileColumnOffset	= 0;
	::llc::u2_t			FileLineWidthMax	= 0;
	::llc::u2_t			VisibleColumns		= 0;
	SBlitterScroll			FolderScrollY	= {};
	SBlitterScroll			FileScrollY		= {};
	SBlitterScroll			FileScrollX		= {};
	bool					WordWrap		= false;
	bool					SelectAll		= false;
	::llc::s3_t			SelectionAnchor	= -1;
	::llc::s3_t			SelectionCursor	= -1;
	::llc::astatic<::llc::SViewport, SAMPLE_GUI_VIEWPORT_COUNT>
						Viewports		= {};
	::llc::astatic<::llc::astcid<::llc::GUI_VIEWPORT_ACTION_COUNT>, SAMPLE_GUI_VIEWPORT_COUNT>
						ViewportActions = {};
	::llc::astatic<::llc::n2s1_t, SAMPLE_GUI_VIEWPORT_COUNT>
						UnfoldedSize		= {};
	::llc::astatic<bool, SAMPLE_GUI_VIEWPORT_COUNT>
						Folded			= {};
};

::llc::err_t	blitterDashboardSetup				(SBlitterDashboard & app);
::llc::err_t	blitterDashboardUpdate				(SBlitterDashboard & app, bool systemRequestedExit, double secondsLastFrame);
::llc::err_t	blitterDashboardDraw				(SBlitterDashboard & app, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::u2_t frameIndex, ::llc::g8bgra target);

#endif // LLC_BLITTER_DASHBOARD_H_23627
