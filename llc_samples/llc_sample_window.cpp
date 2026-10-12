#include "llc_sample_window.h"
#include "llc_runtime.h"

::llc::err_t sampleWindowSetup(SSampleWindow& app) {
	rtrn app.Target.resize(app.Window.Size);
}

::llc::err_t sampleWindowUpdate(SSampleWindow& app, bool systemRequestedExit) {
	if(systemRequestedExit || app.Window.Closed || app.Window.Input.KeyDown(0x1B))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if(app.Target.metrics() != app.Window.Size.u2())
		if_fail_fe(app.Target.resize(app.Window.Size));
	rtrn 0;
}

::llc::err_t sampleWindowDraw(SSampleWindow& app) {
	cnst ::llc::n2u2_t metrics = app.Target.metrics();
	cnst ::llc::n2s1_t pointer = app.Window.Input.MouseCurrent.Position;
	for(::llc::u2_t y = 0; y < metrics.y; ++y)
	for(::llc::u2_t x = 0; x < metrics.x; ++x) {
		cnst ::llc::u0_t blue = (::llc::u0_t)((x + app.FrameIndex) & 0xFFU);
		cnst ::llc::u0_t green = (::llc::u0_t)((y + app.FrameIndex / 2U) & 0xFFU);
		cnst ::llc::u0_t red = (::llc::u0_t)(((x ^ y) + app.FrameIndex / 4U) & 0xFFU);
		cnst bool pointerAxis = (::llc::s2_t)x == pointer.x || (::llc::s2_t)y == pointer.y;
		cnst bool gridLine = 0 == (x % 64U) || 0 == (y % 64U);
		app.Target[y][x] = pointerAxis ? ::llc::SColorBGRA{0xFF, 0xFF, 0xFF} : gridLine ? ::llc::SColorBGRA{0x20, 0x20, 0x20} : ::llc::SColorBGRA{blue, green, red};
	}
	++app.FrameIndex;
	rtrn 0;
}
