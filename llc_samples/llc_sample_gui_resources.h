#include "llc_gui_render.h"

#ifndef LLC_SAMPLE_GUI_RESOURCES_H_23627
#define LLC_SAMPLE_GUI_RESOURCES_H_23627

stxp ::llc::u0_t SAMPLE_GUI_THEME_COUNT = 3;

::llc::vcst_t		sampleGUIThemeText		(::llc::u0_t theme);
::llc::vcst_t		sampleGUIThemeName		(::llc::u0_t theme);
::llc::SColorBGRA	sampleGUIThemeColor		(::llc::u0_t theme) nxpt;
::llc::SColorBGRA	sampleGUIShade			(::llc::SColorBGRA color, ::llc::u0_t shade) nxpt;
::llc::err_t		sampleGUIResourcesCreate	(::llc::SGUIRenderResources & resources);
void				sampleGUIControlThemeApply	(::llc::SGUI & gui, ::llc::cid_t idControl, ::llc::u0_t theme) nxpt;
void				sampleGUIThemeApply		(::llc::SGUI & gui, ::llc::u0_t theme) nxpt;

#endif // LLC_SAMPLE_GUI_RESOURCES_H_23627
