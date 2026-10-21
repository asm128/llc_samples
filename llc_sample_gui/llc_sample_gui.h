#include "llc_engine.h"
#include "llc_window.h"
#include "llc_gui.h"

#ifndef LLC_SAMPLE_GUI_H_23627
#define LLC_SAMPLE_GUI_H_23627

stct SSampleGUI {
	::llc::SEngine		Engine			= {};
	::llc::SWindow		& Window;
	::llc::SGUI			& GUI;

	SSampleGUI(::llc::SWindow & window, ::llc::SGUI & gui) : Window(window), GUI(gui) {}
	::llc::cid_t		ControlTitle			= ::llc::CID_INVALID;
	::llc::cid_t		ControlMode				= ::llc::CID_INVALID;
	::llc::astcid<3>	ControlsTheme			= {};
	::llc::astcid<8>	ControlsState			= {};
	::llc::cid_t		ControlLive				= ::llc::CID_INVALID;
	::llc::cid_t		ControlInspector		= ::llc::CID_INVALID;
	::llc::cid_t		ControlHelp				= ::llc::CID_INVALID;
	::llc::cid_t		ControlExit				= ::llc::CID_INVALID;
	::llc::cid_t		ControlAlignmentTitle	= ::llc::CID_INVALID;
	::llc::cid_t		ControlAlignmentRoot	= ::llc::CID_INVALID;
	::llc::astcid<9>	ControlsAlignment		= {};
	::llc::astcid<6>	ControlsKeyboard		= {};
	::llc::astchar<512>	TextLive				= {};
	::llc::astchar<512>	TextInspector			= {};
	::llc::u0_t			Theme					= 0;
};

::llc::err_t		sampleGUISetup				(SSampleGUI & app);
::llc::err_t		sampleGUIUpdate				(SSampleGUI & app, bool systemRequestedExit);
::llc::err_t		sampleGUIDraw				(SSampleGUI & app, ::llc::g8bgra target);

#endif // LLC_SAMPLE_GUI_H_23627
