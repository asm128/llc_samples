#include "llc_engine_window.h"
#include "llc_gui_checkbox.h"
#include "llc_gui_inputbox.h"
#include "llc_gui_render.h"
#include "llc_gui_slider.h"
#include "llc_gui_viewport.h"

#ifndef LLC_SAMPLE_GUI_COMPONENTS_H_23627
#define LLC_SAMPLE_GUI_COMPONENTS_H_23627

stxp ::llc::u0_t SAMPLE_GUI_COMPONENTS_COUNT = 3;

stct SSampleGUIComponents {
	::llc::SEngineWindow		Window			= {};
	::llc::SGUI				GUI				= {};
	::llc::cid_t			Desktop			= ::llc::CID_INVALID;
	::llc::cid_t			Help			= ::llc::CID_INVALID;
	::llc::cid_t			Performance		= ::llc::CID_INVALID;
	::llc::astchar<96>		PerformanceText	= {};
	::llc::cid_t			ComponentsPanel	= ::llc::CID_INVALID;
	::llc::cid_t			ControlEdit		= ::llc::CID_INVALID;
	::llc::cid_t			ControlExit		= ::llc::CID_INVALID;
	::llc::SCheckBox		CheckBox			= {};
	::llc::SSlider			SliderHorizontal	= {};
	::llc::SSlider			SliderVertical		= {};
	::llc::SInputBox		InputBox			= {};
	::llc::acid			ControlsKeyboard	= {};
	::llc::astatic<::llc::SViewport, SAMPLE_GUI_COMPONENTS_COUNT>
						Viewports		= {};
};

::llc::err_t	sampleGUIComponentsSetup				(SSampleGUIComponents & app);
::llc::err_t	sampleGUIComponentsRenderResourcesSetup	(cnst SSampleGUIComponents & app, ::llc::SGUIRenderResources & resources, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_COMPONENTS_COUNT]);
::llc::err_t	sampleGUIComponentsUpdate				(SSampleGUIComponents & app, bool systemRequestedExit, double secondsLastFrame);
::llc::err_t	sampleGUIComponentsRenderEvents			(SSampleGUIComponents & app, ::llc::TGUIRenderEventQueue & output, cnst ::llc::n2u1_t & targetSize, cnst ::llc::FGUITextMeasure & textMeasure);
::llc::err_t	sampleGUIComponentsDraw					(cnst ::llc::TGUIRenderEventQueue & events, ::llc::SGUIRenderResources & resources, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_COMPONENTS_COUNT], ::llc::u2_t frameIndex, ::llc::g8bgra target);

#endif // LLC_SAMPLE_GUI_COMPONENTS_H_23627
