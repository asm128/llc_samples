#include "llc_input.h"
#include "llc_range.h"

#ifndef LLC_FILE_SELECTION_H_23627
#define LLC_FILE_SELECTION_H_23627

static inline ::llc::err_t fileViewerSelectionUpdate
	(::llc::apod<::llc::rangeu2_t> & selections, ::llc::u2_t & anchor, ::llc::u2_t & active, bool capsSelectionMode
	, const ::llc::SInput & input, ::llc::u2_t characterOffset, bool overTextLine) {
	if(input.ButtonDown(0) && overTextLine) {
		if(false == capsSelectionMode || active >= selections.size() || selections[active].Count) {
			anchor = characterOffset;
			cnst ::llc::err_t iSelection = selections.push_back({characterOffset, 0});
			if_fail_fe(iSelection);
			active = (::llc::u2_t)iSelection;
		}
	}
	if(input.MouseCurrent.ButtonState[0] && (::llc::u2_t)-1 != anchor && active < selections.size() && overTextLine) {
		::llc::rangeu2_t & selection = selections[active];
		selection.Offset = ::llc::min(anchor, characterOffset);
		selection.Count = ::llc::max(anchor, characterOffset) - selection.Offset;
	}
	if(input.ButtonUp(0) && (::llc::u2_t)-1 != anchor) {
		if(active < selections.size() && 0 == selections[active].Count && false == capsSelectionMode)
			if_fail_fe(selections.remove(active));
		if(false == capsSelectionMode || active >= selections.size() || selections[active].Count) {
			anchor = (::llc::u2_t)-1;
			active = (::llc::u2_t)-1;
		}
	}
	rtrn 0;
}

#endif // LLC_FILE_SELECTION_H_23627
