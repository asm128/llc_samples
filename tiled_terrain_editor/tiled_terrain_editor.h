#include "llc_framework.h"
#include "llc_gui.h"
#include "llc_dialog.h"
#include "llc_scene.h"
#include "llc_ro_rsw.h"
#include "llc_ro_rsm.h"
#include "llc_ro_gnd.h"

#include <mutex>

#ifndef APPLICATION_H_23627
#define APPLICATION_H_23627


namespace gme // I'm gonna use a different namespace in order to test a few things about the macros.
{

	struct SApplication {
		::llc::SFramework				Framework						;
		::llc::pobj<::llc::rtbgra8d32>	Offscreen						;

		::llc::cid_t					IdExit							= -1;

		::std::mutex					LockRender						;

		::llc::SDialog					DialogMain						;
		::llc::cid_t					SliderH							;
		::llc::cid_t					SliderV							;
		::llc::cid_t					NumericTuner					;
		::llc::cid_t					CheckBox						;
		::llc::cid_t					Viewport						;
		::llc::cid_t					ViewportMinimap					;
		::llc::cid_t					IdFrameRateUpdate				= -1;
		::llc::cid_t					IdFrameRateRender				= -1;
		char							StringFrameRateUpdate	[256]	= {};
		char							StringFrameRateRender	[256]	= {};

		::llc::img8bgra							TextureMinimap					= {};
		::llc::aobj<::llc::img8bgra>			TexturesGND						= {};
		::llc::SModelPivot<float>				GridPivot						= {};
		::llc::SScene							Scene;
		::llc::n3f2_t							LightDirection					= {10, 5, 0};
		::llc::SGNDFileContents					GNDData							= {};
		::llc::SRSWFileContents					RSWData							= {};
		::llc::aobj<::llc::SRSMFileContents>	RSMData							= {};
		::llc::SModelGND						GNDModel						= {};
		::llc::SRenderCache						RenderCache						= {};

												SApplication					(::llc::SRuntimeValues& runtimeValues)	: Framework(runtimeValues)		{}
	};
} // namespace


#endif // APPLICATION_H_23627
