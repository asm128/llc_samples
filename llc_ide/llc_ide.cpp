#include "llc_ide.h"
#include "../llc_file_selection.h"

#include "llc_apod_serialize.h"
#include "llc_app_impl.h"
#include "llc_encoding.h"
#include "llc_file.h"
#include "llc_cstring.h"
#include "llc_framework.h"
#include "llc_gui_control_list.h"
#include "llc_json_expression.h"
#include "llc_path.h"
#include "llc_runtime.h"
#include "llc_system_key.h"
#include <filesystem>

stxp ::llc::u2_t DESKTOP_VISIBLE_ROWS = 20;
stxp ::llc::vcst_c SCREENSHOT_PATH_DEFAULT = "d:/enterprise/logs/screenshots";
stxp ::llc::u2_t DESKTOP_FILE_MAX_BYTES = 1024 * 1024;

stxp ::llc::u0_t SAMPLE_GUI_THEME_COUNT = 3;
stxp ::llc::u0_t PALETTE_SHADE_COUNT = 16;

sttc ::llc::SColorBGRA sampleGUIThemeColor(::llc::u0_t theme) nxpt {
	stxp ::llc::SColorBGRA colors[SAMPLE_GUI_THEME_COUNT] =
		{ {248, 208,  72}
		, { 32, 176, 248}
		, {232, 232, 232}
		};
	rtrn colors[theme % SAMPLE_GUI_THEME_COUNT];
}

sttc ::llc::SColorBGRA sampleGUIShade(::llc::SColorBGRA color, ::llc::u0_t shade) nxpt {
	shade = ::llc::min(shade, (::llc::u0_t)(PALETTE_SHADE_COUNT - 1));
	rtrn color * ((PALETTE_SHADE_COUNT - (::llc::f2_t)shade) / PALETTE_SHADE_COUNT);
}

sttc ::llc::SColorBGRA sampleGUIDesaturate(::llc::SColorBGRA color) nxpt {
	cnst ::llc::u0_t gray = (::llc::u0_t)(((::llc::u2_t)color.r + color.g + color.b) / 3);
	rtrn
		{ (::llc::u0_t)((color.b + (::llc::u1_t)gray * 2) / 3)
		, (::llc::u0_t)((color.g + (::llc::u1_t)gray * 2) / 3)
		, (::llc::u0_t)((color.r + (::llc::u1_t)gray * 2) / 3)
		};
}

sttc ::llc::u0_t sampleGUIStateShade(::llc::u0_t state) nxpt {
	stxp ::llc::u0_t shades[::llc::GUI_CONTROL_PALETTE_COUNT] = {8, 10, 5, 11, 4, 7, 2, 7, 1, 13};
	rtrn shades[state % ::llc::GUI_CONTROL_PALETTE_COUNT];
}

sttc ::llc::u0_t sampleGUIStateBorderOffset(::llc::u0_t state) nxpt {
	stxp ::llc::u0_t offsets[::llc::GUI_CONTROL_PALETTE_COUNT] = {5, 1, 7, 3, 0, 1, 7, 3, 5, 5};
	rtrn offsets[state % ::llc::GUI_CONTROL_PALETTE_COUNT];
}

sttc ::llc::SColorBGRA sampleGUITextColor(::llc::SColorBGRA background) nxpt {
	cnst ::llc::u2_t luminance = (::llc::u2_t)background.r * 299 + (::llc::u2_t)background.g * 587 + (::llc::u2_t)background.b * 114;
	rtrn ::sampleGUIShade({255, 255, 255}, luminance > 150000 ? 15 : 1);
}

sttc ::llc::err_t sampleGUIResourcesCreate(::llc::SGUI & gui) {
	if_true_fe(0 == gui.Colors.create());
	if_fail_fe(gui.Colors->Palettes.resize(SAMPLE_GUI_THEME_COUNT * ::llc::GUI_CONTROL_PALETTE_COUNT));
	for(::llc::u0_t iTheme = 0; iTheme < SAMPLE_GUI_THEME_COUNT; ++iTheme)
	for(::llc::u0_t iState = 0; iState < ::llc::GUI_CONTROL_PALETTE_COUNT; ++iState) {
		::llc::astatic<::llc::bgra, ::llc::UI_CONTROL_AREA_COUNT> & palette = gui.Colors->Palettes[iTheme * ::llc::GUI_CONTROL_PALETTE_COUNT + iState];
		::llc::SColorBGRA tone = ::sampleGUIThemeColor(iTheme);
		if(iState == ::llc::GUI_CONTROL_PALETTE_DISABLED || iState == ::llc::GUI_CONTROL_PALETTE_SELECTED_DISABLED)
			tone = ::sampleGUIDesaturate(tone);
		cnst ::llc::u0_t shade = ::sampleGUIStateShade(iState);
		cnst ::llc::SColorBGRA base = ::sampleGUIShade(tone, shade);
		cnst ::llc::u0_t borderOffset = ::sampleGUIStateBorderOffset(iState);
		cnst ::llc::u0_t borderShade = shade > 7
			? (::llc::u0_t)(shade > borderOffset ? shade - borderOffset : 0)
			: (::llc::u0_t)::llc::min((::llc::u2_t)shade + borderOffset, (::llc::u2_t)PALETTE_SHADE_COUNT - 1);
		cnst ::llc::SColorBGRA borderColor = ::sampleGUIShade(tone, borderShade);
		for(::llc::u2_t iArea = 0; iArea < palette.size(); ++iArea)
			palette[iArea] = ::sampleGUIShade(tone, (::llc::u0_t)::llc::min((::llc::u2_t)shade + 5, (::llc::u2_t)PALETTE_SHADE_COUNT - 1));
		palette[::llc::UI_CONTROL_AREA_BACKGROUND] = ::sampleGUIShade(tone, (::llc::u0_t)::llc::min((::llc::u2_t)shade + 6, (::llc::u2_t)PALETTE_SHADE_COUNT - 1));
		palette[::llc::UI_CONTROL_AREA_CLIENT] = base;
		palette[::llc::UI_CONTROL_AREA_BORDER_LEFT] = borderColor;
		palette[::llc::UI_CONTROL_AREA_BORDER_TOP] = borderColor;
		palette[::llc::UI_CONTROL_AREA_BORDER_RIGHT] = borderColor;
		palette[::llc::UI_CONTROL_AREA_BORDER_BOTTOM] = borderColor;
		palette[::llc::UI_CONTROL_AREA_TEXT_BACKGROUND] = ::sampleGUIShade(tone, (::llc::u0_t)::llc::min((::llc::u2_t)shade + 2, (::llc::u2_t)PALETTE_SHADE_COUNT - 1));
		palette[::llc::UI_CONTROL_AREA_TEXT_FACE] = ::sampleGUITextColor(base);
	}
	::llc::SRasterFontManager fonts = {};
	if_fail_fe(::llc::rasterFontDefaults(fonts));
	gui.Fonts = fonts.Fonts;
	rtrn 0;
}

sttc void sampleGUIControlThemeApply(::llc::SGUI & gui, ::llc::cid_t idControl, ::llc::u0_t theme) nxpt {
	if(::llc::controlInvalid(gui, idControl))
		rtrn;
	::llc::SControlDraw & draw = gui.Controls.Draw[idControl];
	draw.UseNewPalettes = true;
	for(::llc::u0_t iState = 0; iState < ::llc::GUI_CONTROL_PALETTE_COUNT; ++iState)
		draw.Palettes[iState] = (::llc::s1_t)((theme % SAMPLE_GUI_THEME_COUNT) * ::llc::GUI_CONTROL_PALETTE_COUNT + iState);
}

sttc ::llc::err_t sampleGUIViewportDoubleClick(cnst ::llc::TQueueSystemEvent & events, bool & doubleClick) {
	doubleClick = false;
	for(::llc::u2_t iEvent = 0; iEvent < events.size(); ++iEvent) {
		cnst ::llc::pobj<::llc::SEventSystem> & eventSystem = events[iEvent];
		if(0 == eventSystem.get_ref() || eventSystem->Type != ::llc::SYSTEM_EVENT_Mouse)
			continue;
		::llc::SEView<::llc::EVENT_MOUSE> eventMouse = {};
		if_fail_fe(eventSystem->ExtractChild(eventMouse));
		if(eventMouse.Type != ::llc::EVENT_MOUSE_DoubleClick)
			continue;
		::llc::vcu0_t input = eventMouse.Data;
		::llc::u0_t button = 0;
		if_fail_fe(::llc::loadPOD(input, button));
		if_true_fef(input.size(), "Unexpected mouse double-click payload tail:%u.", input.size());
		if(0 == button)
			doubleClick = true;
	}
	rtrn 0;
}

sttc bool sampleGUIViewportPressed(cnst SSampleGUIViewport & app, cnst ::llc::SViewport & viewport) nxpt {
	rtrn ::llc::viewportIsPressed(app.GUI, viewport);
}

sttc ::llc::err_t sampleGUIViewportActivate(SSampleGUIViewport & app, ::llc::u0_t iViewport) {
	rtrn ::llc::viewportActivate(app.GUI, app.Desktop, app.Viewports, iViewport);
}

sttc void sampleGUIViewportImageUpdate(::llc::img8bgra & image, ::llc::u0_t theme, ::llc::u2_t frameIndex) nxpt {
	cnst ::llc::SColorBGRA tone = ::sampleGUIThemeColor(theme);
	cnst ::llc::bgra background = ::sampleGUIShade(tone, 14);
	cnst ::llc::bgra grid = ::sampleGUIShade(tone, 9);
	cnst ::llc::bgra pulse = ::sampleGUIShade(tone, 2);
	for(::llc::u2_t y = 0; y < image.metrics().y; ++y)
	for(::llc::u2_t x = 0; x < image.metrics().x; ++x) {
		cnst bool onGrid = 0 == x % 32 || 0 == y % 24;
		cnst bool onPulse = ((x + y + frameIndex) % 160) < 3;
		image[y][x] = onPulse ? pulse : onGrid ? grid : background;
	}
}

sttc ::llc::err_t sampleGUIViewportCreate(SSampleGUIViewport & app, ::llc::u0_t iViewport, ::llc::vcst_t title, cnst ::llc::rect2s1_t & area) {
	if_true_fe(iViewport >= app.Viewports.size());
	::llc::SViewport & viewport = app.Viewports[iViewport];
	if_fail_fe(::llc::guiCreateViewport(app.GUI, viewport, app.Desktop, app.ViewportActions[iViewport], 1 == iViewport, 1 == iViewport, 1 == iViewport));
	::llc::SControlPlacement & root = app.GUI.Controls.Placement[viewport.IdControl];
	root.Area = area;
	root.Border = {3, 3, 3, 3};
	root.Margin = {1, 1, 1, 1};
	cnst ::llc::cid_t idTitle = viewport.IdControls[::llc::VIEWPORT_CONTROL_TITLE];
	cnst ::llc::cid_t idClient = viewport.IdControls[::llc::VIEWPORT_CONTROL_TARGET];
	app.GUI.Controls.Text[idTitle].Text = title;
	app.GUI.Controls.Text[idTitle].FontSelected = 7;
	app.GUI.Controls.Placement[idTitle].Border = {2, 2, 2, 2};
	app.GUI.Controls.Placement[idClient].Area.Size = {(::llc::s1_t)(area.Size.x - 8), (::llc::s1_t)(area.Size.y - 32)};
	app.GUI.Controls.Placement[idClient].Border = {1, 1, 1, 1};
	::sampleGUIControlThemeApply(app.GUI, viewport.IdControl, iViewport);
	for(::llc::u0_t iControl = 0; iControl < viewport.IdControls.size(); ++iControl)
		::sampleGUIControlThemeApply(app.GUI, viewport.IdControls[iControl], iViewport);
	for(::llc::u0_t iControl = 0; iControl < app.ViewportActions[iViewport].size(); ++iControl)
		::sampleGUIControlThemeApply(app.GUI, app.ViewportActions[iViewport][iControl], iViewport);
	viewport.UnfoldedSize = area.Size;
	rtrn ::llc::controlMetricsInvalidate(app.GUI, viewport.IdControl);
}

sttc ::llc::err_t sampleGUIViewportFold(SSampleGUIViewport & app, ::llc::u0_t iViewport) {
	if_true_fe(iViewport >= app.Viewports.size());
	rtrn ::llc::viewportFold(app.GUI, app.Viewports[iViewport]);
}

sttc ::llc::err_t sampleGUIViewportPerformanceUpdate(SSampleGUIViewport & app, double secondsLastFrame) {
	if(secondsLastFrame <= 0)
		rtrn 0;
	cnst ::llc::err_t textLength = ::llc::sprintf_s(app.PerformanceText.Storage, "FPS %.2f    Frame %.2f ms", 1.0 / secondsLastFrame, secondsLastFrame * 1000.0);
	if_fail_fe(textLength);
	app.GUI.Controls.Text[app.Performance].Text = {app.PerformanceText.Storage, (::llc::u2_t)textLength};
	app.GUI.Controls.SetUpdated(app.Performance, false);
	rtrn 0;
}

sttc ::llc::err_t desktopListCreate(SSampleGUIViewport & app, ::llc::u0_t iViewport, ::llc::acid & controls) {
	::llc::vcst_t labels[DESKTOP_VISIBLE_ROWS] = {};
	::llc::SControlPlacement row = {};
	row.Area.Size = {0, 21};
	row.Border = {1, 1, 1, 1};
	row.Margin = {3, 1, 3, 1};
	cnst ::llc::cid_t parent = app.Viewports[iViewport].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
	if_fail_fe(::llc::guiCreateControlList(app.GUI, ::llc::DIRECTION_DOWN, {labels, DESKTOP_VISIBLE_ROWS}, parent, row, ::llc::ALIGN_CENTER_LEFT, controls));
	for(::llc::u2_t i = 0; i < controls.size(); ++i) {
		app.GUI.Controls.Text[controls[i]].FontSelected = 7;
		::sampleGUIControlThemeApply(app.GUI, controls[i], iViewport);
	}
	rtrn 0;
}

sttc ::llc::err_t desktopFolderRowsRefresh(SSampleGUIViewport & app) {
	for(::llc::u2_t i = 0; i < app.FolderControls.size(); ++i) {
		cnst ::llc::cid_t id = app.FolderControls[i];
		cnst ::llc::u2_t entry = i ? app.FolderOffset + i - 1 : 0;
		cnst bool visible = 0 == i || entry < app.FolderEntries.Name.size();
		app.GUI.Controls.SetHidden(id, !visible);
		if(!visible)
			continue;
		::llc::vcst_t label = 0 == i ? ::llc::vcst_t{".."} : app.FolderEntries.Name[entry];
		if_fail_fe(label.slice(label, 0, ::llc::min(label.size(), (::llc::u2_t)38)));
		app.GUI.Controls.Text[id].Text = label;
		::sampleGUIControlThemeApply(app.GUI, id, 0 == i ? 2 : app.FolderEntries.Info[entry].Folder ? 1 : 0);
	}
	rtrn 0;
}

sttc ::llc::err_t desktopFileRowsRefresh(SSampleGUIViewport & app) {
	cnst bool editorText = app.OpenFile.size() && app.FileBytes.find('\0') < 0;
	for(::llc::u2_t i = 0; i < app.FileControls.size(); ++i) {
		cnst ::llc::cid_t id = app.FileControls[i];
		cnst ::llc::u2_t line = app.FileOffset + i;
		cnst bool visible = line < app.TextRows.size() || (0 == i && 0 == app.TextRows.size());
		app.GUI.Controls.SetHidden(id, !visible);
		if(visible) {
			::llc::vcst_t text = app.TextRows.size() ? app.TextRows[line].Text : ::llc::vcst_t{"Select a file"};
			if(!app.WordWrap && app.FileColumnOffset < text.size())
				if_fail_fe(text.slice(text, app.FileColumnOffset));
			if(!app.WordWrap && app.FileColumnOffset >= text.size())
				text = {};
			if_fail_fe(text.slice(text, 0, ::llc::min(text.size(), ::llc::guiViewportTextColumns(app.GUI, id))));
			app.GUI.Controls.Text[id].Text = text;
		}
		::llc::u2_t lineBegin = 0;
		if(editorText && visible && line < app.TextRows.size()) {
			cnst ::llc::SGUIViewportTextRow & row = app.TextRows[line];
			lineBegin = ::fileViewerLineOffset(app.Editor, row.SourceLine) + row.SourceColumn + (app.WordWrap ? 0 : app.FileColumnOffset);
		}
		::llc::apod<::llc::STextState> & textStates = app.GUI.Controls.TextState[id];
		if_fail_fe(textStates.resize(app.GUI.Controls.Text[id].Text.size(), ::llc::STextState{}));
		for(::llc::u2_t iCharacter = 0; iCharacter < textStates.size(); ++iCharacter) {
			bool selected = false;
			for(::llc::u2_t iSelection = 0; iSelection < app.Editor.TextSelections.size(); ++iSelection) {
				cnst ::llc::rangeu2_t selection = app.Editor.TextSelections[iSelection];
				if(editorText && selection.Count && lineBegin + iCharacter >= selection.Offset && lineBegin + iCharacter < selection.Offset + selection.Count) {
					selected = true;
					break;
				}
			}
			textStates[iCharacter].SetSelected(id, iCharacter, app.GUI.Controls.EventQueue, selected);
			textStates[iCharacter].SetSwapColors(id, iCharacter, app.GUI.Controls.EventQueue, selected);
		}
	}
	app.GUI.Controls.SetHidden(app.Editor.IdCursor, true);
	if(editorText)
		for(::llc::u2_t i = 0; i < app.FileControls.size() && app.FileOffset + i < app.TextRows.size(); ++i) {
			cnst ::llc::SGUIViewportTextRow & row = app.TextRows[app.FileOffset + i];
			cnst ::llc::u2_t firstColumn = row.SourceColumn + (app.WordWrap ? 0 : app.FileColumnOffset);
			if(row.SourceLine != app.Editor.Cursor.y || app.Editor.Cursor.x < firstColumn || app.Editor.Cursor.x > firstColumn + app.GUI.Controls.Text[app.FileControls[i]].Text.size())
				continue;
			cnst ::llc::u0_t characterWidth = ::llc::max((::llc::u0_t)1, app.GUI.Fonts[7]->CharSize.x);
			app.GUI.Controls.Placement[app.Editor.IdCursor].Area.Offset = app.GUI.Controls.Placement[app.FileControls[i]].Area.Offset + ::llc::n2s1_t{(::llc::s1_t)((app.Editor.Cursor.x - firstColumn) * characterWidth), 0};
			app.GUI.Controls.SetHidden(app.Editor.IdCursor, false);
			if_fail_fe(::llc::controlMetricsInvalidate(app.GUI, app.Editor.IdCursor));
			break;
		}
	rtrn 0;
}

sttc ::llc::err_t desktopTextRowsRebuild(SSampleGUIViewport & app) {
	app.VisibleColumns = ::llc::guiViewportTextColumns(app.GUI, app.FileControls[0]);
	cnst bool editorText = app.OpenFile.size() && app.FileBytes.find('\0') < 0;
	if(editorText)
		if_fail_fe(::llc::guiViewportTextRowsBuild(app.Editor.FileLines, app.VisibleColumns, app.WordWrap, app.TextRows));
	if(!editorText)
		if_fail_fe(::llc::guiViewportTextRowsBuild(app.FileLines, app.VisibleColumns, app.WordWrap, app.TextRows));
	app.TextBytesBegin = app.Editor.FileText.begin();
	app.TextBytesSize = app.Editor.FileText.size();
	app.TextLineCount = app.Editor.FileLines.size();
	app.FileOffset = ::llc::min(app.FileOffset, app.TextRows.size() > DESKTOP_VISIBLE_ROWS ? app.TextRows.size() - DESKTOP_VISIBLE_ROWS : 0);
	rtrn ::desktopFileRowsRefresh(app);
}

sttc ::llc::err_t desktopFolderOpen(SSampleGUIViewport & app, ::llc::vcst_t path) {
	for(::llc::u2_t i = 0; i < app.FolderControls.size(); ++i)
		if_fail_fe(::llc::controlTextSet(app.GUI, app.FolderControls[i], {}));
	app.GUI.Controls.Text[app.Viewports[0].IdControls[::llc::VIEWPORT_CONTROL_TITLE]].Text = {};
	app.FolderPath = path;
	if_fail_fe(::llc::pathList(app.FolderEntries, app.FolderPath, [] (bool folder, ::llc::vcst_c &) -> ::llc::err_t { rtrn folder ? 2 : 0; }));
	app.FolderOffset = 0;
	if_fail_fe(::desktopFolderRowsRefresh(app));
	::llc::vcst_t folderName = {};
	if_fail_fe(::llc::pathFilename(app.FolderPath, folderName));
	app.GUI.Controls.Text[app.Viewports[0].IdControls[::llc::VIEWPORT_CONTROL_TITLE]].Text = folderName.size() ? folderName : ::llc::vcst_t{"ROOT"};
	for(::llc::u2_t i = 0; i < app.FileControls.size(); ++i)
		if_fail_fe(::llc::controlTextSet(app.GUI, app.FileControls[i], {}));
	app.GUI.Controls.Text[app.Viewports[1].IdControls[::llc::VIEWPORT_CONTROL_TITLE]].Text = "FILE CONTENT";
	if_fail_fe(app.TextRows.clear());
	if_fail_fe(app.FileLines.clear());
	if_fail_fe(app.FileBytes.clear());
	if_fail_fe(app.FileDisplay.clear());
	if_fail_fe(app.OpenFile.clear());
	app.Editor.TextSelections.clear();
	app.Editor.SelectionAnchor = app.Editor.SelectionActive = (::llc::u2_t)-1;
	app.FileOffset = 0;
	app.FileColumnOffset = 0;
	app.GUI.Controls.SetHidden(app.Keyboard.IdRoot, true);
	app.GUI.Controls.SetHidden(app.Editor.IdKeyboard, true);
	app.GUI.Controls.SetHidden(app.Editor.IdCursor, true);
	app.GUI.Controls.SetDisabled(app.ViewportActions[1][::llc::GUI_VIEWPORT_ACTION_SAVE], true);
	app.GUI.Controls.SetDisabled(app.ViewportActions[1][::llc::GUI_VIEWPORT_ACTION_SELECT_ALL], true);
	rtrn ::desktopFileRowsRefresh(app);
}

sttc ::llc::err_t desktopEditorSelectionUpdate(SSampleGUIViewport & app) {
	SFileEditor & editor = app.Editor;
	cnst ::llc::SInput & input = *app.Window.Input;
	::llc::n2u2_t cursor = {};
	::llc::u2_t characterOffset = 0;
	bool overTextLine = false;
	cnst ::llc::u0_t characterWidth = ::llc::max((::llc::u0_t)1, app.GUI.Fonts[7]->CharSize.x);
	for(::llc::u2_t iControl = 0; iControl < app.FileControls.size(); ++iControl) {
		cnst ::llc::cid_t idControl = app.FileControls[iControl];
		if(app.GUI.Controls.IsHidden(idControl) || false == ::llc::in_range(app.GUI.CursorPos.s1(), app.GUI.Controls.Area[idControl].Total.Global))
			continue;
		cnst ::llc::u2_t iRow = app.FileOffset + iControl;
		if(iRow >= app.TextRows.size())
			continue;
		cnst ::llc::SGUIViewportTextRow & row = app.TextRows[iRow];
		cnst ::llc::u2_t iLine = row.SourceLine;
		if(iLine >= editor.FileLines.size())
			continue;
		cnst ::llc::s2_t mouseOffsetX = (::llc::s2_t)input.MouseCurrent.Position.x - app.GUI.Controls.Area[idControl].Text.Offset.x;
		cnst ::llc::u2_t visibleColumn = 0 >= mouseOffsetX ? 0 : (::llc::u2_t)(mouseOffsetX / characterWidth);
		cnst ::llc::u2_t iColumn = ::llc::min(row.SourceColumn + (app.WordWrap ? 0 : app.FileColumnOffset) + visibleColumn, editor.FileLines[iLine].size());
		cursor = {iColumn, iLine};
		characterOffset = ::fileViewerLineOffset(editor, iLine) + iColumn;
		overTextLine = true;
		break;
	}
	if(input.ButtonDown(0) && overTextLine) {
		editor.Cursor = cursor;
	}
	if(input.MouseCurrent.ButtonState[0] && overTextLine)
		editor.Cursor = cursor;
	rtrn ::fileViewerSelectionUpdate(editor.TextSelections, editor.SelectionAnchor, editor.SelectionActive, editor.CapsSelectionMode, input, characterOffset, overTextLine);
}

sttc ::llc::err_t desktopFileOpen(SSampleGUIViewport & app, ::llc::vcst_t path) {
	for(::llc::u2_t i = 0; i < app.FileControls.size(); ++i)
		if_fail_fe(::llc::controlTextSet(app.GUI, app.FileControls[i], {}));
	if_fail_fe(app.TextRows.clear());
	app.GUI.Controls.Text[app.Viewports[1].IdControls[::llc::VIEWPORT_CONTROL_TITLE]].Text = "FILE CONTENT";
	if_fail_fe(app.Editor.FileLines.clear());
	if_fail_fe(app.FileLines.clear());
	if_fail_fe(app.FileBytes.clear());
	if_fail_fe(app.FileDisplay.clear());
	app.OpenFile = path;
	cnst ::llc::s3_t fileLength = ::llc::fileSize(app.OpenFile);
	if_true_fe(fileLength < 0);
	::llc::vcst_t fileName = {};
	if_fail_fe(::llc::pathFilename(app.OpenFile, fileName));
	cnst ::llc::err_t extensionAt = ::llc::rfind_string(".json", fileName);
	if(extensionAt >= 0 && (::llc::u2_t)extensionAt + 5 == fileName.size()) {
		if_fail_fe(app.JSONDocument.Reader.Tree.clear());
		if_fail_fe(app.JSONDocument.Reader.Reset());
		if_fail_fe(app.JSONDocument.Bytes.clear());
		app.JSONParseResult = ::llc::jsonFileRead(app.JSONDocument, path);
		if(fileLength && app.JSONDocument.Bytes.size() == 0)
			rtrn app.JSONParseResult;
		app.FileBytes = app.JSONDocument.Bytes;
	}
	else if(fileLength)
		if_fail_fe(::llc::fileToMemory(app.OpenFile, app.FileBytes, DESKTOP_FILE_MAX_BYTES));
	app.Editor.TextSelections.clear();
	app.Editor.SelectionAnchor = app.Editor.SelectionActive = (::llc::u2_t)-1;
	app.Editor.Cursor = {};
	if(app.FileBytes.find('\0') < 0) {
		app.Editor.FilePath = app.OpenFile;
		app.Editor.FileText = app.FileBytes;
		if_fail_fe(::fileViewerTextViewsRebuild(app.Editor));
		app.GUI.Controls.SetHidden(app.Editor.IdKeyboard, false);
	}
	else {
		app.GUI.Controls.SetHidden(app.Editor.IdKeyboard, true);
		app.GUI.Controls.SetHidden(app.Editor.IdCursor, true);
	}
	app.GUI.Controls.SetDisabled(app.ViewportActions[1][::llc::GUI_VIEWPORT_ACTION_SAVE], app.FileBytes.find('\0') >= 0);
	app.GUI.Controls.SetDisabled(app.ViewportActions[1][::llc::GUI_VIEWPORT_ACTION_SELECT_ALL], app.FileBytes.find('\0') >= 0);
	::llc::vcst_t remaining = app.FileBytes;
	if(0 == fileLength)
		if_fail_fe(app.FileLines.push_back(::llc::vcst_t{"Empty file"}));
	if(remaining.find('\0') >= 0) {
		if_fail_fe(::llc::hexEncode(::llc::vcu0_t{(cnst ::llc::u0_t*)remaining.begin(), ::llc::min(remaining.size(), (::llc::u2_t)4096)}, app.FileDisplay));
		remaining = app.FileDisplay;
		if_fail_fe(app.FileLines.push_back(::llc::vcst_t{"Binary file: hex preview"}));
	}
	while(remaining.size()) {
		::llc::vcst_t line = {}, next = {};
		cnst ::llc::err_t found = ::llc::split('\n', remaining, line, next);
		if(line.size() && line[line.size() - 1] == '\r')
			if_fail_fe(line.slice(line, 0, line.size() - 1));
		if_fail_fe(app.FileLines.push_back(line));
		if(found < 0)
			break;
		remaining = next;
	}
	if(fileLength > DESKTOP_FILE_MAX_BYTES)
		if_fail_fe(app.FileLines.push_back(::llc::vcst_t{"Preview limited to first 1 MiB"}));
	app.FileOffset = 0;
	app.FileColumnOffset = 0;
	app.GUI.Controls.Text[app.Viewports[1].IdControls[::llc::VIEWPORT_CONTROL_TITLE]].Text = fileName;
	rtrn ::desktopTextRowsRebuild(app);
}

::llc::err_t sampleGUIViewportSetup(SSampleGUIViewport & app) {
	app.GUI.SelectedFont = 7;
	app.GUI.ColorModeDefault = ::llc::GUI_COLOR_MODE_3D;
	if_fail_fe(::sampleGUIResourcesCreate(app.GUI));
	app.Desktop = ::llc::controlCreate(app.GUI);
	if_true_fe(app.Desktop < 0);
	app.GUI.Controls.Draw[app.Desktop].NoBorder = true;
	app.GUI.Controls.Draw[app.Desktop].NoClient = true;
	app.GUI.Controls.Constraints[app.Desktop].AttachSizeToControl = {app.Desktop, app.Desktop};
	::sampleGUIControlThemeApply(app.GUI, app.Desktop, 2);
	app.Help = ::llc::controlCreateChild(app.GUI, app.Desktop);
	if_true_fe(app.Help < 0);
	app.GUI.Controls.Placement[app.Help].Area = {{18, 18}, {1040, 42}};
	app.GUI.Controls.Placement[app.Help].Border = {2, 2, 2, 2};
	app.GUI.Controls.Placement[app.Help].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[app.Help].Text = "OPEN OR EDIT FILE | W WRAP | A SELECT ALL | S SAVE | SHIFT+WHEEL HORIZONTAL | DRAG TITLES";
	app.GUI.Controls.Text[app.Help].Align = ::llc::ALIGN_CENTER_LEFT;
	app.GUI.Controls.Text[app.Help].FontSelected = 7;
	::sampleGUIControlThemeApply(app.GUI, app.Help, 2);
	if_fail_fe(::sampleGUIViewportCreate(app, 0, "FOLDER", {{24, 92}, {350, 500}}));
	if_fail_fe(::sampleGUIViewportCreate(app, 1, "FILE CONTENT", {{386, 92}, {490, 500}}));
	if_fail_fe(::sampleGUIViewportCreate(app, 2, "MONO CPU SURFACE", {{890, 104}, {360, 230}}));
	if_fail_fe(::desktopListCreate(app, 0, app.FolderControls));
	if_fail_fe(::desktopListCreate(app, 1, app.FileControls));
	app.Editor.Lines.IdControls = app.FileControls;
	app.Editor.Lines.IdRoot = app.Viewports[1].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
	app.Editor.Viewport.IdRoot = app.Viewports[1].IdControl;
	app.Editor.Viewport.IdClient = app.Editor.Lines.IdRoot;
	app.Editor.Viewport.IdTitle = app.Viewports[1].IdControls[::llc::VIEWPORT_CONTROL_TITLE];
	app.Editor.IdCursor = ::llc::controlCreateChild(app.GUI, app.Editor.Lines.IdRoot);
	if_true_fe(app.Editor.IdCursor < 0);
	app.GUI.Controls.Placement[app.Editor.IdCursor].Area.Size = {10, 21};
	app.GUI.Controls.Text[app.Editor.IdCursor].Text = "|";
	app.GUI.Controls.Text[app.Editor.IdCursor].FontSelected = 7;
	::sampleGUIControlThemeApply(app.GUI, app.Editor.IdCursor, 1);
	app.GUI.Controls.SetDisabled(app.Editor.IdCursor, true);
	app.GUI.Controls.SetHidden(app.Editor.IdCursor, true);
	if_fail_fe(::llc::virtualKeyboardSetup437(app.GUI, app.Keyboard));
	::sampleGUIControlThemeApply(app.GUI, app.Keyboard.IdRoot, 2);
	for(::llc::u2_t iCharacter = 0; iCharacter < app.Keyboard.Characters.size(); ++iCharacter)
		::sampleGUIControlThemeApply(app.GUI, app.Keyboard.Characters[iCharacter].IdControl, 2);
	for(::llc::u2_t iCommand = 0; iCommand < app.Keyboard.Commands.size(); ++iCommand)
		::sampleGUIControlThemeApply(app.GUI, app.Keyboard.Commands[iCommand].IdControl, 2);
	app.Editor.IdKeyboard = ::llc::controlCreateChild(app.GUI, app.Viewports[1].IdControl);
	if_true_fe(app.Editor.IdKeyboard < 0);
	app.GUI.Controls.Placement[app.Editor.IdKeyboard].Area = {{18, 18}, {100, 24}};
	app.GUI.Controls.Placement[app.Editor.IdKeyboard].Align = ::llc::ALIGN_BOTTOM_RIGHT;
	app.GUI.Controls.Text[app.Editor.IdKeyboard].Text = "KEYBOARD";
	::sampleGUIControlThemeApply(app.GUI, app.Editor.IdKeyboard, 2);
	app.GUI.Controls.SetHidden(app.Editor.IdKeyboard, true);
	::llc::string initialFolder = {};
	if_fail_fe(::llc::pathAbsolute(".", initialFolder));
	if_fail_fe(::desktopFolderOpen(app, initialFolder));
	if_fail_fe(::sampleGUIViewportActivate(app, 2));
	app.Performance = ::llc::controlCreate(app.GUI);
	if_true_fe(app.Performance < 0);
	app.GUI.Controls.Placement[app.Performance].Area = {{0, 14}, {360, 36}};
	app.GUI.Controls.Placement[app.Performance].Align = ::llc::ALIGN_CENTER_BOTTOM;
	app.GUI.Controls.Placement[app.Performance].Border = {1, 1, 1, 1};
	app.GUI.Controls.Placement[app.Performance].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[app.Performance].Text = "FPS --    Frame -- ms";
	app.GUI.Controls.Text[app.Performance].FontSelected = 7;
	::sampleGUIControlThemeApply(app.GUI, app.Performance, 2);
	app.Exit = ::llc::controlCreate(app.GUI);
	if_true_fe(app.Exit < 0);
	app.GUI.Controls.Placement[app.Exit].Area = {{18, 18}, {96, 36}};
	app.GUI.Controls.Placement[app.Exit].Align = ::llc::ALIGN_BOTTOM_RIGHT;
	app.GUI.Controls.Placement[app.Exit].Border = {2, 2, 2, 2};
	app.GUI.Controls.Placement[app.Exit].Margin = {1, 1, 1, 1};
	app.GUI.Controls.Text[app.Exit].Text = "EXIT";
	app.GUI.Controls.Text[app.Exit].FontSelected = 7;
	::sampleGUIControlThemeApply(app.GUI, app.Exit, 1);
	rtrn ::llc::guiUpdateMetrics(app.GUI, app.Window.Size, false);
}

::llc::err_t sampleGUIViewportUpdate(SSampleGUIViewport & app, bool systemRequestedExit, double secondsLastFrame) {
	if(systemRequestedExit || app.Window.Closed)
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if(app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Escape) && app.GUI.Controls.IsHidden(app.Keyboard.IdRoot) && 0 == app.Editor.TextSelections.size())
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if(app.GUI.Controls.IsAction(app.Exit))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if(app.Editor.CapsSelectionMode) {
		if_fail_fe(::llc::controlTextSet(app.GUI, app.Help, "CAPS SELECT: CLICK START AND END FOR EACH RANGE | CAPS RETURNS TO TYPING | ESC CLEARS"));
	}
	else {
		if_fail_fe(::llc::controlTextSet(app.GUI, app.Help, "OPEN OR EDIT FILE | W WRAP | A SELECT ALL | S SAVE | SHIFT+WHEEL HORIZONTAL | DRAG TITLES"));
	}
	if_fail_fe(::sampleGUIViewportPerformanceUpdate(app, secondsLastFrame));
	bool doubleClick = false;
	if_fail_fe(::sampleGUIViewportDoubleClick(app.Window.EventQueue, doubleClick));
	::llc::u2_t hoveredViewport = app.Viewports.size();
	for(::llc::u0_t iViewport = 0; iViewport < app.Viewports.size(); ++iViewport) {
		::llc::SViewport & viewport = app.Viewports[iViewport];
		cnst ::llc::cid_t idTitle = viewport.IdControls[::llc::VIEWPORT_CONTROL_TITLE];
		cnst ::llc::cid_t idClose = viewport.IdControls[::llc::VIEWPORT_CONTROL_CLOSE];
		cnst ::llc::cid_t idResize = viewport.IdControls[::llc::VIEWPORT_CONTROL_RESIZE_BOTTOM_RIGHT];
		if(::sampleGUIViewportPressed(app, viewport))
			if_fail_fe(::sampleGUIViewportActivate(app, iViewport));
		if(app.GUI.Controls.IsAction(idClose))
			app.GUI.Controls.SetHidden(viewport.IdControl, true);
		if(app.GUI.Controls.IsPressed(idTitle) && (app.Window.Input->MouseCurrent.Deltas.x || app.Window.Input->MouseCurrent.Deltas.y)) {
			cnst ::llc::n2s1_t delta = {(::llc::s1_t)app.Window.Input->MouseCurrent.Deltas.x, (::llc::s1_t)app.Window.Input->MouseCurrent.Deltas.y};
			app.GUI.Controls.Placement[viewport.IdControl].Area.Offset += delta;
			if_fail_fe(::llc::controlMetricsInvalidate(app.GUI, viewport.IdControl));
		}
		if(app.GUI.Controls.IsPressed(idResize) && (app.Window.Input->MouseCurrent.Deltas.x || app.Window.Input->MouseCurrent.Deltas.y)) {
			cnst ::llc::n2s1_t delta = {(::llc::s1_t)app.Window.Input->MouseCurrent.Deltas.x, (::llc::s1_t)app.Window.Input->MouseCurrent.Deltas.y};
			app.GUI.Controls.Placement[viewport.IdControl].Area.Size += delta;
			app.GUI.Controls.Placement[viewport.IdControls[::llc::VIEWPORT_CONTROL_TARGET]].Area.Size += delta;
			viewport.UnfoldedSize = app.GUI.Controls.Placement[viewport.IdControl].Area.Size;
			if_fail_fe(::llc::controlMetricsInvalidate(app.GUI, viewport.IdControl));
		}
		if(app.GUI.Controls.IsHovered(idTitle))
			hoveredViewport = iViewport;
	}
	if(doubleClick && hoveredViewport < app.Viewports.size())
		if_fail_fe(::sampleGUIViewportFold(app, (::llc::u0_t)hoveredViewport));
	cnst ::llc::s2_t wheel = app.Window.Input->MouseCurrent.Deltas.z;
	if(wheel) {
		cnst ::llc::cid_t idFolder = app.Viewports[0].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
		cnst ::llc::cid_t idFile = app.Viewports[1].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
		if(::llc::in_range(app.GUI.CursorPos.s1(), app.GUI.Controls.Area[idFolder].Total.Global)) {
			cnst ::llc::u2_t maxOffset = app.FolderEntries.Name.size() > DESKTOP_VISIBLE_ROWS - 1 ? app.FolderEntries.Name.size() - (DESKTOP_VISIBLE_ROWS - 1) : 0;
			app.FolderOffset = wheel > 0 ? (app.FolderOffset > 5 ? app.FolderOffset - 5 : 0) : ::llc::min(maxOffset, app.FolderOffset + 5);
			if_fail_fe(::desktopFolderRowsRefresh(app));
		}
		else if(::llc::in_range(app.GUI.CursorPos.s1(), app.GUI.Controls.Area[idFile].Total.Global)) {
			if(!app.WordWrap && app.Window.Input->KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Shift]) {
				::llc::u2_t maxWidth = app.Editor.FileLineWidthMax;
				if(app.FileBytes.find('\0') >= 0)
					for(::llc::u2_t i = 0; i < app.FileLines.size(); ++i)
						maxWidth = ::llc::max(maxWidth, app.FileLines[i].size());
				cnst ::llc::u2_t maxColumn = maxWidth > app.VisibleColumns ? maxWidth - app.VisibleColumns : 0;
				app.FileColumnOffset = wheel > 0 ? (app.FileColumnOffset > 8 ? app.FileColumnOffset - 8 : 0) : ::llc::min(maxColumn, app.FileColumnOffset + 8);
			}
			else {
				cnst ::llc::u2_t maxOffset = app.TextRows.size() > DESKTOP_VISIBLE_ROWS ? app.TextRows.size() - DESKTOP_VISIBLE_ROWS : 0;
				app.FileOffset = wheel > 0 ? (app.FileOffset > 15 ? app.FileOffset - 15 : 0) : ::llc::min(maxOffset, app.FileOffset + 15);
			}
			if_fail_fe(::desktopFileRowsRefresh(app));
		}
	}
	for(::llc::u2_t i = 0; i < app.FolderControls.size(); ++i)
		if(app.GUI.Controls.IsAction(app.FolderControls[i])) {
			::llc::string path = {};
			if(0 == i) {
				if_fail_fe(::llc::pathNameCompose(app.FolderPath, "..", path));
				::llc::string parent = {};
				if_fail_fe(::llc::pathAbsolute(path, parent));
				if_fail_fe(::desktopFolderOpen(app, parent));
			}
			else {
				cnst ::llc::u2_t entry = app.FolderOffset + i - 1;
				if_true_fe(entry >= app.FolderEntries.Name.size());
				cnst bool folder = app.FolderEntries.Info[entry].Folder;
				if_fail_fe(::llc::pathNameCompose(app.FolderPath, app.FolderEntries.Name[entry], path));
				if(folder) {
					if_fail_fe(::desktopFolderOpen(app, path));
				}
				else {
					if_fail_fe(::desktopFileOpen(app, path));
					if_fail_fe(::sampleGUIViewportActivate(app, 1));
				}
			}
			break;
		}
	if(app.OpenFile.size() && app.FileBytes.find('\0') < 0) {
		cnst ::llc::n2u2_t cursorBefore = app.Editor.Cursor;
		if(app.GUI.Controls.IsAction(app.Editor.IdKeyboard))
			app.GUI.Controls.SetHidden(app.Keyboard.IdRoot, false == app.GUI.Controls.IsHidden(app.Keyboard.IdRoot));
		if_fail_fe(::desktopEditorSelectionUpdate(app));
		if(app.GUI.Controls.IsSelected(app.Viewports[1].IdControl))
			if_fail_fe(::fileViewerEditorInput(app.GUI, app.Keyboard, app.Editor, app.Window.EventQueue, *app.Window.Input));
		if(app.TextBytesBegin != app.Editor.FileText.begin() || app.TextBytesSize != app.Editor.FileText.size() || app.TextLineCount != app.Editor.FileLines.size())
			if_fail_fe(::desktopTextRowsRebuild(app));
		if(!app.WordWrap && cursorBefore != app.Editor.Cursor) {
			if(app.Editor.Cursor.x < app.FileColumnOffset)
				app.FileColumnOffset = app.Editor.Cursor.x;
			else if(app.Editor.Cursor.x >= app.FileColumnOffset + app.VisibleColumns)
				app.FileColumnOffset = app.Editor.Cursor.x - app.VisibleColumns + 1;
		}
		for(::llc::u2_t iRow = 0; cursorBefore != app.Editor.Cursor && iRow < app.TextRows.size(); ++iRow) {
			cnst ::llc::SGUIViewportTextRow & row = app.TextRows[iRow];
			if(row.SourceLine != app.Editor.Cursor.y || app.Editor.Cursor.x < row.SourceColumn || (app.WordWrap && app.Editor.Cursor.x > row.SourceColumn + row.Text.size()))
				continue;
			if(iRow < app.FileOffset)
				app.FileOffset = iRow;
			else if(iRow >= app.FileOffset + DESKTOP_VISIBLE_ROWS)
				app.FileOffset = iRow - DESKTOP_VISIBLE_ROWS + 1;
			break;
		}
		if_fail_fe(::desktopFileRowsRefresh(app));
	}
	rtrn ::llc::guiUpdateMetrics(app.GUI, app.Window.Size, false);
}

::llc::err_t sampleGUIViewportDraw(SSampleGUIViewport & app, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::u2_t frameIndex, ::llc::g8bgra target) {
	if_fail_fe(target.fill(::llc::SColorBGRA{16, 14, 12}));
	for(::llc::u0_t iViewport = 2; iViewport < SAMPLE_GUI_VIEWPORT_COUNT; ++iViewport) {
		cnst ::llc::cid_t idClient = app.Viewports[iViewport].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
		cnst ::llc::n2u2_t imageSize = app.GUI.Controls.Placement[idClient].Area.Size.u2();
		if(clientImages[iViewport].metrics() != imageSize)
			if_fail_fe(clientImages[iViewport].resize(imageSize));
		::sampleGUIViewportImageUpdate(clientImages[iViewport], iViewport, frameIndex);
		app.GUI.Controls.Images[idClient].Image = clientImages[iViewport].View;
		app.GUI.Controls.Images[idClient].ImageAlign = ::llc::ALIGN_CENTER;
	}
	rtrn ::llc::guiDraw(app.GUI, target);
}

#define LLC_SAMPLE_GUI_VIEWPORT_TITLE "LLC IDE"

stct SApplication {
	::llc::SFramework	Framework;
	SSampleGUIViewport	Sample;
	::llc::string		ScreenshotDirectory = {};
	::llc::img8bgra	ClientImages[SAMPLE_GUI_VIEWPORT_COUNT] = {};
	::llc::u2_t		FrameIndex = 0;

	SApplication(::llc::SRuntimeValues & runtimeValues)
		: Framework(runtimeValues, {}), Sample(Framework.RootWindow, *Framework.GUI) {
		if_fail_e(::llc::rasterFontDefaults(Sample.Engine.Scene->Graphics->Fonts));
	}
};

::llc::err_t setup(SApplication & app) {
	::llc::vcst_t screenshotPath = SCREENSHOT_PATH_DEFAULT;
	::std::error_code configError = {};
	if(::std::filesystem::exists("llc_config.json", configError) && 0 <= ::llc::jsonFileRead(app.Framework.JSONConfig, "llc_config.json")) {
		::llc::vcst_t configuredPath = {};
		if(0 <= ::llc::jsonExpressionResolve("screenshot_path", app.Framework.JSONConfig.Reader, 0, configuredPath) && configuredPath.size())
			screenshotPath = configuredPath;
	}
	if(::llc::failed(::llc::pathNormalize(screenshotPath, app.ScreenshotDirectory)) || ::llc::failed(::llc::pathCreate(app.ScreenshotDirectory)))
		app.ScreenshotDirectory = SCREENSHOT_PATH_DEFAULT;
	::llc::SWindow & window = app.Framework.RootWindow;
	window.Size = {1280, 720};
	if_null_ve(-1, window.BackBuffer.create());
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	if_fail_fe(::llc::mainWindowCreate(window, app.Framework.RuntimeValues.PlatformDetail, window.Input));
	rtrn ::sampleGUIViewportSetup(app.Sample);
}

::llc::err_t cleanup(SApplication & app) {
	rtrn ::llc::mainWindowDestroy(app.Framework.RootWindow);
}

::llc::err_t update(SApplication & app, bool systemRequestedExit) {
	if(systemRequestedExit)
		rtrn ::llc::APPLICATION_STATE_EXIT;
	cnst ::llc::err_t result = ::llc::updateFramework(app.Framework);
	if(result)
		rtrn result;
	cnst ::llc::err_t resultSample = ::sampleGUIViewportUpdate(app.Sample, false, app.Framework.FrameInfo.Seconds.LastFrame);
	if(resultSample)
		rtrn resultSample;
	::llc::GUI_VIEWPORT_ACTION action = ::llc::GUI_VIEWPORT_ACTION_NONE;
	bool resized = false;
	cnst ::llc::cid_t idFile = app.Sample.Viewports[1].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
	if_fail_fe(::llc::guiViewportEvents(app.Sample.GUI, app.Sample.ViewportActions[1], idFile, action, resized));
	if(::llc::GUI_VIEWPORT_ACTION_WRAP == action) {
		app.Sample.WordWrap = !app.Sample.WordWrap;
		app.Sample.FileColumnOffset = 0;
		app.Sample.GUI.Controls.SetSelected(app.Sample.ViewportActions[1][::llc::GUI_VIEWPORT_ACTION_WRAP], app.Sample.WordWrap);
	}
	if(app.Sample.OpenFile.size() && app.Sample.FileBytes.find('\0') < 0) {
		if(::llc::GUI_VIEWPORT_ACTION_SAVE == action)
			if_fail_fe(::llc::fileFromMemory(app.Sample.Editor.FilePath, app.Sample.Editor.FileText.cc()));
		if(::llc::GUI_VIEWPORT_ACTION_SELECT_ALL == action) {
			if_fail_fe(app.Sample.Editor.TextSelections.clear());
			if_fail_fe(app.Sample.Editor.TextSelections.push_back({0, app.Sample.Editor.FileText.size()}));
			app.Sample.Editor.SelectionAnchor = (::llc::u2_t)-1;
			app.Sample.Editor.SelectionActive = (::llc::u2_t)-1;
		}
	}
	if(resized || ::llc::GUI_VIEWPORT_ACTION_WRAP == action || app.Sample.VisibleColumns != ::llc::guiViewportTextColumns(app.Sample.GUI, app.Sample.FileControls[0])) {
		if_fail_fe(::desktopTextRowsRebuild(app.Sample));
	}
	else if(::llc::GUI_VIEWPORT_ACTION_SELECT_ALL == action) {
		if_fail_fe(::desktopFileRowsRefresh(app.Sample));
	}
	if_fail_fe(::llc::windowScreenshotHotkey(app.Framework.RootWindow, app.Framework.RuntimeValues.EntryPointArgs.ProgramName, app.ScreenshotDirectory));
	::llc::sleep(1);
	rtrn resultSample;
}

::llc::err_t draw(SApplication & app) {
	::llc::SWindow & window = app.Framework.RootWindow;
	if(window.NoDraw)
		rtrn 0;
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	rtrn ::sampleGUIViewportDraw(app.Sample, app.ClientImages, app.FrameIndex++, window.BackBuffer->Color.View);
}

LLC_DEFINE_APPLICATION_ENTRY_POINT(SApplication, LLC_SAMPLE_GUI_VIEWPORT_TITLE)
