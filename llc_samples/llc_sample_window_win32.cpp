#include "llc_app_impl.h"
#include "llc_frameinfo.h"
#include "llc_grid_color.h"
#include "llc_mutex.h"
#include "llc_string.h"
#include "llc_sync.h"

#if defined(LLC_SAMPLE_GUI_VIEWPORT)
#	include "llc_sample_gui_viewport.h"
	typedef SSampleGUIViewport SSampleContent;
#	define LLC_SAMPLE_TITLE "LLC GUI viewport"
#	define LLC_SAMPLE_WINDOW_SIZE {1280, 720}
#elif defined(LLC_SAMPLE_GUI)
#	include "llc_sample_gui.h"
	typedef SSampleGUI SSampleContent;
#	define LLC_SAMPLE_TITLE "LLC GUI palettes, states and alignment"
#	define LLC_SAMPLE_WINDOW_SIZE {1280, 540}
#else
#	include "llc_sample_window.h"
	typedef SSampleWindow SSampleContent;
#	define LLC_SAMPLE_TITLE "LLC engine window sample"
#	define LLC_SAMPLE_WINDOW_SIZE {960, 540}
#endif

#if defined(LLC_SAMPLE_RUNTIME_MT)
#	define LLC_SAMPLE_RUNTIME_TITLE LLC_SAMPLE_TITLE " [MT]"
#else
#	define LLC_SAMPLE_RUNTIME_TITLE LLC_SAMPLE_TITLE
#endif

#if defined(LLC_WINDOWS)
#	include <Windowsx.h>

stct SWindowRuntimeWin32 {
	HWND	WindowHandle	= {};
};

stct SGDIApplication {
	HWND			WindowHandle	= {};
	::llc::n2u1_t	WindowSize	= {};
#if defined(LLC_SAMPLE_GUI_VIEWPORT) || defined(LLC_SAMPLE_GUI)
	::llc::img8bgra			Target			= {};
	::llc::SGUIRenderResources	GUIResources		= {};
	::llc::SFrameTimer		FrameTimer		= {};
	vltl ::llc::refcount_t		LastFrameMicroseconds	= 0;
#endif
#if defined(LLC_SAMPLE_GUI_VIEWPORT)
	::llc::img8bgra			ClientImages[SAMPLE_GUI_VIEWPORT_COUNT] = {};
	::llc::u2_t			FrameIndex		= 0;
#endif

	::llc::err_t	Initialize	(HWND windowHandle) nxpt;
	::llc::err_t	SetWindowSize	(::llc::n2u1_t size) nxpt;
	::llc::err_t	Present		(::llc::gc8bgra target) nxpt;
	::llc::err_t	Shutdown	() nxpt;
#if defined(LLC_SAMPLE_GUI_VIEWPORT) || defined(LLC_SAMPLE_GUI)
	::llc::err_t	MeasureText	(::llc::s1_t font, ::llc::vcst_t text, ::llc::n2u1_t & size) cnst;
	double		SecondsLastFrame() nxpt;
#endif
};

#if defined(LLC_SAMPLE_GUI_VIEWPORT) || defined(LLC_SAMPLE_GUI)
stct SSampleRenderFrame {
	::llc::n2u1_t			TargetSize	= {};
	::llc::TGUIRenderEventQueue	Events		= {};
	bool				NoDraw		= false;
};

stct SSampleFrameExchange {
	::llc::mutex				Lock		= {};
	::llc::apobj<SSampleRenderFrame>	Available	= {};
	::llc::pobj<SSampleRenderFrame>	Published	= {};
	::llc::pobj<SSampleRenderFrame>	Current		= {};

	::llc::err_t	Initialize	(::llc::u2_t frameCount);
	::llc::err_t	UpdateAcquire	(::llc::pobj<SSampleRenderFrame> & frame);
	::llc::err_t	UpdateCancel	(::llc::pobj<SSampleRenderFrame> & frame);
	::llc::err_t	UpdatePublish	(::llc::pobj<SSampleRenderFrame> & frame);
	::llc::err_t	DrawAcquire	(::llc::pobj<SSampleRenderFrame> & frame);
};
#endif

stct SApplication {
	SSampleContent		Sample		= {};
	SWindowRuntimeWin32	Runtime		= {};
	SGDIApplication		Presentation	= {};
#if defined(LLC_SAMPLE_GUI_VIEWPORT) || defined(LLC_SAMPLE_GUI)
	SSampleFrameExchange	Frames		= {};
#endif

	SApplication(::llc::SRuntimeValues&) {}
};

namespace
{
	stxp cnst char	WINDOW_CLASS_NAME[] = "LLC_SAMPLE_WINDOW_WIN32";

	SApplication* applicationFromHandle(HWND handle) nxpt {
		rtrn (SApplication*)::GetWindowLongPtrA(handle, GWLP_USERDATA);
	}

	void enqueueResult(::llc::err_t result) nxpt {
		if(::llc::failed(result))
			error_printf("Failed to enqueue window event with result: %i.", result);
	}

	LRESULT CALLBACK windowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
		if(WM_NCCREATE == message) {
			cnst CREATESTRUCTA* createInfo = (cnst CREATESTRUCTA*)lParam;
			::SetWindowLongPtrA(handle, GWLP_USERDATA, (LONG_PTR)createInfo->lpCreateParams);
		}

		SApplication* app = ::applicationFromHandle(handle);
		if(0 == app)
			rtrn ::DefWindowProcA(handle, message, wParam, lParam);
		::llc::SEngineWindow& window = app->Sample.Window;

		switch(message) {
		default: break;
		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize = {160, 120};
			rtrn 0;
		case WM_CLOSE:
			::enqueueResult(::llc::eventEnqueueScreenClose(window.EventQueue));
			window.Closed = true;
			rtrn 0;
		case WM_DESTROY:
			::enqueueResult(::llc::eventEnqueueScreenDestroy(window.EventQueue));
			app->Runtime.WindowHandle = 0;
			window.Closed = true;
			rtrn 0;
		case WM_NCDESTROY:
			::SetWindowLongPtrA(handle, GWLP_USERDATA, 0);
			break;
		case WM_CREATE: {
			RECT clientRect = {};
			::GetClientRect(handle, &clientRect);
			cnst ::llc::n2u1_t clientSize = {(::llc::u1_t)(clientRect.right - clientRect.left), (::llc::u1_t)(clientRect.bottom - clientRect.top)};
			::llc::SEventArgsScreenCreate eventArgs = {};
			eventArgs.Size = clientSize;
			::enqueueResult(::llc::eventEnqueueScreenCreate(window.EventQueue, eventArgs));
			break;
		}
		case WM_SHOWWINDOW:
			if(wParam) {
				::enqueueResult(::llc::eventEnqueueScreenShow(window.EventQueue, (::llc::u0_t)lParam));
				window.Repaint = true;
			}
			else
				::enqueueResult(::llc::eventEnqueueScreenHide(window.EventQueue, (::llc::u0_t)lParam));
			break;
		case WM_ACTIVATE:
			switch(LOWORD(wParam)) {
			default: break;
			case WA_ACTIVE:		::enqueueResult(::llc::eventEnqueueScreenActivate(window.EventQueue, 0)); break;
			case WA_CLICKACTIVE:	::enqueueResult(::llc::eventEnqueueScreenActivate(window.EventQueue, 1)); break;
			case WA_INACTIVE:	::enqueueResult(::llc::eventEnqueueScreenDeactivate(window.EventQueue, 0)); break;
			}
			break;
		case WM_CHAR:
			::enqueueResult(::llc::eventEnqueueTextChar(window.EventQueue, (::llc::u2_t)wParam));
			window.Repaint = true;
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if(wParam < ::llc::size(window.Input.KeyboardCurrent.KeyState))
				window.Input.KeyboardCurrent.KeyState[(::llc::u2_t)wParam] = 1;
			::enqueueResult(WM_KEYDOWN == message
				? ::llc::eventEnqueueKeyboardDown(window.EventQueue, (::llc::u0_t)wParam)
				: ::llc::eventEnqueueKeyboardSysDown(window.EventQueue, (::llc::u0_t)wParam));
			window.Repaint = true;
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if(wParam < ::llc::size(window.Input.KeyboardCurrent.KeyState))
				window.Input.KeyboardCurrent.KeyState[(::llc::u2_t)wParam] = 0;
			::enqueueResult(WM_KEYUP == message
				? ::llc::eventEnqueueKeyboardUp(window.EventQueue, (::llc::u0_t)wParam)
				: ::llc::eventEnqueueKeyboardSysUp(window.EventQueue, (::llc::u0_t)wParam));
			window.Repaint = true;
			break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN: {
			cnst ::llc::u0_t button = WM_LBUTTONDOWN == message ? 0 : WM_RBUTTONDOWN == message ? 1 : 2;
			window.Input.MouseCurrent.ButtonState[button] = 1;
			::enqueueResult(::llc::eventEnqueueMouseDown(window.EventQueue, button));
			window.Repaint = true;
			break;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP: {
			cnst ::llc::u0_t button = WM_LBUTTONUP == message ? 0 : WM_RBUTTONUP == message ? 1 : 2;
			window.Input.MouseCurrent.ButtonState[button] = 0;
			::enqueueResult(::llc::eventEnqueueMouseUp(window.EventQueue, button));
			window.Repaint = true;
			break;
		}
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK: {
			cnst ::llc::u0_t button = WM_LBUTTONDBLCLK == message ? 0 : WM_RBUTTONDBLCLK == message ? 1 : 2;
			window.Input.MouseCurrent.ButtonState[button] = 1;
			::enqueueResult(::llc::eventEnqueueMouseDoubleClick(window.EventQueue, button));
			window.Repaint = true;
			break;
		}
		case WM_MOUSEWHEEL:
			window.Input.MouseCurrent.Deltas.z = (::llc::s1_t)GET_WHEEL_DELTA_WPARAM(wParam);
			::enqueueResult(::llc::eventEnqueueMouseWheel(window.EventQueue, window.Input.MouseCurrent.Deltas.z));
			window.Repaint = true;
			break;
		case WM_MOUSEMOVE: {
			cnst ::llc::n2s1_t position = {(::llc::s1_t)GET_X_LPARAM(lParam), (::llc::s1_t)GET_Y_LPARAM(lParam)};
			window.Input.MouseCurrent.Position = position;
			window.Input.MouseCurrent.Deltas.x = position.x - window.Input.MousePrevious.Position.x;
			window.Input.MouseCurrent.Deltas.y = position.y - window.Input.MousePrevious.Position.y;
			::enqueueResult(::llc::eventEnqueueMousePosition(window.EventQueue, position));
			window.Repaint = true;
			break;
		}
		case WM_SIZE: {
			window.NoDraw = SIZE_MINIMIZED == wParam;
			if(false == window.NoDraw) {
				RECT clientRect = {};
				::GetClientRect(handle, &clientRect);
				cnst ::llc::n2u1_t size = {(::llc::u1_t)(clientRect.right - clientRect.left), (::llc::u1_t)(clientRect.bottom - clientRect.top)};
				::enqueueResult(::llc::engineWindowResize(window, size));
			}
			break;
		}
		case WM_ERASEBKGND:
			rtrn 1;
		case WM_PAINT: {
			PAINTSTRUCT paint = {};
			::BeginPaint(handle, &paint);
			::EndPaint(handle, &paint);
			window.Repaint = true;
			rtrn 0;
		}
		}
		rtrn ::DefWindowProcA(handle, message, wParam, lParam);
	}
}

#if defined(LLC_SAMPLE_GUI_VIEWPORT) || defined(LLC_SAMPLE_GUI)
::llc::err_t SSampleFrameExchange::Initialize(::llc::u2_t frameCount) {
	if_true_fe(frameCount < 3 || Available.size() || Published || Current);
	if_fail_fe(Available.reserve(frameCount));
	for(::llc::u2_t iFrame = 0; iFrame < frameCount; ++iFrame) {
		::llc::pobj<SSampleRenderFrame> frame = {};
		if_null_ve(::llc::OS_NO_MEMORY, frame.create());
		if_fail_fe(Available.push_back(frame));
	}
	rtrn 0;
}

::llc::err_t SSampleFrameExchange::UpdateAcquire(::llc::pobj<SSampleRenderFrame> & frame) {
	if_true_fe(frame);
	::llc::lock_guard<::llc::mutex> guard{Lock};
	if(Available.size()) {
		if_fail_fe(Available.pop_back(frame));
	}
	else if(Published) {
		frame = Published;
		Published = {};
	}
	rtrn frame ? 0 : ::llc::OS_BUSY;
}

::llc::err_t SSampleFrameExchange::UpdateCancel(::llc::pobj<SSampleRenderFrame> & frame) {
	if_null_fe(frame);
	::llc::lock_guard<::llc::mutex> guard{Lock};
	if_fail_fe(Available.push_back(frame));
	frame = {};
	rtrn 0;
}

::llc::err_t SSampleFrameExchange::UpdatePublish(::llc::pobj<SSampleRenderFrame> & frame) {
	if_null_fe(frame);
	::llc::lock_guard<::llc::mutex> guard{Lock};
	if(Published)
		if_fail_fe(Available.push_back(Published));
	Published = frame;
	frame = {};
	rtrn 0;
}

::llc::err_t SSampleFrameExchange::DrawAcquire(::llc::pobj<SSampleRenderFrame> & frame) {
	::llc::lock_guard<::llc::mutex> guard{Lock};
	if(Published) {
		if(Current)
			if_fail_fe(Available.push_back(Current));
		Current = Published;
		Published = {};
	}
	frame = Current;
	rtrn frame ? 0 : ::llc::OS_BUSY;
}
#endif

static ::llc::err_t windowCreate(SApplication& app, ::llc::vcst_t title, ::llc::n2u1_t size) nxpt {
	::llc::SEngineWindow& window = app.Sample.Window;
	if(0 == size.x || 0 == size.y)
		rtrn ::llc::OS_INVALID_PARAMETER;
	if(false == window.Closed || app.Runtime.WindowHandle)
		rtrn ::llc::OS_INVALID_PARAMETER;
	HINSTANCE instance = ::GetModuleHandleA(0);
	WNDCLASSEXA windowClass = {szof(WNDCLASSEXA)};
	windowClass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = ::windowProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = ::LoadCursorA(0, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.lpszClassName = ::WINDOW_CLASS_NAME;
	if(0 == ::RegisterClassExA(&windowClass) && ERROR_CLASS_ALREADY_EXISTS != ::GetLastError())
		rtrn ::llc::OS_ERROR;

	RECT windowRect = {100, 100, 100 + (LONG)size.x, 100 + (LONG)size.y};
	cnst DWORD style = WS_OVERLAPPEDWINDOW;
	if(FALSE == ::AdjustWindowRectEx(&windowRect, style, FALSE, 0))
		rtrn ::llc::OS_ERROR;

	::llc::string titleText = title;
	window.Size = size;
	window.PreviousSize = size;
	app.Runtime.WindowHandle = ::CreateWindowExA(0, ::WINDOW_CLASS_NAME, titleText.begin(), style
		, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top
		, 0, 0, instance, &app
		);
	if(0 == app.Runtime.WindowHandle)
		rtrn ::llc::OS_ERROR;

	window.Closed = false;
	window.Resized = true;
	window.Repaint = true;
	::ShowWindow(app.Runtime.WindowHandle, SW_SHOW);
	::UpdateWindow(app.Runtime.WindowHandle);
	rtrn 0;
}

static ::llc::err_t windowDestroy(SApplication& app) nxpt {
	if(app.Runtime.WindowHandle) {
		if(FALSE == ::DestroyWindow(app.Runtime.WindowHandle))
			rtrn ::llc::OS_ERROR;
	}
	app.Runtime = {};
	app.Sample.Window.Closed = true;
	rtrn 0;
}

static ::llc::err_t windowUpdate(SApplication& app) nxpt {
	::llc::SEngineWindow& window = app.Sample.Window;
	::llc::engineWindowBeginFrame(window);

	MSG message = {};
	while(::PeekMessageA(&message, 0, 0, 0, PM_REMOVE)) {
		if(WM_QUIT == message.message)
			window.Closed = true;
		else {
			::TranslateMessage(&message);
			::DispatchMessageA(&message);
		}
	}
#if !defined(LLC_SAMPLE_GUI_VIEWPORT) && !defined(LLC_SAMPLE_GUI)
	if(window.Resized)
		if_fail_fe(app.Presentation.SetWindowSize(window.Size));
#endif
	rtrn window.Closed ? ::llc::APPLICATION_STATE_EXIT : 0;
}

::llc::err_t SGDIApplication::Initialize(HWND windowHandle) nxpt {
	if(0 == windowHandle || WindowHandle)
		rtrn ::llc::OS_INVALID_PARAMETER;
	WindowHandle = windowHandle;
	RECT clientRect = {};
	if(FALSE == ::GetClientRect(WindowHandle, &clientRect))
		rtrn ::llc::OS_ERROR;
	if_fail_fe(SetWindowSize({(::llc::u1_t)(clientRect.right - clientRect.left), (::llc::u1_t)(clientRect.bottom - clientRect.top)}));
#if defined(LLC_SAMPLE_GUI_VIEWPORT) || defined(LLC_SAMPLE_GUI)
	FrameTimer.Timer.Reset();
	llc_sync_exchange(LastFrameMicroseconds, 0);
#endif
	rtrn 0;
}

::llc::err_t SGDIApplication::SetWindowSize(::llc::n2u1_t size) nxpt {
	if(0 == size.x || 0 == size.y)
		rtrn ::llc::OS_INVALID_PARAMETER;
	WindowSize = size;
#if defined(LLC_SAMPLE_GUI_VIEWPORT) || defined(LLC_SAMPLE_GUI)
	if(Target.metrics() != size.u2())
		if_fail_fe(Target.resize(size));
#endif
	rtrn 0;
}

::llc::err_t SGDIApplication::Present(::llc::gc8bgra target) nxpt {
	if(0 == target.size() || 0 == target.metrics().x || 0 == target.metrics().y)
		rtrn ::llc::OS_INVALID_PARAMETER;
	if(0 == WindowHandle || 0 == WindowSize.x || 0 == WindowSize.y)
		rtrn ::llc::OS_INVALID_PARAMETER;
	HDC deviceContext = ::GetDC(WindowHandle);
	if(0 == deviceContext)
		rtrn ::llc::OS_ERROR;

	BITMAPINFO bitmapInfo = {};
	bitmapInfo.bmiHeader.biSize = szof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biWidth = (LONG)target.metrics().x;
	bitmapInfo.bmiHeader.biHeight = -(LONG)target.metrics().y;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	::SetStretchBltMode(deviceContext, COLORONCOLOR);
	cnst int result = ::StretchDIBits(deviceContext, 0, 0, WindowSize.x, WindowSize.y, 0, 0, target.metrics().x, target.metrics().y
		, target.begin(), &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
	::ReleaseDC(WindowHandle, deviceContext);
	if(GDI_ERROR == result)
		rtrn ::llc::OS_ERROR;
#if defined(LLC_SAMPLE_GUI_VIEWPORT) || defined(LLC_SAMPLE_GUI)
	cnst double secondsLastFrame = FrameTimer.Frame();
	llc_sync_exchange(LastFrameMicroseconds, (::llc::refcount_t)(secondsLastFrame * 1000000.0));
#endif
	rtrn 0;
}

::llc::err_t SGDIApplication::Shutdown() nxpt {
	WindowHandle = {};
	WindowSize = {};
#if defined(LLC_SAMPLE_GUI_VIEWPORT) || defined(LLC_SAMPLE_GUI)
	GUIResources = {};
	llc_sync_exchange(LastFrameMicroseconds, 0);
#endif
#if defined(LLC_SAMPLE_GUI_VIEWPORT)
	FrameIndex = 0;
#endif
	rtrn 0;
}

#if defined(LLC_SAMPLE_GUI_VIEWPORT) || defined(LLC_SAMPLE_GUI)
::llc::err_t SGDIApplication::MeasureText(::llc::s1_t font, ::llc::vcst_t text, ::llc::n2u1_t & size) cnst {
	rtrn ::llc::guiMeasureTextRaster(GUIResources, font, text, size);
}

double SGDIApplication::SecondsLastFrame() nxpt {
	cnst ::llc::refcount_t microseconds = llc_sync_compare_exchange(LastFrameMicroseconds, 0, 0);
	rtrn microseconds > 0 ? microseconds / 1000000.0 : 0;
}

static ::llc::err_t sampleRenderResourcesSetup(SApplication & app) {
#if defined(LLC_SAMPLE_GUI_VIEWPORT)
	rtrn ::sampleGUIViewportRenderResourcesSetup(app.Sample, app.Presentation.GUIResources, app.Presentation.ClientImages);
#else
	rtrn ::sampleGUIRenderResourcesSetup(app.Sample, app.Presentation.GUIResources);
#endif
}

static ::llc::err_t sampleRenderFrameUpdate(SApplication & app) {
	::llc::pobj<SSampleRenderFrame> frame = {};
	if_fail_fe(app.Frames.UpdateAcquire(frame));
	frame->TargetSize = app.Sample.Window.Size;
	frame->NoDraw = app.Sample.Window.NoDraw;
	cnst ::llc::FGUITextMeasure textMeasure = [&app](::llc::s1_t font, ::llc::vcst_t text, ::llc::n2u1_t & size) { rtrn app.Presentation.MeasureText(font, text, size); };
#if defined(LLC_SAMPLE_GUI_VIEWPORT)
	cnst ::llc::err_t result = ::sampleGUIViewportRenderEvents(app.Sample, frame->Events, frame->TargetSize, textMeasure);
#else
	cnst ::llc::err_t result = ::sampleGUIRenderEvents(app.Sample, frame->Events, frame->TargetSize, textMeasure);
#endif
	if(::llc::failed(result)) {
		if_fail_fe(app.Frames.UpdateCancel(frame));
		rtrn result;
	}
	rtrn app.Frames.UpdatePublish(frame);
}

static ::llc::err_t sampleRenderDraw(SApplication & app) {
	::llc::pobj<SSampleRenderFrame> frame = {};
	if_fail_fe(app.Frames.DrawAcquire(frame));
	if(frame->NoDraw) {
		::llc::sleep(8);
		rtrn 0;
	}
	if(app.Presentation.WindowSize != frame->TargetSize)
		if_fail_fe(app.Presentation.SetWindowSize(frame->TargetSize));
#if defined(LLC_SAMPLE_GUI_VIEWPORT)
	if_fail_fe(::sampleGUIViewportDraw(frame->Events, app.Presentation.GUIResources, app.Presentation.ClientImages, app.Presentation.FrameIndex++, app.Presentation.Target.View));
#else
	if_fail_fe(::sampleGUIDraw(frame->Events, app.Presentation.GUIResources, app.Presentation.Target.View));
#endif
	rtrn app.Presentation.Present(app.Presentation.Target.View);
}
#endif

::llc::err_t setup(SApplication& app) {
	if_fail_fe(::windowCreate(app, LLC_SAMPLE_RUNTIME_TITLE, LLC_SAMPLE_WINDOW_SIZE));
	::llc::err_t result = app.Presentation.Initialize(app.Runtime.WindowHandle);
	if(::llc::not_failed(result)) {
	#if defined(LLC_SAMPLE_GUI_VIEWPORT)
		result = ::sampleGUIViewportSetup(app.Sample);
	#elif defined(LLC_SAMPLE_GUI)
		result = ::sampleGUISetup(app.Sample);
#else
		result = ::sampleWindowSetup(app.Sample);
#endif
	}
#if defined(LLC_SAMPLE_GUI_VIEWPORT) || defined(LLC_SAMPLE_GUI)
	if(::llc::not_failed(result))
		result = ::sampleRenderResourcesSetup(app);
	if(::llc::not_failed(result))
		result = app.Frames.Initialize(3);
	if(::llc::not_failed(result))
		result = ::sampleRenderFrameUpdate(app);
#endif
	if(::llc::failed(result)) {
		app.Presentation.Shutdown();
		::windowDestroy(app);
	}
	rtrn result;
}

::llc::err_t cleanup(SApplication& app) {
	cnst ::llc::err_t resultPresentation = app.Presentation.Shutdown();
	cnst ::llc::err_t resultWindow = ::windowDestroy(app);
	rtrn ::llc::failed(resultPresentation) ? resultPresentation : resultWindow;
}

::llc::err_t update(SApplication& app, bool systemRequestedExit) {
	cnst ::llc::err_t resultWindow = ::windowUpdate(app);
	if(::llc::APPLICATION_STATE_EXIT == resultWindow)
		rtrn resultWindow;
	if_fail_fe(resultWindow);
	cnst ::llc::err_t resultSample =
#if defined(LLC_SAMPLE_GUI_VIEWPORT)
		::sampleGUIViewportUpdate(app.Sample, systemRequestedExit, app.Presentation.SecondsLastFrame());
#elif defined(LLC_SAMPLE_GUI)
		::sampleGUIUpdate(app.Sample, systemRequestedExit);
#else
		::sampleWindowUpdate(app.Sample, systemRequestedExit);
#endif
#if defined(LLC_SAMPLE_GUI_VIEWPORT) || defined(LLC_SAMPLE_GUI)
	if(0 == resultSample)
		if_fail_fe(::sampleRenderFrameUpdate(app));
#endif
	::llc::sleep(8);
	rtrn resultSample;
}

::llc::err_t draw(SApplication& app) {
#if defined(LLC_SAMPLE_GUI_VIEWPORT) || defined(LLC_SAMPLE_GUI)
	rtrn ::sampleRenderDraw(app);
#else
	if_fail_fe(::sampleWindowDraw(app.Sample));
	if(app.Sample.Window.NoDraw)
		rtrn 0;
	rtrn app.Presentation.Present(app.Sample.Target.View);
#endif
}

#if defined(LLC_SAMPLE_RUNTIME_MT)
LLC_DEFINE_APPLICATION_ENTRY_POINT_MT(SApplication, LLC_SAMPLE_RUNTIME_TITLE)
#else
LLC_DEFINE_APPLICATION_ENTRY_POINT(SApplication, LLC_SAMPLE_RUNTIME_TITLE)
#endif

#endif // LLC_WINDOWS
