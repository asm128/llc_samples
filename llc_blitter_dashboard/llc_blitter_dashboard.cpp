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

sttc void sampleGUIControlThemeApply(::llc::SGUI & gui, ::llc::cid_t idControl, ::llc::u0_t theme) nxpt {
	if(::llc::controlInvalid(gui, idControl))
		rtrn;
	stxp int16_t themes[] = {::llc::GUI_THEME_DARK_BLUE, ::llc::GUI_THEME_DARK_SLATE, ::llc::GUI_THEME_DARK_CHROME};
	gui.Controls.Draw[idControl].ColorTheme = themes[theme % SAMPLE_GUI_THEME_COUNT];
}

sttc ::llc::err_t blitterDashboardActivate(SBlitterDashboard & app, ::llc::u0_t iViewport) {
	if_true_fe(iViewport >= app.Viewports.size());
	cnst ::llc::cid_t idRoot = app.Viewports[iViewport].IdControl;
	::llc::acid & desktopChildren = app.GUI.Controls.Children[app.Desktop];
	if(0 == desktopChildren.size() || desktopChildren[desktopChildren.size() - 1] != idRoot)
		if_fail_fe(::llc::controlToFront(app.GUI, idRoot));
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

sttc ::llc::s3_t desktopRowByte(cnst SBlitterDashboard & app, ::llc::u2_t row);

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
			app.GUI.Controls.SetSelected(id, false);
			::llc::apod<::llc::STextState> & states = app.GUI.Controls.TextState[id];
			if_fail_fe(states.resize(text.size(), ::llc::STextState{}));
			cnst ::llc::s3_t rowByte = ::desktopRowByte(app, line) + (app.WordWrap ? 0 : app.FileColumnOffset);
			cnst ::llc::s3_t selectedBegin = ::llc::min(app.SelectionAnchor, app.SelectionCursor);
			cnst ::llc::s3_t selectedEnd = ::llc::max(app.SelectionAnchor, app.SelectionCursor);
			for(::llc::u2_t iCharacter = 0; iCharacter < text.size(); ++iCharacter) {
				cnst ::llc::s3_t position = rowByte + iCharacter;
				cnst bool selected = app.SelectAll || (0 <= selectedBegin && selectedBegin <= position && position < selectedEnd);
				states[iCharacter] = {};
				states[iCharacter].Flag = ::llc::setSelected(states[iCharacter].Flag, states[iCharacter].Mask, selected);
				states[iCharacter].Flag = ::llc::setSwapColors(states[iCharacter].Flag, states[iCharacter].Mask, selected);
			}
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

sttc ::llc::err_t desktopScrollCreate(SBlitterDashboard & app, SBlitterScroll & scroll, ::llc::u0_t iViewport, bool vertical) {
	cnst ::llc::cid_t parent = app.Viewports[iViewport].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
	if_fail_fe(scroll.IdTrack = ::llc::controlCreateChild(app.GUI, parent));
	if_fail_fe(scroll.IdThumb = ::llc::controlCreateChild(app.GUI, scroll.IdTrack));
	scroll.Vertical = vertical;
	app.GUI.Controls.Placement[scroll.IdTrack].Border = {1, 1, 1, 1};
	app.GUI.Controls.Placement[scroll.IdThumb].Border = {1, 1, 1, 1};
	::sampleGUIControlThemeApply(app.GUI, scroll.IdTrack, iViewport);
	::sampleGUIControlThemeApply(app.GUI, scroll.IdThumb, iViewport);
	rtrn 0;
}

sttc ::llc::err_t desktopScrollLayout(SBlitterDashboard & app, SBlitterScroll & scroll, ::llc::u0_t iViewport, ::llc::u2_t visible, ::llc::u2_t total, ::llc::u2_t & value, bool corner) {
	cnst ::llc::cid_t parent = app.Viewports[iViewport].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
	cnst ::llc::n2s1_t clientSize = app.GUI.Controls.Placement[parent].Area.Size;
	cnst ::llc::u2_t maximum = total > visible ? total - visible : 0;
	value = ::llc::min(value, maximum);
	::llc::SControlPlacement & track = app.GUI.Controls.Placement[scroll.IdTrack];
	::llc::SControlPlacement & thumb = app.GUI.Controls.Placement[scroll.IdThumb];
	cnst ::llc::s1_t trackLength = ::llc::max((::llc::s1_t)0, (::llc::s1_t)((scroll.Vertical ? clientSize.y : clientSize.x) - (corner ? 14 : 0)));
	track.Area.Offset = scroll.Vertical ? ::llc::n2s1_t{(::llc::s1_t)::llc::max(0, (int)clientSize.x - 14), 0} : ::llc::n2s1_t{0, (::llc::s1_t)::llc::max(0, (int)clientSize.y - 14)};
	track.Area.Size = scroll.Vertical ? ::llc::n2s1_t{14, trackLength} : ::llc::n2s1_t{trackLength, 14};
	cnst ::llc::s1_t thumbLength = maximum && total ? (::llc::s1_t)::llc::min((::llc::u2_t)trackLength, ::llc::max((::llc::u2_t)16, (::llc::u2_t)((::llc::u3_t)trackLength * visible / total))) : trackLength;
	cnst ::llc::s1_t travel = trackLength - thumbLength;
	cnst ::llc::s1_t position = maximum && travel ? (::llc::s1_t)((::llc::u3_t)travel * value / maximum) : 0;
	thumb.Area.Offset = scroll.Vertical ? ::llc::n2s1_t{0, position} : ::llc::n2s1_t{position, 0};
	thumb.Area.Size = scroll.Vertical ? ::llc::n2s1_t{14, thumbLength} : ::llc::n2s1_t{thumbLength, 14};
	rtrn ::llc::controlMetricsInvalidate(app.GUI, scroll.IdTrack);
}

sttc ::llc::err_t desktopScrollsLayout(SBlitterDashboard & app) {
	for(::llc::u0_t iViewport = 0; iViewport < 2; ++iViewport) {
		cnst ::llc::cid_t parent = app.Viewports[iViewport].IdControls[::llc::VIEWPORT_CONTROL_TARGET];
		cnst ::llc::s1_t width = (::llc::s1_t)::llc::max(0, (int)app.GUI.Controls.Placement[parent].Area.Size.x - 16);
		::llc::acid & controls = iViewport ? app.FileControls : app.FolderControls;
		for(::llc::u2_t i = 0; i < controls.size(); ++i)
			if(app.GUI.Controls.Placement[controls[i]].Area.Size.x != width) {
				app.GUI.Controls.Constraints[controls[i]].AttachSizeToControl.x = ::llc::CID_INVALID;
				app.GUI.Controls.Placement[controls[i]].Area.Size.x = width;
				if_fail_fe(::llc::controlMetricsInvalidate(app.GUI, controls[i]));
			}
	}
	if_fail_fe(::desktopScrollLayout(app, app.FolderScrollY, 0, DESKTOP_VISIBLE_ROWS - 1, app.FolderEntries.Name.size(), app.FolderOffset, false));
	if_fail_fe(::desktopScrollLayout(app, app.FileScrollY, 1, DESKTOP_VISIBLE_ROWS, app.TextRows.size(), app.FileOffset, true));
	rtrn ::desktopScrollLayout(app, app.FileScrollX, 1, app.VisibleColumns, app.WordWrap ? app.VisibleColumns : app.FileLineWidthMax, app.FileColumnOffset, true);
}

sttc bool desktopScrollInput(SBlitterDashboard & app, cnst SBlitterScroll & scroll, ::llc::u2_t maximum, ::llc::u2_t page, ::llc::u2_t & value) {
	cnst ::llc::SInput & input = *app.Window.Input;
	cnst ::llc::u2_t before = value;
	cnst ::llc::SControlArea & track = app.GUI.Controls.Area[scroll.IdTrack];
	cnst ::llc::SControlArea & thumb = app.GUI.Controls.Area[scroll.IdThumb];
	cnst ::llc::s2_t cursor = scroll.Vertical ? input.MouseCurrent.Position.y : input.MouseCurrent.Position.x;
	cnst ::llc::s2_t thumbStart = scroll.Vertical ? thumb.Total.Global.Offset.y : thumb.Total.Global.Offset.x;
	cnst ::llc::s2_t thumbLength = scroll.Vertical ? thumb.Total.Global.Size.y : thumb.Total.Global.Size.x;
	if(input.MouseCurrent.ButtonState[0] && app.GUI.Controls.IsPressed(scroll.IdThumb)) {
		cnst ::llc::s2_t trackStart = scroll.Vertical ? track.Client.Global.Offset.y : track.Client.Global.Offset.x;
		cnst ::llc::s2_t trackLength = scroll.Vertical ? track.Client.Global.Size.y : track.Client.Global.Size.x;
		cnst ::llc::s2_t travel = ::llc::max((::llc::s2_t)0, trackLength - thumbLength);
		if(travel)
			value = (::llc::u2_t)((::llc::u3_t)::llc::max((::llc::s2_t)0, ::llc::min(travel, cursor - trackStart - thumbLength / 2)) * maximum / travel);
	}
	else if(input.ButtonDown(0) && app.GUI.Controls.IsPressed(scroll.IdTrack))
		value = cursor < thumbStart ? (value > page ? value - page : 0) : ::llc::min(maximum, value + page);
	rtrn value != before;
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
	app.SelectionAnchor = app.SelectionCursor = -1;
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

sttc ::llc::err_t desktopFileSelectionUpdate(SBlitterDashboard & app) {
	cnst ::llc::SInput & input = *app.Window.Input;
	if(!input.ButtonDown(0) && !input.MouseCurrent.ButtonState[0])
		rtrn 0;
	cnst ::llc::u0_t characterWidth = ::llc::max((::llc::u0_t)1, app.GUI.Fonts[7]->CharSize.x);
	for(::llc::u2_t iControl = 0; iControl < app.FileControls.size(); ++iControl) {
		cnst ::llc::cid_t id = app.FileControls[iControl];
		if(app.GUI.Controls.IsHidden(id) || false == ::llc::in_range(app.GUI.CursorPos.s1(), app.GUI.Controls.Area[id].Total.Global))
			continue;
		cnst ::llc::u2_t iRow = app.FileOffset + iControl;
		if(iRow >= app.TextRows.size())
			continue;
		cnst ::llc::SGUIViewportTextRow & row = app.TextRows[iRow];
		cnst ::llc::s2_t mouseOffsetX = (::llc::s2_t)input.MouseCurrent.Position.x - app.GUI.Controls.Area[id].Text.Offset.x;
		cnst ::llc::u2_t visibleColumn = 0 >= mouseOffsetX ? 0 : (::llc::u2_t)(mouseOffsetX / characterWidth);
		cnst ::llc::u2_t firstColumn = app.WordWrap ? 0 : ::llc::min(app.FileColumnOffset, row.Text.size());
		cnst ::llc::s3_t cursor = ::desktopRowByte(app, iRow) + firstColumn + ::llc::min(visibleColumn, row.Text.size() - firstColumn);
		if(input.ButtonDown(0)) {
			app.SelectAll = false;
			app.SelectionAnchor = cursor;
			app.SelectionCursor = -1;
		}
		if(0 <= app.SelectionAnchor && cursor != app.SelectionCursor) {
			app.SelectionCursor = cursor;
			rtrn ::desktopFileRowsRefresh(app);
		}
		break;
	}
	rtrn 0;
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
	cnst bool binary = app.FileBytes.find('\0') >= 0;
	::llc::vcst_t remaining = app.FileBytes;
	if(0 == app.FileLength)
		if_fail_fe(app.FileLines.push_back(::llc::vcst_t{"Empty file"}));
	if(!binary && app.FileWindowStart && remaining.size()) {
		::llc::vcst_t partial = {}, next = {};
		if(0 <= ::llc::split('\n', remaining, partial, next))
			remaining = next;
	}
	if(!binary && app.FileWindowStart + app.FileBytes.size() < app.FileLength && remaining.size()) {
		cnst ::llc::err_t lastNewline = remaining.rfind('\n');
		if(lastNewline >= 0) {
			::llc::vcst_t prefix = {};
			if_fail_fe(remaining.slice(prefix, 0, (::llc::u2_t)lastNewline + 1));
			remaining = prefix;
		}
	}
	if(binary) {
		if_fail_fe(app.FileLines.push_back(::llc::vcst_t{"Binary file: CP437"}));
		for(::llc::u2_t iByte = 0; iByte < remaining.size(); iByte += 64) {
			::llc::vcst_t line = {};
			if_fail_fe(remaining.slice(line, iByte, ::llc::min((::llc::u2_t)64, remaining.size() - iByte)));
			if_fail_fe(app.FileLines.push_back(line));
		}
	}
	else
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
	app.SelectionAnchor = app.SelectionCursor = -1;
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
	cnst ::llc::err_t iJSON = app.OpenFile.rfind(::llc::vcst_t{".json", 5});
	if(!binary && app.FileLength <= DESKTOP_FILE_MAX_BYTES && 0 <= iJSON && (::llc::u2_t)iJSON + 5 == app.OpenFile.size())
		rtrn ::jsonFileShow(app);
	rtrn 0;
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
	app.GUI.ColorModeDefault = ::llc::GUI_COLOR_MODE_Flat;
	app.GUI.ThemeDefault = ::llc::GUI_THEME_DARK_CHROME - 1;
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
	if_fail_fe(::desktopScrollCreate(app, app.FolderScrollY, 0, true));
	if_fail_fe(::desktopScrollCreate(app, app.FileScrollY, 1, true));
	if_fail_fe(::desktopScrollCreate(app, app.FileScrollX, 1, false));
	::llc::string initialFolder = {};
	if_fail_fe(::llc::pathAbsolute(".", initialFolder));
	if_fail_fe(::desktopFolderOpen(app, initialFolder));
	if(0 <= ::llc::fileSize("./blitter.json"))
		if_fail_fe(::desktopFileOpen(app, "./blitter.json"));
	if_fail_fe(::desktopScrollsLayout(app));
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
	if_fail_fe(::llc::eventMouseDoubleClick(app.Window.EventQueue, doubleClick));
	::llc::u2_t hoveredViewport = app.Viewports.size();
	for(::llc::u0_t iViewport = 0; iViewport < app.Viewports.size(); ++iViewport) {
		::llc::SViewport & viewport = app.Viewports[iViewport];
		cnst ::llc::cid_t idTitle = viewport.IdControls[::llc::VIEWPORT_CONTROL_TITLE];
		cnst ::llc::cid_t idClose = viewport.IdControls[::llc::VIEWPORT_CONTROL_CLOSE];
		cnst ::llc::cid_t idResize = viewport.IdControls[::llc::VIEWPORT_CONTROL_RESIZE_BOTTOM_RIGHT];
		if(::llc::viewportIsPressed(app.GUI, viewport))
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
	cnst ::llc::u2_t folderMaximum = app.FolderEntries.Name.size() > DESKTOP_VISIBLE_ROWS - 1 ? app.FolderEntries.Name.size() - (DESKTOP_VISIBLE_ROWS - 1) : 0;
	if(::desktopScrollInput(app, app.FolderScrollY, folderMaximum, DESKTOP_VISIBLE_ROWS - 1, app.FolderOffset))
		if_fail_fe(::desktopFolderRowsRefresh(app));
	cnst ::llc::u2_t fileMaximum = app.TextRows.size() > DESKTOP_VISIBLE_ROWS ? app.TextRows.size() - DESKTOP_VISIBLE_ROWS : 0;
	if(::desktopScrollInput(app, app.FileScrollY, fileMaximum, DESKTOP_VISIBLE_ROWS, app.FileOffset)) {
		if_fail_fe(::desktopFileRowsRefresh(app));
		if_fail_fe(::desktopFileWindowCheck(app));
	}
	cnst ::llc::u2_t columnMaximum = !app.WordWrap && app.FileLineWidthMax > app.VisibleColumns ? app.FileLineWidthMax - app.VisibleColumns : 0;
	if(::desktopScrollInput(app, app.FileScrollX, columnMaximum, app.VisibleColumns, app.FileColumnOffset))
		if_fail_fe(::desktopFileRowsRefresh(app));
	if_fail_fe(::desktopFileSelectionUpdate(app));
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
	if_fail_fe(::desktopScrollsLayout(app));
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
