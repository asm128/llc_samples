#include "llc_blitter_dashboard.h"

#include "llc_apod_serialize.h"
#include "llc_app_impl.h"
#include "llc_encoding.h"
#include "llc_file.h"
#include "llc_cstring.h"
#include "llc_framework.h"
#include "llc_gui_control_list.h"
#include "llc_runtime.h"
#include "llc_system_key.h"
#include "llc_string_compose.h"

stxp ::llc::u2_t DESKTOP_VISIBLE_ROWS = 20;
stxp ::llc::u2_t DESKTOP_FILE_MAX_BYTES = 4 * 1024 * 1024; // Below the 100 MiB read limit.

stacxpr ::llc::u0_t SAMPLE_GUI_THEME_COUNT = 3;
stacxpr ::llc::u0_t PALETTE_SHADE_COUNT = 16;

sttc ::llc::SColorBGRA sampleGUIThemeColor(::llc::u0_t theme) nxpt {
	stacxpr ::llc::SColorBGRA colors[SAMPLE_GUI_THEME_COUNT] =
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
	stacxpr ::llc::u0_t shades[::llc::GUI_CONTROL_PALETTE_COUNT] = {8, 10, 5, 11, 4, 7, 2, 7, 1, 13};
	rtrn shades[state % ::llc::GUI_CONTROL_PALETTE_COUNT];
}

sttc ::llc::u0_t sampleGUIStateBorderOffset(::llc::u0_t state) nxpt {
	stacxpr ::llc::u0_t offsets[::llc::GUI_CONTROL_PALETTE_COUNT] = {5, 1, 7, 3, 0, 1, 7, 3, 5, 5};
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

sttc ::llc::err_t blitterDashboardDoubleClick(cnst ::llc::TQueueSystemEvent & events, bool & doubleClick) {
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

sttc bool blitterDashboardPressed(cnst SBlitterDashboard & app, cnst ::llc::SViewport & viewport) nxpt {
	rtrn app.GUI.Controls.IsPressed(viewport.IdControl)
		|| app.GUI.Controls.IsPressed(viewport.IdControls[::llc::VIEWPORT_CONTROL_TITLE])
		|| app.GUI.Controls.IsPressed(viewport.IdControls[::llc::VIEWPORT_CONTROL_TARGET])
		;
}

sttc ::llc::err_t blitterDashboardActivate(SBlitterDashboard & app, ::llc::u0_t iViewport) {
	if_true_fe(iViewport >= app.Viewports.size());
	cnst ::llc::cid_t idRoot = app.Viewports[iViewport].IdControl;
	::llc::acid & desktopChildren = app.GUI.Controls.Children[app.Desktop];
	if(0 == desktopChildren.size() || desktopChildren[desktopChildren.size() - 1] != idRoot) {
		cnst ::llc::err_t iChild = desktopChildren.find(idRoot);
		if(iChild >= 0)
			if_fail_fe(desktopChildren.remove((::llc::u2_t)iChild));
		if_fail_fe(desktopChildren.push_back(idRoot));
	}
	for(::llc::u0_t iOther = 0; iOther < app.Viewports.size(); ++iOther) {
		cnst bool active = iOther == iViewport;
		app.GUI.Controls.SetSelected(app.Viewports[iOther].IdControl, active);
		app.GUI.Controls.SetSelected(app.Viewports[iOther].IdControls[::llc::VIEWPORT_CONTROL_TITLE], active);
	}
	rtrn 0;
}

sttc void blitterDashboardImageUpdate(::llc::img8bgra & image, ::llc::u0_t theme, ::llc::u2_t frameIndex) nxpt {
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

sttc ::llc::err_t blitterDashboardCreate(SBlitterDashboard & app, ::llc::u0_t iViewport, ::llc::vcst_t title, cnst ::llc::rect2s1_t & area) {
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
	app.UnfoldedSize[iViewport] = area.Size;
	rtrn ::llc::controlMetricsInvalidate(app.GUI, viewport.IdControl);
}

sttc ::llc::err_t blitterDashboardFold(SBlitterDashboard & app, ::llc::u0_t iViewport) {
	if_true_fe(iViewport >= app.Viewports.size());
	::llc::SViewport & viewport = app.Viewports[iViewport];
	::llc::SControlPlacement & root = app.GUI.Controls.Placement[viewport.IdControl];
	cnst ::llc::cid_t idClient = viewport.IdControls[::llc::VIEWPORT_CONTROL_TARGET];
	if(app.Folded[iViewport]) {
		root.Area.Size = app.UnfoldedSize[iViewport];
		app.GUI.Controls.SetHidden(idClient, false);
	}
	else {
		app.UnfoldedSize[iViewport] = root.Area.Size;
		root.Area.Size.y = 30;
		app.GUI.Controls.SetHidden(idClient, true);
	}
	app.Folded[iViewport] = false == app.Folded[iViewport];
	rtrn ::llc::controlMetricsInvalidate(app.GUI, viewport.IdControl);
}

sttc ::llc::err_t blitterDashboardPerformanceUpdate(SBlitterDashboard & app, double secondsLastFrame) {
	if(secondsLastFrame <= 0)
		rtrn 0;
	cnst ::llc::err_t textLength = ::llc::sprintf_s(app.PerformanceText.Storage, "FPS %.2f    Frame %.2f ms", 1.0 / secondsLastFrame, secondsLastFrame * 1000.0);
	if_fail_fe(textLength);
	app.GUI.Controls.Text[app.Performance].Text = {app.PerformanceText.Storage, (::llc::u2_t)textLength};
	app.GUI.Controls.SetUpdated(app.Performance, false);
	rtrn 0;
}

sttc ::llc::err_t desktopListCreate(SBlitterDashboard & app, ::llc::u0_t iViewport, ::llc::acid & controls) {
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

sttc ::llc::err_t desktopFolderRowsRefresh(SBlitterDashboard & app) {
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

sttc ::llc::err_t desktopFileRowsRefresh(SBlitterDashboard & app) {
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
			app.GUI.Controls.SetSelected(id, app.SelectAll);
		}
	}
	rtrn 0;
}

sttc ::llc::err_t desktopTextRowsRebuild(SBlitterDashboard & app) {
	app.VisibleColumns = ::llc::guiViewportTextColumns(app.GUI, app.FileControls[0]);
	app.FileLineWidthMax = 0;
	for(::llc::u2_t i = 0; i < app.FileLines.size(); ++i)
		app.FileLineWidthMax = ::llc::max(app.FileLineWidthMax, app.FileLines[i].size());
	if_fail_fe(::llc::guiViewportTextRowsBuild(app.FileLines, app.VisibleColumns, app.WordWrap, app.TextRows));
	app.FileOffset = ::llc::min(app.FileOffset, app.TextRows.size() > DESKTOP_VISIBLE_ROWS ? app.TextRows.size() - DESKTOP_VISIBLE_ROWS : 0);
	rtrn ::desktopFileRowsRefresh(app);
}

sttc ::llc::err_t jsonEntryAdd(SBlitterDashboard & app, ::llc::u2_t node, ::llc::s2_t parent, ::llc::vcst_t name) {
	::llc::pobj<SBlitterDashboard::SJSONEntry> entry = {};
	if_true_fe(0 == entry.create());
	entry->Node = node;
	entry->Parent = parent;
	cnst ::llc::JSON_TYPE type = app.JSONReader.Token[node].Type;
	if_fail_fe(::llc::append_strings(entry->Label, name, ": "));
	if(type == ::llc::JSON_TYPE_OBJECT || type == ::llc::JSON_TYPE_ARRAY) {
		if_fail_fe(::llc::append_strings(entry->Label, type == ::llc::JSON_TYPE_OBJECT ? ::llc::vcst_t{"{...}"} : ::llc::vcst_t{"[...]"}));
	}
	else {
		::llc::string value = {};
		if_fail_fe(::llc::jsonWrite(app.JSONReader.Tree[node], app.JSONReader.View, value));
		if_fail_fe(entry->Label.append({value.begin(), ::llc::min(value.size(), (::llc::u2_t)42)}));
	}
	cnst ::llc::cid_t idClient = app.Viewports[3].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
	if_fail_fe(entry->Root = ::llc::controlCreateChild(app.GUI, idClient));
	app.GUI.Controls.Placement[entry->Root].Area.Size = {330, 23};
	::sampleGUIControlThemeApply(app.GUI, entry->Root, 2);
	::llc::SControlPlacement row = {};
	row.Area.Size = {330, 22};
	row.Border = {1, 1, 1, 1};
	row.Margin = {2, 1, 2, 1};
	::llc::vcst_t label = entry->Label;
	::llc::astcid<SBlitterDashboard::JSON_ENTRY_CHILD_Count> children = {};
	if_fail_fe(::llc::guiCreateControlList(app.GUI, ::llc::DIRECTION_DOWN, {&label, 1}, entry->Root, row, ::llc::ALIGN_CENTER_LEFT, children));
	::sampleGUIControlThemeApply(app.GUI, app.GUI.Controls.Children[entry->Root][SBlitterDashboard::JSON_ENTRY_CHILD_Row], 0);
	if_fail_fe(app.JSONEntries.push_back(entry));
	rtrn 0;
}

sttc ::llc::err_t jsonChildrenBuild(SBlitterDashboard & app, ::llc::s2_t parent) {
	SBlitterDashboard::SJSONEntry & entry = *app.JSONEntries[parent];
	if(entry.ChildrenBuilt)
		rtrn 0;
	entry.ChildrenBuilt = true;
	if(app.JSONReader.Token[entry.Node].Type == ::llc::JSON_TYPE_OBJECT) {
		::llc::aobj<::llc::vcst_t> keys = {};
		if_fail_fe(::llc::jsonObjectKeyList(app.JSONReader, entry.Node, keys));
		for(::llc::u2_t i = 0; i < keys.size(); ++i) {
			cnst ::llc::err_t child = ::llc::jsonObjectValueGet(app.JSONReader, entry.Node, keys[i]);
			if_fail_fe(child);
			if_fail_fe(::jsonEntryAdd(app, child, parent, keys[i]));
		}
	}
	else if(app.JSONReader.Token[entry.Node].Type == ::llc::JSON_TYPE_ARRAY) {
		cnst ::llc::err_t count = ::llc::jsonArraySize(app.JSONReader, entry.Node);
		if_fail_fe(count);
		for(::llc::u2_t i = 0; i < (::llc::u2_t)count; ++i) {
			cnst ::llc::err_t child = ::llc::jsonArrayValueGet(app.JSONReader, entry.Node, i);
			if_fail_fe(child);
			cnst auto indexText = ::llc::str(i);
			if_fail_fe(::jsonEntryAdd(app, child, parent, ::llc::vcst_t{indexText.Storage}));
		}
	}
	rtrn 0;
}

sttc ::llc::err_t jsonLayout(SBlitterDashboard & app, ::llc::s2_t parent, ::llc::u2_t depth, ::llc::s2_t & y, bool visible) {
	for(::llc::u2_t i = 0; i < app.JSONEntries.size(); ++i) {
		SBlitterDashboard::SJSONEntry & entry = *app.JSONEntries[i];
		if(!entry.Active || entry.Parent != parent)
			continue;
		app.GUI.Controls.SetHidden(entry.Root, !visible);
		if(visible) {
			app.GUI.Controls.Placement[entry.Root].Area.Offset = {(::llc::s1_t)(8 + depth * 14), (::llc::s1_t)y};
			app.GUI.Controls.Placement[entry.Root].Area.Size.x = (::llc::s1_t)(330 - depth * 14);
			y += 23;
		}
		if_fail_fe(::jsonLayout(app, i, depth + 1, y, visible && entry.Expanded));
	}
	rtrn 0;
}

sttc ::llc::err_t jsonFileShow(SBlitterDashboard & app) {
	for(::llc::u2_t i = 0; i < app.JSONEntries.size(); ++i) {
		app.JSONEntries[i]->Active = false;
		app.GUI.Controls.SetHidden(app.JSONEntries[i]->Root, true);
	}
	app.GUI.Controls.Text[app.Viewports[3].IdControls[::llc::VIEWPORT_CONTROL_TITLE]].Text = "BLITTER JSON";
	if(0 == app.FileBytes.size() || 0 > ::llc::jsonParse(app.JSONReader, app.FileBytes, ::llc::JSON_PARSE_STRICT))
		rtrn 0;
	cnst ::llc::s2_t root = app.JSONEntries.size();
	if_fail_fe(::jsonEntryAdd(app, 0, -1, "JSON"));
	if_fail_fe(::jsonChildrenBuild(app, root));
	app.JSONEntries[root]->Expanded = true;
	::llc::s2_t y = 6;
	rtrn ::jsonLayout(app, -1, 0, y, true);
}

sttc ::llc::err_t desktopFolderOpen(SBlitterDashboard & app, ::llc::vcst_t path) {
	for(::llc::u2_t i = 0; i < app.JSONEntries.size(); ++i) {
		app.JSONEntries[i]->Active = false;
		app.GUI.Controls.SetHidden(app.JSONEntries[i]->Root, true);
	}
	app.FolderPath = path;
	if_fail_fe(::llc::pathList(app.FolderEntries, app.FolderPath, [] (bool folder, ::llc::vcst_c &) -> ::llc::err_t { rtrn folder ? 2 : 0; }));
	app.FolderOffset = 0;
	if_fail_fe(::desktopFolderRowsRefresh(app));
	::llc::vcst_t folderName = {};
	if_fail_fe(::llc::pathFilename(app.FolderPath, folderName));
	app.GUI.Controls.Text[app.Viewports[0].IdControls[::llc::VIEWPORT_CONTROL_TITLE]].Text = folderName.size() ? folderName : ::llc::vcst_t{"ROOT"};
	for(::llc::u2_t i = 0; i < app.FileControls.size(); ++i)
		if_fail_fe(::llc::controlTextSet(app.GUI, app.FileControls[i], {}));
	if_fail_fe(app.TextRows.clear());
	if_fail_fe(app.FileLines.clear());
	app.JSONReader = {};
	if_fail_fe(app.FileBytes.clear());
	if_fail_fe(app.FileDisplay.clear());
	if_fail_fe(app.OpenFile.clear());
	app.FileLength = 0;
	app.FileWindowStart = 0;
	app.FileOffset = 0;
	app.FileColumnOffset = 0;
	app.SelectAll = false;
	app.GUI.Controls.SetDisabled(app.ViewportActions[1][::llc::GUI_VIEWPORT_ACTION_SAVE], true);
	app.GUI.Controls.SetDisabled(app.ViewportActions[1][::llc::GUI_VIEWPORT_ACTION_SELECT_ALL], true);
	app.GUI.Controls.Text[app.Viewports[1].IdControls[::llc::VIEWPORT_CONTROL_TITLE]].Text = "FILE CONTENT";
	rtrn ::desktopFileRowsRefresh(app);
}

sttc ::llc::s3_t desktopRowByte(cnst SBlitterDashboard & app, ::llc::u2_t row) {
	if(row >= app.TextRows.size() || 0 == app.FileBytes.size())
		rtrn app.FileWindowStart;
	cnst char * begin = app.TextRows[row].Text.begin();
	if(begin < app.FileBytes.begin() || begin >= app.FileBytes.begin() + app.FileBytes.size())
		rtrn app.FileWindowStart;
	rtrn app.FileWindowStart + (begin - app.FileBytes.begin());
}

sttc ::llc::err_t desktopFileWindowLoad(SBlitterDashboard & app, ::llc::s3_t cursor) {
	cnst ::llc::s3_t window = DESKTOP_FILE_MAX_BYTES;
	cnst ::llc::s3_t maxStart = app.FileLength > window ? app.FileLength - window : 0;
	app.FileWindowStart = ::llc::min(maxStart, ::llc::max((::llc::s3_t)0, cursor - window / 2));
	for(::llc::u2_t i = 0; i < app.JSONEntries.size(); ++i) {
		app.JSONEntries[i]->Active = false;
		app.GUI.Controls.SetHidden(app.JSONEntries[i]->Root, true);
	}
	for(::llc::u2_t i = 0; i < app.FileControls.size(); ++i)
		if_fail_fe(::llc::controlTextSet(app.GUI, app.FileControls[i], {}));
	if_fail_fe(app.TextRows.clear());
	if_fail_fe(app.FileLines.clear());
	app.JSONReader = {};
	if_fail_fe(app.FileBytes.clear());
	if_fail_fe(app.FileDisplay.clear());
	if(app.FileLength)
		if_fail_fe(::llc::fileToMemory(app.OpenFile, app.FileBytes, DESKTOP_FILE_MAX_BYTES, app.FileWindowStart));
	::llc::vcst_t remaining = app.FileBytes;
	if(0 == app.FileLength)
		if_fail_fe(app.FileLines.push_back(::llc::vcst_t{"Empty file"}));
	if(app.FileWindowStart && remaining.size()) {
		::llc::vcst_t partial = {}, next = {};
		if(0 <= ::llc::split('\n', remaining, partial, next))
			remaining = next;
	}
	if(app.FileWindowStart + app.FileBytes.size() < app.FileLength && remaining.size()) {
		cnst ::llc::err_t lastNewline = remaining.rfind('\n');
		if(lastNewline >= 0) {
			::llc::vcst_t prefix = {};
			if_fail_fe(remaining.slice(prefix, 0, (::llc::u2_t)lastNewline + 1));
			remaining = prefix;
		}
	}
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
	app.FileOffset = 0;
	app.FileColumnOffset = 0;
	app.SelectAll = false;
	app.GUI.Controls.SetDisabled(app.ViewportActions[1][::llc::GUI_VIEWPORT_ACTION_SAVE], true);
	app.GUI.Controls.SetDisabled(app.ViewportActions[1][::llc::GUI_VIEWPORT_ACTION_SELECT_ALL], false);
	::llc::vcst_t fileName = {};
	if_fail_fe(::llc::pathFilename(app.OpenFile, fileName));
	app.GUI.Controls.Text[app.Viewports[1].IdControls[::llc::VIEWPORT_CONTROL_TITLE]].Text = fileName;
	if_fail_fe(::desktopTextRowsRebuild(app));
	app.FileOffset = app.TextRows.size() > DESKTOP_VISIBLE_ROWS ? app.TextRows.size() - DESKTOP_VISIBLE_ROWS : 0;
	for(::llc::u2_t i = 0; i < app.TextRows.size(); ++i)
		if(::desktopRowByte(app, i) >= cursor) {
			app.FileOffset = ::llc::min(i, app.TextRows.size() > DESKTOP_VISIBLE_ROWS ? app.TextRows.size() - DESKTOP_VISIBLE_ROWS : 0);
			break;
		}
	if_fail_fe(::desktopFileRowsRefresh(app));
	rtrn app.FileLength <= DESKTOP_FILE_MAX_BYTES ? ::jsonFileShow(app) : 0;
}

sttc ::llc::err_t desktopFileOpen(SBlitterDashboard & app, ::llc::vcst_t path) {
	app.OpenFile = path;
	app.FileLength = ::llc::fileSize(app.OpenFile);
	if_true_fe(app.FileLength < 0);
	rtrn ::desktopFileWindowLoad(app, 0);
}

sttc ::llc::err_t desktopFileWindowCheck(SBlitterDashboard & app) {
	if(app.FileLength <= DESKTOP_FILE_MAX_BYTES || 0 == app.TextRows.size())
		rtrn 0;
	cnst ::llc::u2_t margin = 2 * DESKTOP_VISIBLE_ROWS;
	cnst bool nearStart = app.FileOffset < margin && app.FileWindowStart > 0;
	cnst bool nearEnd = app.FileOffset + margin + DESKTOP_VISIBLE_ROWS >= app.TextRows.size()
		&& app.FileWindowStart + app.FileBytes.size() < app.FileLength;
	if(nearStart || nearEnd)
		rtrn ::desktopFileWindowLoad(app, ::desktopRowByte(app, app.FileOffset));
	rtrn 0;
}

::llc::err_t blitterDashboardSetup(SBlitterDashboard & app) {
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
	app.GUI.Controls.Text[app.Help].Text = "OPEN FILE | CLICK JSON ROWS TO EXPAND | W WRAP | WHEEL/PAGE UP/DOWN | DRAG TITLES";
	app.GUI.Controls.Text[app.Help].Align = ::llc::ALIGN_CENTER_LEFT;
	app.GUI.Controls.Text[app.Help].FontSelected = 7;
	::sampleGUIControlThemeApply(app.GUI, app.Help, 2);
	if_fail_fe(::blitterDashboardCreate(app, 0, "FOLDER", {{24, 92}, {350, 500}}));
	if_fail_fe(::blitterDashboardCreate(app, 1, "FILE CONTENT", {{386, 92}, {490, 500}}));
	if_fail_fe(::blitterDashboardCreate(app, 2, "MONO CPU SURFACE", {{890, 104}, {360, 230}}));
	if_fail_fe(::blitterDashboardCreate(app, 3, "BLITTER JSON", {{890, 346}, {360, 246}}));
	if_fail_fe(::desktopListCreate(app, 0, app.FolderControls));
	if_fail_fe(::desktopListCreate(app, 1, app.FileControls));
	::llc::string initialFolder = {};
	if_fail_fe(::llc::pathAbsolute(".", initialFolder));
	if_fail_fe(::desktopFolderOpen(app, initialFolder));
	if(0 <= ::llc::fileSize("./blitter.json"))
		if_fail_fe(::desktopFileOpen(app, "./blitter.json"));
	if_fail_fe(::blitterDashboardActivate(app, 2));
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

::llc::err_t blitterDashboardUpdate(SBlitterDashboard & app, bool systemRequestedExit, double secondsLastFrame) {
	if(systemRequestedExit || app.Window.Closed || app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Escape))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if(app.GUI.Controls.IsAction(app.Exit))
		rtrn ::llc::APPLICATION_STATE_EXIT;
	if_fail_fe(::blitterDashboardPerformanceUpdate(app, secondsLastFrame));
	bool doubleClick = false;
	if_fail_fe(::blitterDashboardDoubleClick(app.Window.EventQueue, doubleClick));
	::llc::u2_t hoveredViewport = app.Viewports.size();
	for(::llc::u0_t iViewport = 0; iViewport < app.Viewports.size(); ++iViewport) {
		::llc::SViewport & viewport = app.Viewports[iViewport];
		cnst ::llc::cid_t idTitle = viewport.IdControls[::llc::VIEWPORT_CONTROL_TITLE];
		cnst ::llc::cid_t idClose = viewport.IdControls[::llc::VIEWPORT_CONTROL_CLOSE];
		cnst ::llc::cid_t idResize = viewport.IdControls[::llc::VIEWPORT_CONTROL_RESIZE_BOTTOM_RIGHT];
		if(::blitterDashboardPressed(app, viewport))
			if_fail_fe(::blitterDashboardActivate(app, iViewport));
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
			app.UnfoldedSize[iViewport] = app.GUI.Controls.Placement[viewport.IdControl].Area.Size;
			if_fail_fe(::llc::controlMetricsInvalidate(app.GUI, viewport.IdControl));
		}
		if(app.GUI.Controls.IsHovered(idTitle))
			hoveredViewport = iViewport;
	}
	if(doubleClick && hoveredViewport < app.Viewports.size())
		if_fail_fe(::blitterDashboardFold(app, (::llc::u0_t)hoveredViewport));
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
			if(app.Window.Input->KeyboardCurrent.KeyState[::llc::SYSTEM_KEY_Shift]) {
				cnst ::llc::u2_t maxColumn = app.FileLineWidthMax > app.VisibleColumns ? app.FileLineWidthMax - app.VisibleColumns : 0;
				app.FileColumnOffset = wheel > 0 ? (app.FileColumnOffset > 8 ? app.FileColumnOffset - 8 : 0) : ::llc::min(maxColumn, app.FileColumnOffset + 8);
			}
			else {
				cnst ::llc::u2_t maxOffset = app.TextRows.size() > DESKTOP_VISIBLE_ROWS ? app.TextRows.size() - DESKTOP_VISIBLE_ROWS : 0;
				app.FileOffset = wheel > 0 ? (app.FileOffset > 15 ? app.FileOffset - 15 : 0) : ::llc::min(maxOffset, app.FileOffset + 15);
			}
			if_fail_fe(::desktopFileRowsRefresh(app));
			if_fail_fe(::desktopFileWindowCheck(app));
		}
	}
	cnst ::llc::cid_t idFileTarget = app.Viewports[1].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
	if(::llc::in_range(app.GUI.CursorPos.s1(), app.GUI.Controls.Area[idFileTarget].Total.Global) && app.TextRows.size()) {
		cnst bool pageUp = app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Prior);
		cnst bool pageDown = app.Window.Input->KeyDown(::llc::SYSTEM_KEY_Next);
		if(pageUp || pageDown) {
			cnst ::llc::u2_t maxOffset = app.TextRows.size() > DESKTOP_VISIBLE_ROWS ? app.TextRows.size() - DESKTOP_VISIBLE_ROWS : 0;
			app.FileOffset = pageUp ? (app.FileOffset > DESKTOP_VISIBLE_ROWS ? app.FileOffset - DESKTOP_VISIBLE_ROWS : 0)
				: ::llc::min(maxOffset, app.FileOffset + DESKTOP_VISIBLE_ROWS);
			if_fail_fe(::desktopFileRowsRefresh(app));
			if_fail_fe(::desktopFileWindowCheck(app));
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
					if_fail_fe(::blitterDashboardActivate(app, 1));
				}
			}
			break;
		}
	::llc::cid_t pressed = ::llc::CID_INVALID;
	for(::llc::u2_t i = 0; i < app.JSONEntries.size(); ++i)
		if(app.JSONEntries[i]->Active && app.GUI.Controls.IsPressed(app.GUI.Controls.Children[app.JSONEntries[i]->Root][SBlitterDashboard::JSON_ENTRY_CHILD_Row])) {
			pressed = app.GUI.Controls.Children[app.JSONEntries[i]->Root][SBlitterDashboard::JSON_ENTRY_CHILD_Row];
			if(pressed == app.JSONPressed)
				break;
			if_fail_fe(::jsonChildrenBuild(app, i));
			app.JSONEntries[i]->Expanded = !app.JSONEntries[i]->Expanded;
			::llc::s2_t y = 6;
			if_fail_fe(::jsonLayout(app, -1, 0, y, true));
			break;
		}
	app.JSONPressed = pressed;
	rtrn ::llc::guiUpdateMetrics(app.GUI, app.Window.Size, false);
}

::llc::err_t blitterDashboardDraw(SBlitterDashboard & app, ::llc::img8bgra (&clientImages)[SAMPLE_GUI_VIEWPORT_COUNT], ::llc::u2_t frameIndex, ::llc::g8bgra target) {
	if_fail_fe(target.fill(::llc::SColorBGRA{16, 14, 12}));
	for(::llc::u0_t iViewport = 2; iViewport < 3; ++iViewport) {
		cnst ::llc::cid_t idClient = app.Viewports[iViewport].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
		cnst ::llc::n2u2_t imageSize = app.GUI.Controls.Placement[idClient].Area.Size.u2();
		if(clientImages[iViewport].metrics() != imageSize)
			if_fail_fe(clientImages[iViewport].resize(imageSize));
		::blitterDashboardImageUpdate(clientImages[iViewport], iViewport, frameIndex);
		app.GUI.Controls.Images[idClient].Image = clientImages[iViewport].View;
		app.GUI.Controls.Images[idClient].ImageAlign = ::llc::ALIGN_CENTER;
	}
	rtrn ::llc::guiDraw(app.GUI, target);
}

#define LLC_SAMPLE_GUI_VIEWPORT_TITLE "LLC desktop"

stct SApplication {
	::llc::SFramework	Framework;
	SBlitterDashboard	Sample;
	::llc::img8bgra	ClientImages[SAMPLE_GUI_VIEWPORT_COUNT] = {};
	::llc::u2_t		FrameIndex = 0;

	SApplication(::llc::SRuntimeValues & runtimeValues)
		: Framework(runtimeValues, {}), Sample(Framework.RootWindow, *Framework.GUI) {
		if_fail_e(::llc::rasterFontDefaults(Sample.Engine.Scene->Graphics->Fonts));
	}
};

::llc::err_t setup(SApplication & app) {
	::llc::SWindow & window = app.Framework.RootWindow;
	window.Size = {1280, 720};
	if_null_ve(::llc::OS_NO_MEMORY, window.BackBuffer.create());
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	if_fail_fe(::llc::mainWindowCreate(window, app.Framework.RuntimeValues.PlatformDetail, window.Input));
	rtrn ::blitterDashboardSetup(app.Sample);
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
	cnst ::llc::err_t resultSample = ::blitterDashboardUpdate(app.Sample, false, app.Framework.FrameInfo.Seconds.LastFrame);
	if(resultSample)
		rtrn resultSample;
	::llc::GUI_VIEWPORT_ACTION action = ::llc::GUI_VIEWPORT_ACTION_NONE;
	bool resized = false;
	cnst ::llc::cid_t idFile = app.Sample.Viewports[1].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
	if_fail_fe(::llc::guiViewportEvents(app.Sample.GUI, app.Sample.ViewportActions[1], idFile, action, resized));
	if(::llc::GUI_VIEWPORT_ACTION_WRAP == action) {
		app.Sample.WordWrap = !app.Sample.WordWrap;
		app.Sample.GUI.Controls.SetSelected(app.Sample.ViewportActions[1][::llc::GUI_VIEWPORT_ACTION_WRAP], app.Sample.WordWrap);
	}
	if(::llc::GUI_VIEWPORT_ACTION_SELECT_ALL == action)
		app.Sample.SelectAll = true;
	if(resized || ::llc::GUI_VIEWPORT_ACTION_WRAP == action || ::llc::GUI_VIEWPORT_ACTION_SELECT_ALL == action || app.Sample.VisibleColumns != ::llc::guiViewportTextColumns(app.Sample.GUI, app.Sample.FileControls[0]))
		if_fail_fe(::desktopTextRowsRebuild(app.Sample));
	::llc::sleep(1);
	rtrn resultSample;
}

::llc::err_t draw(SApplication & app) {
	::llc::SWindow & window = app.Framework.RootWindow;
	if(window.NoDraw)
		rtrn 0;
	if_fail_fe(window.BackBuffer->resize(window.Size.u2()));
	rtrn ::blitterDashboardDraw(app.Sample, app.ClientImages, app.FrameIndex++, window.BackBuffer->Color.View);
}

LLC_DEFINE_APPLICATION_ENTRY_POINT(SApplication, LLC_SAMPLE_GUI_VIEWPORT_TITLE)
