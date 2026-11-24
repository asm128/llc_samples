// Tip: Hold Left ALT + SHIFT while tapping or holding the arrow keys in order to select multiple columns and write on them at once.
//		Also useful for copy & paste operations in which you need to copy a bunch of variable or function names and you can't afford the time of copying them one by one.
#include "ssiege.win32.h"
#include "llc_sun.h"

#include "llc_grid_copy.h"
#include "llc_grid_scale.h"
#include "llc_view_bit.h"
#include "llc_matrix.h"
#include "llc_png.h"

#include "llc_app_impl.h"
#include "llc_bitmap_target.h"

#include <DirectXColors.h>

using ::llc::get_value_namep, ::llc::get_enum_namep, ::llc::failed;
LLC_USING_TYPEINT();

LLC_DEFINE_APPLICATION_ENTRY_POINT(::SApplication, "Solar Siege v0.1");

// --- Cleanup application resources.
llc::err_t			cleanup					(::SApplication & app)											{
	::llc::SFramework			& framework				= app.Framework;
	::llc::SWindow				& mainWindow			= framework.RootWindow;
#if !defined(DISABLE_D3D11)
	app.Renderer.Shutdown();
#endif

	if_fail_fe(::llc::mainWindowDestroy(mainWindow));
	if_fail_fe(::ssg::ssiegeUpdate(app.SSiegeApp, 0, mainWindow.Input, mainWindow.EventQueue, {}));
	return 0;
}

static	llc::err_t	updateSizeDependentResources(::SApplication & app)											{
	::llc::SWindow				& mainWindow			= app.Framework.RootWindow;
	const ::llc::n2u1_t			newSize					= mainWindow.Size;
#if !defined(DISABLE_D3D11)
	mainWindow.BackBuffer	= {};
	if_fail_fe(app.Renderer.SetWindowSize(newSize));
#else
	if_fail_fe(mainWindow.BackBuffer->resize(newSize, ::llc::bgra{0, 0, 0, 0}, 0xFFFFFFFF));
#endif
	mainWindow.Resized		= false;
	return 0;
}

static	llc::err_t	processScreenEvent		(::SApplication & app, const ::llc::SEventView<::llc::EVENT_SCREEN> & screenEvent) { 
	switch(screenEvent.Type) {
	default: break;
	case ::llc::EVENT_SCREEN_Create:
#if !defined(DISABLE_D3D11)
		if_fail_fe(app.Renderer.Initialize(app.Framework.RootWindow, app.SSiegeApp.Game.Engine.Scene->Graphics));
#endif
	case ::llc::EVENT_SCREEN_Resize: 
		if_fail_fe(::updateSizeDependentResources(app));
		break;
	}
	return 0;
}

static	llc::err_t	processGUIEvent			(::SApplication & /*app*/, const ::llc::SEventView<::llc::EVENT_GUI_CONTROL> & screenEvent) { 
	switch(screenEvent.Type) {
	default: break;
	case ::llc::EVENT_GUI_CONTROL_StateChange: {
		const ::llc::SChangeControlState	stateChange	= *(const ::llc::SChangeControlState*)screenEvent.Data.begin();
		if(0 == (stateChange.Set & ::llc::GUI_CONTROL_FLAG_Action))
			return 0;
	}
	}
	return 0;
}

static	llc::err_t	processSystemEvent		(::SApplication & app, const ::llc::SEventSystem & sysEvent) { 
	switch(sysEvent.Type) {
	default: break;
	case ::llc::SYSTEM_EVENT_Screen	: return ::llc::eventExtractAndHandle<::llc::EVENT_SCREEN		>(sysEvent, [&app](const ::llc::SEventView<::llc::EVENT_SCREEN		> & screenEvent) { return processScreenEvent(app, screenEvent); }); 
	case ::llc::SYSTEM_EVENT_GUI	: return ::llc::eventExtractAndHandle<::llc::EVENT_GUI_CONTROL	>(sysEvent, [&app](const ::llc::SEventView<::llc::EVENT_GUI_CONTROL	> & screenEvent) { return processGUIEvent	(app, screenEvent); }); 
	}
	return 0;
}

llc::err_t			setup					(::SApplication& app)											{
#if !defined(DISABLE_D3D11)
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif
	::llc::SFramework			& framework				= app.Framework;
	::llc::SWindow				& mainWindow			= framework.RootWindow;
	mainWindow.Size			= {1280, 720};
	if_fail_fe(::llc::mainWindowCreate(mainWindow, framework.RuntimeValues.PlatformDetail, mainWindow.Input));

	return 0;
}

llc::err_t			update					(::SApplication& app, bool systemRequestedExit)					{
	::llc::SFramework			& framework				= app.Framework;
	::llc::SWindow				& mainWindow			= app.Framework.RootWindow;
	::llc::SGUI					& gui					= *framework.GUI;

	bool						systemExit				= false;

	::llc::apobj<::llc::SEventSystem>	eventsToProcess	= mainWindow.EventQueue;
	if_fail_fe(eventsToProcess.append(gui.Controls.EventQueue));
	if_fail_fe(eventsToProcess.for_each([&app, &systemExit](const ::llc::pobj<::llc::SEventSystem> & sysEvent) { 
		llc::err_t				result; 
		if_fail_fe(result = ::processSystemEvent(app, *sysEvent)); 
		if(result == 1) 
			systemExit				= true; 
		return result;
	}));

	if_true_vif(::llc::APPLICATION_STATE_EXIT, systemExit || systemRequestedExit, "%u || %u", systemExit, systemRequestedExit);

	::llc::SFrameInfo			& frameInfo				= framework.FrameInfo;
	{
		::llc::STimer				timer					= {};
		if_true_vi(::llc::APPLICATION_STATE_EXIT, ::ssg::APP_STATE_Quit == ::ssg::ssiegeUpdate(app.SSiegeApp, frameInfo.Seconds.LastFrame, mainWindow.Input, mainWindow.EventQueue, {}));
		timer.Frame();
		//info_printf("Update engine in %f seconds", timer.LastTimeSeconds);
	}

#if !defined(DISABLE_D3D11)
	if(app.SSiegeApp.ActiveState >= ::ssg::APP_STATE_Welcome && app.Renderer.Scene.IndexBuffer.size() < app.SSiegeApp.Game.Engine.Scene->Graphics->Meshes.size() || !app.Renderer.GUIStuff.IndexBuffer) {
		if_fail_fe(app.Renderer.CreateDeviceResources(*app.SSiegeApp.Game.Engine.Scene->Graphics));
	}
	app.Renderer.Text.Update(frameInfo.Seconds.LastFrame, frameInfo.Seconds.Total, (u2_t)frameInfo.FrameMeter.FramesPerSecond);
#endif

	//-----------------------------
	::llc::STimer				& timer					= app.Framework.Timer;
	sc_t						buffer	[256]			= {};
	sprintf_s(buffer, "[%u x %u]. FPS: %g. Last frame seconds: %g.", mainWindow.Size.x, mainWindow.Size.y, 1 / timer.LastTimeSeconds, timer.LastTimeSeconds);
	::HWND						windowHandle			= mainWindow.PlatformDetail.WindowHandle;
	SetWindowTextA(windowHandle, buffer);
	
	return ::llc::updateFramework(framework);
}

llc::err_t			draw					(::SApplication& app)											{	// --- This function will draw some coloured symbols in each cell of the ASCII screen.
	const ::llc::n3f2_t			sunlightPos				= {}; //::llc::calcSunPosition();
	//const double				sunlightFactor			= .1;//:::llc::calcSunlightFactor(app.SSiegeApp.World.WorldState.DaylightRatioExtra, app.SSiegeApp.World.WorldState.DaylightOffsetMinutes);
	const ::llc::rgbaf			clearColor				= ::llc::DARKBLUE * .0625;//::llc::interpolate_linear(::llc::DARKBLUE * .25, ::llc::LIGHTBLUE * 1.1, sunlightFactor);

#if !defined(DISABLE_D3D11) 
	memset(app.Renderer.GUIStuff.RenderTarget.begin(), 0, app.Renderer.GUIStuff.RenderTarget.byte_count());
	
	if_fail_fe(::llc::guiDraw(*app.SSiegeApp.GUI, app.Renderer.GUIStuff.RenderTarget));

	app.Renderer.Scene.ConstantBufferScene.Time	= (float)app.Framework.FrameInfo.Seconds.Total;

	const ::llc::SCamera		& cameraSelected		= app.SSiegeApp.Game.Camera;
	const ::llc::SEngineScene	& engineScene			= *app.SSiegeApp.Game.Engine.Scene;
	if_fail_fe(::llc::d3dAppDraw(app.Renderer, engineScene, clearColor, sunlightPos, cameraSelected.Position, cameraSelected.Target, cameraSelected.NearFar));
#else 
	::llc::SFramework			& framework				= app.Framework;
	::llc::prtbgra8d32			backBuffer				= framework.RootWindow.BackBuffer;
	backBuffer->resize(framework.RootWindow.BackBuffer->Color.metrics(), clearColor, (u2_t)-1);
	if_fail_fe(::ssg::ssiegeDraw(app.SSiegeApp, *backBuffer, false));
	memcpy(framework.RootWindow.BackBuffer->Color.View.begin(), backBuffer->Color.View.begin(), backBuffer->Color.View.byte_count());
	//::llc::grid_mirror_y(framework.RootWindow.BackBuffer->Color.View, backBuffer->Color.View);
	//framework.RootWindow.BackBuffer		= backBuffer;
#endif
	return 0;
}
