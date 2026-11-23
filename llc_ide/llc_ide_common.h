#include "llc_gui.h"
#include "llc_path.h"
#include "llc_range.h"
#include "llc_virtual_keyboard.h"

#ifndef LLC_IDE_COMMON_H_23627
#define LLC_IDE_COMMON_H_23627

usns llc;

stct SFileScroll {
	cid_t				IdRoot				= -1;
	cid_t				IdDecrease			= -1;
	cid_t				IdIncrease			= -1;
	cid_t				IdThumb				= -1;
	minmax<s3_t>		Limits				= {};
	s3_t				Value				= 0;
};

stct SFileViewport {
	cid_t				IdRoot				= -1;
	cid_t				IdTitle			= -1;
	cid_t				IdClient			= -1;
	cid_t				IdClose			= -1;
	cid_t				IdMinimize			= -1;
	cid_t				IdResize			= -1;
	SFileScroll			ScrollX				= {};
	SFileScroll			ScrollY				= {};
};

stct SFileControlList {
	cid_t				IdRoot				= -1;
	acid				IdControls			= {};
};

stct SPathList {
	SFileViewport		Viewport			= {};
	SFileControlList	Files				= {};
	SPathTree			PathContents		= {};
};

stct SFileEditor {
	SFileViewport		Viewport			= {};
	SFileControlList	Lines				= {};
	string				FilePath			= {};
	string				FileText			= {};
	apod<vcst_t>		FileLines			= {};
	apod<rangeu2_t>	TextSelections		= {};
	u2_t				SelectionAnchor		= (u2_t)-1;
	u2_t				SelectionActive		= (u2_t)-1;
	n2u2_t				Cursor				= {};
	cid_t				IdCursor			= -1;
	cid_t				IdKeyboard			= -1;
	cid_t				IdTab				= -1;
	u2_t				FileLineWidthMax	= 0;
};

llc::err_t fileViewerTextViewsRebuild(SFileEditor & editor);
llc::u2_t fileViewerLineOffset(cnst SFileEditor & editor, llc::u2_t line);
llc::err_t fileViewerTextInsert(llc::SGUI & gui, SFileEditor & editor, llc::sc_t character);
llc::err_t fileViewerTextBackspace(llc::SGUI & gui, SFileEditor & editor);
llc::err_t fileViewerTextDelete(llc::SGUI & gui, SFileEditor & editor);
llc::err_t fileViewerEditorHandle(llc::SGUI & gui, llc::SVirtualKeyboard & keyboard, SFileEditor & editor, llc::SYSTEM_KEY scanCode);
llc::err_t fileViewerEditorInput(llc::SGUI & gui, llc::SVirtualKeyboard & keyboard, SFileEditor & editor, llc::TQueueSystemEvent frameEvents);

#endif // LLC_IDE_COMMON_H_23627
