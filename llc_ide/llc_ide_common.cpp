#include "llc_ide_common.h"
#include "llc_event_input.h"
#include "llc_view_serialize.h"


sttc llc::err_t fileViewerTextViewsClear(SGUI & gui, SFileEditor & editor) {
	for(u2_t iControl = 0; iControl < editor.Lines.IdControls.size(); ++iControl)
		if_fail_fe(controlTextSet(gui, editor.Lines.IdControls[iControl], {}));
	editor.FileLines.clear();
	editor.FileLineWidthMax = 0;
	rtrn 0;
}

llc::err_t fileViewerTextViewsRebuild(SFileEditor & editor) {
	editor.FileLines.clear();
	editor.FileLineWidthMax = 0;
	vcst_t remaining = editor.FileText;
	do {
		vcst_t line = {};
		vcst_t next = {};
		cnst llc::err_t splitResult = split('\n', remaining, line, next);
		if(line.size() && '\r' == line[line.size() - 1])
			if_fail_fe(line.slice(line, 0, line.size() - 1));
		if_fail_fe(editor.FileLines.push_back(line));
		editor.FileLineWidthMax = max(editor.FileLineWidthMax, line.size());
		if(0 > splitResult)
			break;
		remaining = next;
	} while(true);
	rtrn 0;
}

u2_t fileViewerLineOffset(cnst SFileEditor & editor, u2_t line) {
	u2_t offset = 0;
	u2_t iLine = 0;
	while(offset < editor.FileText.size() && iLine < line)
		if('\n' == editor.FileText[offset++])
			++iLine;
	rtrn offset;
}

sttc u2_t fileViewerCursorOffset(cnst SFileEditor & editor) {
	if(editor.Cursor.y >= editor.FileLines.size())
		rtrn editor.FileText.size();
	vcst_c line = editor.FileLines[editor.Cursor.y];
	rtrn ::fileViewerLineOffset(editor, editor.Cursor.y) + min(editor.Cursor.x, line.size());
}

sttc void fileViewerCursorOffsetSet(SFileEditor & editor, u2_t offset) {
	editor.Cursor = {};
	for(u2_t iCharacter = 0, countCharacter = min(offset, editor.FileText.size()); iCharacter < countCharacter; ++iCharacter) {
		if('\n' == editor.FileText[iCharacter]) {
			++editor.Cursor.y;
			editor.Cursor.x = 0;
		}
		else if('\r' != editor.FileText[iCharacter] || iCharacter + 1 >= editor.FileText.size() || '\n' != editor.FileText[iCharacter + 1])
			++editor.Cursor.x;
	}
}

llc::err_t fileViewerTextInsert(SGUI & gui, SFileEditor & editor, sc_t character) {
	u2_c offset = ::fileViewerCursorOffset(editor);
	if_fail_fe(::fileViewerTextViewsClear(gui, editor));
	if_fail_fe(editor.FileText.insert(offset, character));
	editor.TextSelections.clear();
	if_fail_fe(::fileViewerTextViewsRebuild(editor));
	if('\n' == character) {
		++editor.Cursor.y;
		editor.Cursor.x = 0;
	}
	else
		++editor.Cursor.x;
	rtrn 0;
}

llc::err_t fileViewerTextBackspace(SGUI & gui, SFileEditor & editor) {
	if(editor.Cursor.y >= editor.FileLines.size())
		rtrn 0;
	u2_c cursorOffset = ::fileViewerCursorOffset(editor);
	if(editor.Cursor.x) {
		if_fail_fe(::fileViewerTextViewsClear(gui, editor));
		if_fail_fe(editor.FileText.remove(cursorOffset - 1));
		--editor.Cursor.x;
	}
	else if(editor.Cursor.y) {
		vcst_c previousLine = editor.FileLines[editor.Cursor.y - 1];
		u2_c separatorOffset = ::fileViewerLineOffset(editor, editor.Cursor.y - 1) + previousLine.size();
		u2_c separatorCount = cursorOffset - separatorOffset;
		editor.Cursor.x = previousLine.size();
		--editor.Cursor.y;
		if_fail_fe(::fileViewerTextViewsClear(gui, editor));
		for(u2_t iCharacter = 0; iCharacter < separatorCount; ++iCharacter)
			if_fail_fe(editor.FileText.remove(separatorOffset));
	}
	editor.TextSelections.clear();
	rtrn ::fileViewerTextViewsRebuild(editor);
}

llc::err_t fileViewerTextDelete(SGUI & gui, SFileEditor & editor) {
	if(editor.TextSelections.size()) {
		apod<rangeu2_t> deletions = {};
		u2_t cursorOffset = editor.FileText.size();
		for(u2_t iSelection = 0; iSelection < editor.TextSelections.size(); ++iSelection) {
			rangeu2_c selection = editor.TextSelections[iSelection];
			u2_c selectionBegin = min(selection.Offset, editor.FileText.size());
			u2_c selectionEnd = (u2_t)min((u3_t)editor.FileText.size(), (u3_t)selection.Offset + selection.Count);
			if(selectionBegin < selectionEnd) {
				if_fail_fe(deletions.push_back({selectionBegin, selectionEnd - selectionBegin}));
				cursorOffset = min(cursorOffset, selectionBegin);
			}
		}
		if(deletions.size()) {
			for(u2_t iDeletion = 1; iDeletion < deletions.size(); ++iDeletion) {
				rangeu2_c deletion = deletions[iDeletion];
				u2_t iPrevious = iDeletion;
				while(iPrevious && deletions[iPrevious - 1].Offset > deletion.Offset) {
					deletions[iPrevious] = deletions[iPrevious - 1];
					--iPrevious;
				}
				deletions[iPrevious] = deletion;
			}
			u2_t countMerged = 1;
			for(u2_t iDeletion = 1; iDeletion < deletions.size(); ++iDeletion) {
				rangeu2_t & previous = deletions[countMerged - 1];
				rangeu2_c current = deletions[iDeletion];
				u3_c previousEnd = (u3_t)previous.Offset + previous.Count;
				u3_c currentEnd = (u3_t)current.Offset + current.Count;
				if(current.Offset <= previousEnd)
					previous.Count = (u2_t)(max(previousEnd, currentEnd) - previous.Offset);
				else
					deletions[countMerged++] = current;
			}
			if_fail_fe(deletions.resize(countMerged));
			if_fail_fe(::fileViewerTextViewsClear(gui, editor));
			for(u2_t iDeletion = deletions.size(); iDeletion; --iDeletion)
				for(u2_t iCharacter = 0; iCharacter < deletions[iDeletion - 1].Count; ++iCharacter)
					if_fail_fe(editor.FileText.remove(deletions[iDeletion - 1].Offset));
			editor.TextSelections.clear();
			if_fail_fe(::fileViewerTextViewsRebuild(editor));
			::fileViewerCursorOffsetSet(editor, cursorOffset);
			rtrn 0;
		}
		editor.TextSelections.clear();
	}
	if(editor.Cursor.y >= editor.FileLines.size())
		rtrn 0;
	u2_c cursorOffset = ::fileViewerCursorOffset(editor);
	if(cursorOffset >= editor.FileText.size())
		rtrn 0;
	u2_t count = ('\r' == editor.FileText[cursorOffset] && cursorOffset + 1 < editor.FileText.size() && '\n' == editor.FileText[cursorOffset + 1]) ? 2 : 1;
	if_fail_fe(::fileViewerTextViewsClear(gui, editor));
	for(u2_t iCharacter = 0; iCharacter < count; ++iCharacter)
		if_fail_fe(editor.FileText.remove(cursorOffset));
	editor.TextSelections.clear();
	rtrn ::fileViewerTextViewsRebuild(editor);
}

llc::err_t fileViewerEditorHandle(SGUI & gui, SVirtualKeyboard & keyboard, SFileEditor & editor, SYSTEM_KEY scanCode) {
	if(editor.Cursor.y >= editor.FileLines.size())
		rtrn 0;
	switch(scanCode) {
	default: break;
	case SYSTEM_KEY_Escape:
		gui.Controls.SetHidden(keyboard.IdRoot, true);
		break;
	case SYSTEM_KEY_Delete		: if_fail_fe(::fileViewerTextDelete(gui, editor)); break;
	case SYSTEM_KEY_Enter		: if_fail_fe(::fileViewerTextInsert(gui, editor, '\n')); break;
	case SYSTEM_KEY_Backspace	: if_fail_fe(::fileViewerTextBackspace(gui, editor)); break;
	case SYSTEM_KEY_Clear:
		if_fail_fe(::fileViewerTextViewsClear(gui, editor));
		editor.FileText.clear();
		editor.TextSelections.clear();
		editor.Cursor = {};
		if_fail_fe(::fileViewerTextViewsRebuild(editor));
		break;
	case SYSTEM_KEY_Left:
		if(editor.Cursor.x)
			--editor.Cursor.x;
		else if(editor.Cursor.y) {
			--editor.Cursor.y;
			editor.Cursor.x = editor.FileLines[editor.Cursor.y].size();
		}
		break;
	case SYSTEM_KEY_Right:
		if(editor.Cursor.x < editor.FileLines[editor.Cursor.y].size())
			++editor.Cursor.x;
		else if(editor.Cursor.y + 1 < editor.FileLines.size()) {
			++editor.Cursor.y;
			editor.Cursor.x = 0;
		}
		break;
	case SYSTEM_KEY_Up:
		if(editor.Cursor.y) {
			--editor.Cursor.y;
			editor.Cursor.x = min(editor.Cursor.x, editor.FileLines[editor.Cursor.y].size());
		}
		break;
	case SYSTEM_KEY_Down:
		if(editor.Cursor.y + 1 < editor.FileLines.size()) {
			++editor.Cursor.y;
			editor.Cursor.x = min(editor.Cursor.x, editor.FileLines[editor.Cursor.y].size());
		}
		break;
	case SYSTEM_KEY_Home:
		editor.Cursor.x = 0;
		break;
	case SYSTEM_KEY_End:
		editor.Cursor.x = editor.FileLines[editor.Cursor.y].size();
		break;
	case SYSTEM_KEY_Prior:
		editor.Cursor.y = editor.Cursor.y > 20 ? editor.Cursor.y - 20 : 0;
		editor.Cursor.x = min(editor.Cursor.x, editor.FileLines[editor.Cursor.y].size());
		break;
	case SYSTEM_KEY_Next:
		editor.Cursor.y = min((u2_t)(editor.FileLines.size() - 1), (u2_t)(editor.Cursor.y + 20));
		editor.Cursor.x = min(editor.Cursor.x, editor.FileLines[editor.Cursor.y].size());
		break;
	}
	rtrn 0;
}

llc::err_t fileViewerEditorInput(SGUI & gui, SVirtualKeyboard & keyboard, SFileEditor & editor, TQueueSystemEvent frameEvents) {
	TQueueSystemEvent systemEvents = frameEvents;
	if_fail_fe(guiProcessControls(gui, [&keyboard, &systemEvents](cid_t iControl) {
		rtrn virtualKeyboardHandleEvent(keyboard, iControl, systemEvents);
	}));
	for(u2_t iEvent = 0; iEvent < systemEvents.size(); ++iEvent) {
		if_zero_fe(systemEvents[iEvent].get_ref());
		switch(systemEvents[iEvent]->Type) {
		case SYSTEM_EVENT_Text:
			if_fail_fe(eventExtractAndHandle<EVENT_TEXT>(*systemEvents[iEvent], [&gui, &editor](cnst SEView<EVENT_TEXT> & eventText) {
				if(eventText.Type != EVENT_TEXT_Char)
					rtrn 0;
				vcu0_t input = eventText.Data;
				u2_t character = {};
				if_fail_fe(loadPOD(input, character));
				if(character >= 0x20 && character <= 0xFF)
					rtrn ::fileViewerTextInsert(gui, editor, (sc_t)character);
				rtrn 0;
			}));
			break;
		case SYSTEM_EVENT_Keyboard:
			if_fail_fe(eventExtractAndHandle<EVENT_KEYBOARD>(*systemEvents[iEvent], [&gui, &keyboard, &editor](cnst SEView<EVENT_KEYBOARD> & eventKeyboard) {
				if(eventKeyboard.Type != EVENT_KEYBOARD_Down)
					rtrn 0;
				vcu0_t input = eventKeyboard.Data;
				u0_t key = {};
				if_fail_fe(loadPOD(input, key));
				rtrn ::fileViewerEditorHandle(gui, keyboard, editor, (SYSTEM_KEY)key);
			}));
			break;
		}
	}
	rtrn 0;
}
