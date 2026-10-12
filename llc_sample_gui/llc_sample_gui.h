#include "llc_engine_window.h"
#include "llc_gui_render.h"

#ifndef LLC_SAMPLE_GUI_H_23627
#define LLC_SAMPLE_GUI_H_23627

stct SSampleGUI {
	::llc::SEngineWindow		Window			= {};
	::llc::img8bgra			Target			= {};
	::llc::SGUI				GUI				= {};
	::llc::SGUIRenderResources	Resources		= {};
	::llc::cid_t			ControlTitle	= ::llc::CID_INVALID;
	::llc::cid_t			ControlMode		= ::llc::CID_INVALID;
	::llc::astcid<3>		ControlsTheme	= {};
	::llc::astcid<8>		ControlsState	= {};
	::llc::cid_t			ControlLive		= ::llc::CID_INVALID;
	::llc::cid_t			ControlInspector= ::llc::CID_INVALID;
	::llc::cid_t			ControlHelp		= ::llc::CID_INVALID;
	::llc::astchar<512>	TextLive		= {};
	::llc::astchar<512>	TextInspector	= {};
	::llc::u0_t			Theme			= 0;
};

::llc::err_t	sampleGUISetup	(SSampleGUI& app);
::llc::err_t	sampleGUIUpdate	(SSampleGUI& app, bool systemRequestedExit);
::llc::err_t	sampleGUIDraw	(SSampleGUI& app);

#endif // LLC_SAMPLE_GUI_H_23627
