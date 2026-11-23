#include "ssiege.h"

#include "llc_framework.h"

//#define DISABLE_D3D11
//#define DISABLE_XAUDIO2

#ifndef DISABLE_D3D11
#	include "llc_engine_d3d_app.h"
#endif

#ifndef DISABLE_XAUDIO2
#	include "llc_xaudio2.h"
#endif

#ifndef SSIEGE_WIN32_H_23701
#define SSIEGE_WIN32_H_23701

struct SApplication {
	::llc::SFramework		Framework	;
#ifndef DISABLE_D3D11
	::llc::SD3DApplication	Renderer		= {};
#endif
#ifndef DISABLE_XAUDIO2
	::llc::AUDIO_STATE		AudioState		= {};
#endif

	::ssg::SSiegeApp	SSiegeApp			= {};

							SApplication	(::llc::SRuntimeValues & runtimeValues)	noexcept	
	: Framework(runtimeValues) { 
		if(!Framework.GUI) 
			Framework.GUI.create(); 
		SSiegeApp.GUI				= Framework.GUI; 
		if_fail_e(::llc::rasterFontDefaults(SSiegeApp.Game.Engine.Scene->Graphics->Fonts));
	}
};

#endif // SSIEGE_WIN32_H_23701
