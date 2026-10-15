#include "llc_sample_solar_system.h"

#include "llc_app_impl.h"
#include "llc_chrono.h"
#include "llc_frameinfo.h"
#include "llc_grid_color.h"
#include "llc_mutex.h"
#include "llc_string.h"
#include "llc_swap.h"
#include "llc_sync.h"

#define LLC_SAMPLE_SOLAR_SYSTEM_TITLE "LLC solar system"

#if defined(LLC_WINDOWS)
#	include <Windowsx.h>

stct SWindowRuntimeWin32 {
	HWND	WindowHandle	= {};
};

stct SGDIApplication {
	HWND				WindowHandle			= {};
	::llc::SFrameTimer	FrameTimer			= {};
	vltl ::llc::refcount_t	LastFrameMicroseconds	= 0;

	::llc::err_t	Initialize		(HWND windowHandle) nxpt;
	::llc::err_t	Present			(HDC deviceContext, ::llc::gc8bgra target) nxpt;
	::llc::err_t	Shutdown		() nxpt;
	double			SecondsLastFrame	() nxpt;
};

stct SApplication {
	SSampleSolarSystem	Sample		= {};
	SWindowRuntimeWin32	Runtime		= {};
	SGDIApplication		Presentation	= {};
	::llc::mutex		LockGUI		= {};
	::llc::mutex		LockRender	= {};
	::llc::pobj<::llc::img8bgra>
					FrontImage	= {};
	::llc::pobj<::llc::img8bgra>
					BackImage	= {};
	::llc::img8bgra		ClientImages[SAMPLE_GUI_VIEWPORT_COUNT] = {};
	vltl ::llc::refcount_t	LastUpdateMicroseconds = 0;
	vltl ::llc::refcount_t	LastDrawMicroseconds = 0;

	SApplication(::llc::SRuntimeValues&) {}
};

namespace {
	stxp cnst char WINDOW_CLASS_NAME[] = "LLC_SAMPLE_SOLAR_SYSTEM_WIN32";

	SApplication* applicationFromHandle(HWND handle) nxpt {
		rtrn (SApplication*)::GetWindowLongPtrA(handle, GWLP_USERDATA);
	}

	LRESULT CALLBACK windowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
		if(WM_NCCREATE == message) {
			cnst CREATESTRUCTA* createInfo = (cnst CREATESTRUCTA*)lParam;
			::SetWindowLongPtrA(handle, GWLP_USERDATA, (LONG_PTR)createInfo->lpCreateParams);
		}

		SApplication* app = ::applicationFromHandle(handle);
		if(0 == app)
			rtrn ::DefWindowProcA(handle, message, wParam, lParam);
		::llc::SEngineWindow & window = app->Sample.Window;

		switch(message) {
		default: break;
		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize = {160, 120};
			rtrn 0;
		case WM_CLOSE:
			if_fail_e(::llc::eventEnqueueScreenClose(window.EventQueue));
			window.Closed = true;
			rtrn 0;
		case WM_DESTROY:
			if_fail_e(::llc::eventEnqueueScreenDestroy(window.EventQueue));
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
			if_fail_e(::llc::eventEnqueueScreenCreate(window.EventQueue, eventArgs));
			break;
		}
		case WM_SHOWWINDOW:
			if(wParam) {
				if_fail_e(::llc::eventEnqueueScreenShow(window.EventQueue, (::llc::u0_t)lParam));
				window.Repaint = true;
			}
			else
				if_fail_e(::llc::eventEnqueueScreenHide(window.EventQueue, (::llc::u0_t)lParam));
			break;
		case WM_ACTIVATE:
			switch(LOWORD(wParam)) {
			default: break;
			case WA_ACTIVE:     if_fail_e(::llc::eventEnqueueScreenActivate(window.EventQueue, 0)); break;
			case WA_CLICKACTIVE: if_fail_e(::llc::eventEnqueueScreenActivate(window.EventQueue, 1)); break;
			case WA_INACTIVE:   if_fail_e(::llc::eventEnqueueScreenDeactivate(window.EventQueue, 0)); break;
			}
			break;
		case WM_CHAR:
			if_fail_e(::llc::eventEnqueueTextChar(window.EventQueue, (::llc::u2_t)wParam));
			window.Repaint = true;
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if(wParam < ::llc::size(window.Input.KeyboardCurrent.KeyState))
				window.Input.KeyboardCurrent.KeyState[(::llc::u2_t)wParam] = 1;
			if_fail_e(WM_KEYDOWN == message
				? ::llc::eventEnqueueKeyboardDown(window.EventQueue, (::llc::u0_t)wParam)
				: ::llc::eventEnqueueKeyboardSysDown(window.EventQueue, (::llc::u0_t)wParam));
			window.Repaint = true;
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if(wParam < ::llc::size(window.Input.KeyboardCurrent.KeyState))
				window.Input.KeyboardCurrent.KeyState[(::llc::u2_t)wParam] = 0;
			if_fail_e(WM_KEYUP == message
				? ::llc::eventEnqueueKeyboardUp(window.EventQueue, (::llc::u0_t)wParam)
				: ::llc::eventEnqueueKeyboardSysUp(window.EventQueue, (::llc::u0_t)wParam));
			window.Repaint = true;
			break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN: {
			cnst ::llc::u0_t button = WM_LBUTTONDOWN == message ? 0 : WM_RBUTTONDOWN == message ? 1 : 2;
			window.Input.MouseCurrent.ButtonState[button] = 1;
			if_fail_e(::llc::eventEnqueueMouseDown(window.EventQueue, button));
			window.Repaint = true;
			break;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP: {
			cnst ::llc::u0_t button = WM_LBUTTONUP == message ? 0 : WM_RBUTTONUP == message ? 1 : 2;
			window.Input.MouseCurrent.ButtonState[button] = 0;
			if_fail_e(::llc::eventEnqueueMouseUp(window.EventQueue, button));
			window.Repaint = true;
			break;
		}
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK: {
			cnst ::llc::u0_t button = WM_LBUTTONDBLCLK == message ? 0 : WM_RBUTTONDBLCLK == message ? 1 : 2;
			window.Input.MouseCurrent.ButtonState[button] = 1;
			if_fail_e(::llc::eventEnqueueMouseDoubleClick(window.EventQueue, button));
			window.Repaint = true;
			break;
		}
		case WM_MOUSEWHEEL:
			window.Input.MouseCurrent.Deltas.z = (::llc::s1_t)GET_WHEEL_DELTA_WPARAM(wParam);
			if_fail_e(::llc::eventEnqueueMouseWheel(window.EventQueue, window.Input.MouseCurrent.Deltas.z));
			window.Repaint = true;
			break;
		case WM_MOUSEMOVE: {
			cnst ::llc::n2s1_t position = {(::llc::s1_t)GET_X_LPARAM(lParam), (::llc::s1_t)GET_Y_LPARAM(lParam)};
			cnst bool positionChanged = position != window.Input.MouseCurrent.Position;
			window.Input.MouseCurrent.Position = position;
			window.Input.MouseCurrent.Deltas.x = position.x - window.Input.MousePrevious.Position.x;
			window.Input.MouseCurrent.Deltas.y = position.y - window.Input.MousePrevious.Position.y;
			if_fail_e(::llc::eventEnqueueMousePosition(window.EventQueue, position));
			if(positionChanged)
				window.Repaint = true;
			break;
		}
		case WM_SIZE: {
			window.NoDraw = SIZE_MINIMIZED == wParam;
			if(false == window.NoDraw) {
				RECT clientRect = {};
				::GetClientRect(handle, &clientRect);
				cnst ::llc::n2u1_t size = {(::llc::u1_t)(clientRect.right - clientRect.left), (::llc::u1_t)(clientRect.bottom - clientRect.top)};
				if_fail_e(::llc::engineWindowResize(window, size));
			}
			break;
		}
		case WM_ERASEBKGND:
			rtrn 1;
		case WM_PAINT: {
			PAINTSTRUCT paint = {};
			::BeginPaint(handle, &paint);
			{
				::llc::lock_guard<::llc::mutex> lock{app->LockRender};
				if(app->FrontImage)
					if_fail_e(app->Presentation.Present(paint.hdc, app->FrontImage->View));
			}
			::EndPaint(handle, &paint);
			rtrn 0;
		}
		}
		rtrn ::DefWindowProcA(handle, message, wParam, lParam);
	}
}

sttc ::llc::err_t windowCreate(SApplication & app, ::llc::vcst_t title, ::llc::n2u1_t size) nxpt {
	::llc::SEngineWindow & window = app.Sample.Window;
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

sttc ::llc::err_t windowDestroy(SApplication & app) nxpt {
	if(app.Runtime.WindowHandle)
		if(FALSE == ::DestroyWindow(app.Runtime.WindowHandle))
			rtrn ::llc::OS_ERROR;
	app.Runtime = {};
	app.Sample.Window.Closed = true;
	rtrn 0;
}

sttc ::llc::err_t windowUpdate(SApplication & app) nxpt {
	::llc::SEngineWindow & window = app.Sample.Window;
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
	rtrn window.Closed ? ::llc::APPLICATION_STATE_EXIT : 0;
}

::llc::err_t SGDIApplication::Initialize(HWND windowHandle) nxpt {
	if(0 == windowHandle || WindowHandle)
		rtrn ::llc::OS_INVALID_PARAMETER;
	WindowHandle = windowHandle;
	FrameTimer.Timer.Reset();
	llc_sync_exchange(LastFrameMicroseconds, 0);
	rtrn 0;
}

::llc::err_t SGDIApplication::Present(HDC deviceContext, ::llc::gc8bgra target) nxpt {
	if(0 == target.size() || 0 == target.metrics().x || 0 == target.metrics().y)
		rtrn ::llc::OS_INVALID_PARAMETER;
	if(0 == WindowHandle || 0 == deviceContext)
		rtrn ::llc::OS_INVALID_PARAMETER;
	RECT clientRect = {};
	if(FALSE == ::GetClientRect(WindowHandle, &clientRect))
		rtrn ::llc::OS_ERROR;
	cnst ::llc::n2u1_t clientSize = {(::llc::u1_t)(clientRect.right - clientRect.left), (::llc::u1_t)(clientRect.bottom - clientRect.top)};
	if(0 == clientSize.x || 0 == clientSize.y)
		rtrn 0;

	BITMAPINFO bitmapInfo = {};
	bitmapInfo.bmiHeader.biSize = szof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biWidth = (LONG)target.metrics().x;
	bitmapInfo.bmiHeader.biHeight = -(LONG)target.metrics().y;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	::SetStretchBltMode(deviceContext, COLORONCOLOR);
	cnst int result = ::StretchDIBits(deviceContext, 0, 0, clientSize.x, clientSize.y, 0, 0, target.metrics().x, target.metrics().y
		, target.begin(), &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
	if(GDI_ERROR == result)
		rtrn ::llc::OS_ERROR;
	cnst double secondsLastFrame = FrameTimer.Frame();
	llc_sync_exchange(LastFrameMicroseconds, (::llc::refcount_t)(secondsLastFrame * 1000000.0));
	rtrn 0;
}

::llc::err_t SGDIApplication::Shutdown() nxpt {
	WindowHandle = {};
	llc_sync_exchange(LastFrameMicroseconds, 0);
	rtrn 0;
}

double SGDIApplication::SecondsLastFrame() nxpt {
	cnst ::llc::refcount_t microseconds = llc_sync_compare_exchange(LastFrameMicroseconds, 0, 0);
	rtrn microseconds > 0 ? microseconds / 1000000.0 : 0;
}

::llc::err_t setup(SApplication & app) {
	if_fail_fe(::windowCreate(app, LLC_CXS(LLC_SAMPLE_SOLAR_SYSTEM_TITLE), {1600, 960}));
	::llc::err_t result = app.Presentation.Initialize(app.Runtime.WindowHandle);
	if(::llc::not_failed(result))
		result = ::sampleSolarSystemSetup(app.Sample);
	if(::llc::failed(result)) {
		app.Presentation.Shutdown();
		::windowDestroy(app);
	}
	rtrn result;
}

::llc::err_t cleanup(SApplication & app) {
	app.FrontImage = {};
	app.BackImage = {};
	cnst ::llc::err_t resultPresentation = app.Presentation.Shutdown();
	cnst ::llc::err_t resultWindow = ::windowDestroy(app);
	rtrn ::llc::failed(resultPresentation) ? resultPresentation : resultWindow;
}

::llc::err_t update(SApplication & app, bool systemRequestedExit) {
	cnst ::llc::s3_t timeStart = ::llc::timeCurrentInUs();
	::llc::err_t resultWindow = 0;
	{
		::llc::lock_guard<::llc::mutex> lock{app.LockGUI};
		resultWindow = ::windowUpdate(app);
	}
	if(::llc::APPLICATION_STATE_EXIT == resultWindow)
		rtrn resultWindow;
	if_fail_fe(resultWindow);
	::llc::err_t resultSample = 0;
	{
		::llc::lock_guard<::llc::mutex> lock{app.LockGUI};
		cnst double secondsUpdate = llc_sync_compare_exchange(app.LastUpdateMicroseconds, 0, 0) * .000001;
		cnst double secondsDraw = llc_sync_compare_exchange(app.LastDrawMicroseconds, 0, 0) * .000001;
		resultSample = ::sampleSolarSystemUpdate(app.Sample, systemRequestedExit, app.Presentation.SecondsLastFrame(), secondsUpdate, secondsDraw);
	}
	llc_sync_exchange(app.LastUpdateMicroseconds, (::llc::refcount_t)(::llc::timeCurrentInUs() - timeStart));
	::llc::sleep(1);
	rtrn resultSample;
}

::llc::err_t draw(SApplication & app) {
	cnst ::llc::s3_t timeStart = ::llc::timeCurrentInUs();
	{
		::llc::lock_guard<::llc::mutex> lock{app.LockGUI};
		if(app.Sample.Window.NoDraw) {
			::llc::sleep(8);
			rtrn 0;
		}
		if(!app.BackImage)
			if_null_ve(::llc::OS_NO_MEMORY, app.BackImage.create());
		if_fail_fe(app.BackImage->resize(app.Sample.Window.Size.u2()));
		if_fail_fe(::sampleSolarSystemDraw(app.Sample, app.ClientImages, app.BackImage->View));
	}
	{
		::llc::lock_guard<::llc::mutex> lock{app.LockRender};
		::llc::swap(app.FrontImage, app.BackImage);
	}
	if(app.Runtime.WindowHandle)
		if(FALSE == ::InvalidateRect(app.Runtime.WindowHandle, 0, FALSE))
			rtrn ::llc::OS_ERROR;
	llc_sync_exchange(app.LastDrawMicroseconds, (::llc::refcount_t)(::llc::timeCurrentInUs() - timeStart));
	rtrn 0;
}

LLC_DEFINE_APPLICATION_ENTRY_POINT(SApplication, LLC_SAMPLE_SOLAR_SYSTEM_TITLE)

#endif // LLC_WINDOWS
