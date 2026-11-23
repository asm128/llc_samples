#include "llc_gui_viewport.h"
#include "llc_apod_serialize.h"

#ifndef LLC_GUI_COMPOSITE_H_23627
#define LLC_GUI_COMPOSITE_H_23627

namespace llc {
	enum GUI_VIEWPORT_ACTION : ::llc::u0_t { GUI_VIEWPORT_ACTION_WRAP, GUI_VIEWPORT_ACTION_SAVE, GUI_VIEWPORT_ACTION_SELECT_ALL, GUI_VIEWPORT_ACTION_COUNT, GUI_VIEWPORT_ACTION_NONE = 0xFF };

	struct SGUIViewportTextRow {
		::llc::vcst_t Text = {};
		::llc::u2_t SourceLine = 0;
		::llc::u2_t SourceColumn = 0;
	};

	template<typename TLines>
	inline ::llc::error_t guiViewportTextRowsBuild(const TLines & source, ::llc::u2_t charactersPerRow, bool wrap, ::llc::aobj<::llc::SGUIViewportTextRow> & rows) {
		if_fail_fe(rows.clear());
		charactersPerRow = ::llc::max((::llc::u2_t)1, charactersPerRow);
		for(::llc::u2_t iLine = 0; iLine < source.size(); ++iLine) {
			const ::llc::vcst_t line = source[iLine];
			if(!wrap || !line.size()) {
				if_fail_fe(rows.push_back({line, iLine, 0}));
				continue;
			}
			for(::llc::u2_t column = 0; column < line.size(); column += charactersPerRow) {
				::llc::vcst_t part = {};
				if_fail_fe(line.slice(part, column, ::llc::min(charactersPerRow, line.size() - column)));
				if_fail_fe(rows.push_back({part, iLine, column}));
			}
		}
		return 0;
	}

	inline ::llc::u2_t guiViewportTextColumns(const ::llc::SGUI & gui, ::llc::cid_t row) {
		const ::llc::u2_t font = gui.Controls.Text[row].FontSelected < 0 ? gui.SelectedFont : (::llc::u2_t)gui.Controls.Text[row].FontSelected;
		return (::llc::u2_t)::llc::max((::llc::s2_t)1, (::llc::s2_t)gui.Controls.Area[row].Client.Global.Size.x / ::llc::max((::llc::u0_t)1, gui.Fonts[font]->CharSize.x));
	}

	inline ::llc::error_t guiViewportEvent(const ::llc::SEventSystem & event, const ::llc::astcid<GUI_VIEWPORT_ACTION_COUNT> & actions, ::llc::cid_t resizedControl, ::llc::GUI_VIEWPORT_ACTION & action, bool & resized) {
		return ::llc::eventExtractAndHandle<::llc::EVENT_GUI_CONTROL>(event, [&actions, resizedControl, &action, &resized](const ::llc::SEView<::llc::EVENT_GUI_CONTROL> & child) {
			if(child.Type != ::llc::EVENT_GUI_CONTROL_StateChange)
				return (::llc::error_t)0;
			::llc::vcu0_t payload = child.Data;
			::llc::SChangeControlState change = {};
			if_fail_fe(::llc::loadPOD(payload, change));
			if(change.Id == resizedControl && (change.Set & ::llc::GUI_CONTROL_FLAG_Updated))
				resized = true;
			if(change.Set & ::llc::GUI_CONTROL_FLAG_Action)
				for(::llc::u0_t i = 0; i < ::llc::GUI_VIEWPORT_ACTION_COUNT; ++i)
					if(change.Id == actions[i])
						action = (::llc::GUI_VIEWPORT_ACTION)i;
			return (::llc::error_t)0;
		});
	}

	inline ::llc::error_t guiViewportEvents(const ::llc::SGUI & gui, const ::llc::astcid<GUI_VIEWPORT_ACTION_COUNT> & actions, ::llc::cid_t resizedControl, ::llc::GUI_VIEWPORT_ACTION & action, bool & resized) {
		for(::llc::u2_t i = 0; i < gui.Controls.EventQueue.size(); ++i)
			if(gui.Controls.EventQueue[i].get_ref())
				if_fail_fe(::llc::guiViewportEvent(*gui.Controls.EventQueue[i], actions, resizedControl, action, resized));
		return 0;
	}

	inline ::llc::cid_t guiCloneControl(::llc::SGUI & gui, ::llc::cid_t source, ::llc::cid_t parent) {
		if(::llc::controlInvalid(gui, source))
			return ::llc::CID_INVALID;
		const ::llc::cid_t id = ::llc::controlCreateChild(gui, parent);
		if(id < 0)
			return id;
		gui.Controls.Placement[id] = gui.Controls.Placement[source];
		gui.Controls.Constraints[id] = gui.Controls.Constraints[source];
		if(gui.Controls.Constraints[id].AttachSizeToControl.x == source)
			gui.Controls.Constraints[id].AttachSizeToControl.x = id;
		if(gui.Controls.Constraints[id].AttachSizeToControl.y == source)
			gui.Controls.Constraints[id].AttachSizeToControl.y = id;
		gui.Controls.Draw[id] = gui.Controls.Draw[source];
		gui.Controls.Text[id] = gui.Controls.Text[source];
		gui.Controls.Images[id] = gui.Controls.Images[source];
		gui.Controls.SetHidden(id, gui.Controls.IsHidden(source));
		gui.Controls.SetDisabled(id, gui.Controls.IsDisabled(source));
		gui.Controls.SetSelected(id, gui.Controls.IsSelected(source));
		return id;
	}

	inline ::llc::error_t guiViewportActionsLayout(::llc::SGUI & gui, const ::llc::SViewport & viewport, const ::llc::astcid<GUI_VIEWPORT_ACTION_COUNT> & actions) {
		const ::llc::cid_t close = viewport.IdControls[::llc::VIEWPORT_CONTROL_CLOSE];
		const ::llc::GUI_VIEWPORT_ACTION order[] = {GUI_VIEWPORT_ACTION_SELECT_ALL, GUI_VIEWPORT_ACTION_SAVE, GUI_VIEWPORT_ACTION_WRAP};
		::llc::cid_t right = close;
		::llc::s1_t buttonOffset = gui.Controls.Placement[close].Area.Offset.x;
		const ::llc::s1_t buttonWidth = gui.Controls.Placement[close].Area.Size.x;
		for(::llc::u0_t i = 0; i < 3; ++i) {
			const ::llc::cid_t id = actions[order[i]];
			gui.Controls.Constraints[id].DockToControl.Right = -1;
			if(!gui.Controls.IsHidden(id)) {
				buttonOffset += buttonWidth;
				gui.Controls.Placement[id].Area.Offset.x = buttonOffset;
				right = id;
			}
		}
		gui.Controls.Constraints[viewport.IdControls[::llc::VIEWPORT_CONTROL_TITLE]].DockToControl.Right = right;
		return ::llc::controlMetricsInvalidate(gui, viewport.IdControl);
	}

	inline ::llc::error_t guiViewportActionShow(::llc::SGUI & gui, const ::llc::SViewport & viewport, const ::llc::astcid<GUI_VIEWPORT_ACTION_COUNT> & actions, ::llc::GUI_VIEWPORT_ACTION action, bool show) {
		if(action >= GUI_VIEWPORT_ACTION_COUNT)
			return -1;
		gui.Controls.SetHidden(actions[action], !show);
		return ::llc::guiViewportActionsLayout(gui, viewport, actions);
	}

	inline ::llc::cid_t guiCreateViewport(::llc::SGUI & gui, ::llc::SViewport & viewport, ::llc::cid_t parent, ::llc::astcid<GUI_VIEWPORT_ACTION_COUNT> & actions, bool showWrap = false, bool showSave = false, bool showSelectAll = false) {
		if(::llc::viewportInitialize(gui, viewport) < 0 || ::llc::controlSetParent(gui, viewport.IdControl, parent) < 0)
			return ::llc::CID_INVALID;
		const ::llc::cid_t close = viewport.IdControls[::llc::VIEWPORT_CONTROL_CLOSE];
		const ::llc::vcst_t labels[] = {"A", "S", "W"};
		const ::llc::GUI_VIEWPORT_ACTION order[] = {GUI_VIEWPORT_ACTION_SELECT_ALL, GUI_VIEWPORT_ACTION_SAVE, GUI_VIEWPORT_ACTION_WRAP};
		const bool visible[] = {showSelectAll, showSave, showWrap};
		for(::llc::u0_t i = 0; i < 3; ++i) {
			const ::llc::cid_t id = ::llc::guiCloneControl(gui, close, viewport.IdControl);
			if(id < 0)
				return id;
			gui.Controls.Text[id].Text = labels[i];
			actions[order[i]] = id;
			gui.Controls.SetHidden(id, !visible[i]);
		}
		if(::llc::guiViewportActionsLayout(gui, viewport, actions) < 0)
			return ::llc::CID_INVALID;
		return viewport.IdControl;
	}

	inline ::llc::cid_t guiCloneViewport(::llc::SGUI & gui, const ::llc::SViewport & source, const ::llc::astcid<GUI_VIEWPORT_ACTION_COUNT> & sourceActions, ::llc::SViewport & target, ::llc::astcid<GUI_VIEWPORT_ACTION_COUNT> & targetActions, ::llc::cid_t parent) {
		const ::llc::cid_t id = ::llc::guiCreateViewport(gui, target, parent, targetActions
			, !gui.Controls.IsHidden(sourceActions[GUI_VIEWPORT_ACTION_WRAP])
			, !gui.Controls.IsHidden(sourceActions[GUI_VIEWPORT_ACTION_SAVE])
			, !gui.Controls.IsHidden(sourceActions[GUI_VIEWPORT_ACTION_SELECT_ALL]));
		if(id < 0)
			return id;
		gui.Controls.Placement[id] = gui.Controls.Placement[source.IdControl];
		gui.Controls.Draw[id] = gui.Controls.Draw[source.IdControl];
		gui.Controls.Text[id] = gui.Controls.Text[source.IdControl];
		for(::llc::u0_t i = 0; i < ::llc::VIEWPORT_CONTROL_COUNT; ++i) {
			const ::llc::cid_t from = source.IdControls[i], to = target.IdControls[i];
			gui.Controls.Placement[to] = gui.Controls.Placement[from];
			gui.Controls.Draw[to] = gui.Controls.Draw[from];
			gui.Controls.Text[to] = gui.Controls.Text[from];
			gui.Controls.SetHidden(to, gui.Controls.IsHidden(from));
		}
		for(::llc::u0_t i = 0; i < GUI_VIEWPORT_ACTION_COUNT; ++i) {
			const ::llc::cid_t from = sourceActions[i], to = targetActions[i];
			gui.Controls.Placement[to] = gui.Controls.Placement[from];
			gui.Controls.Draw[to] = gui.Controls.Draw[from];
			gui.Controls.Text[to] = gui.Controls.Text[from];
			gui.Controls.SetHidden(to, gui.Controls.IsHidden(from));
			gui.Controls.SetDisabled(to, gui.Controls.IsDisabled(from));
		}
		target.UnfoldedSize = source.UnfoldedSize;
		target.Folded = source.Folded;
		if(::llc::guiViewportActionsLayout(gui, target, targetActions) < 0)
			return ::llc::CID_INVALID;
		return id;
	}

	inline ::llc::error_t guiViewportFold(::llc::SGUI & gui, ::llc::SViewport & viewport) {
		return ::llc::viewportFold(gui, viewport);
	}
}

#endif // LLC_GUI_COMPOSITE_H_23627
