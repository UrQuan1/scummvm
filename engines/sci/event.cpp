/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/events.h"
#include "common/file.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/console.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#ifdef ENABLE_SCI32
#include "sci/graphics/cursor32.h"
#include "sci/graphics/frameout.h"
#endif
#include "sci/graphics/screen.h"

namespace Sci {

struct MouseEventConversion {
	Common::EventType commonType;
	SciEventType sciType;
};

static const MouseEventConversion mouseEventMappings[] = {
	{ Common::EVENT_LBUTTONDOWN , kSciEventMousePress   },
	{ Common::EVENT_RBUTTONDOWN , kSciEventMousePress   },
	{ Common::EVENT_MBUTTONDOWN , kSciEventMousePress   },
	{ Common::EVENT_LBUTTONUP   , kSciEventMouseRelease },
	{ Common::EVENT_RBUTTONUP   , kSciEventMouseRelease },
	{ Common::EVENT_MBUTTONUP   , kSciEventMouseRelease }
};

EventManager::EventManager(bool fontIsExtended) :
	_fontIsExtended(fontIsExtended)
#ifdef ENABLE_SCI32
	, _hotRectanglesActive(false)
#endif
	{}

EventManager::~EventManager() {
}

SciEvent EventManager::getScummVMEvent() {
#ifdef ENABLE_SCI32
	SciEvent input   = { kSciEventNone, kSciKeyModNone, 0, Common::Point(), Common::Point(), -1 };
	SciEvent noEvent = { kSciEventNone, kSciKeyModNone, 0, Common::Point(), Common::Point(), -1 };
#else
	SciEvent input   = { kSciEventNone, kSciKeyModNone, 0, Common::Point() };
	SciEvent noEvent = { kSciEventNone, kSciKeyModNone, 0, Common::Point() };
#endif

	Common::EventManager *em = g_system->getEventManager();
	Common::Event ev;

	// SCI does not generate separate events for mouse movement (it puts the
	// current mouse position on every event, including non-mouse events), so
	// skip past all mousemove events in the event queue
	bool found;
	do {
		found = em->pollEvent(ev);
	} while (found && ev.type == Common::EVENT_MOUSEMOVE);

	Common::Point mousePos = em->getMousePos();

#if ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		const GfxFrameout *gfxFrameout = g_sci->_gfxFrameout;

		// This will clamp `mousePos` according to the restricted zone,
		// so any cursor or screen item associated with the mouse position
		// does not bounce when it hits the edge (or ignore the edge)
		g_sci->_gfxCursor32->deviceMoved(mousePos);

		Common::Point mousePosSci = mousePos;
		mulru(mousePosSci, Ratio(gfxFrameout->getScriptWidth(), gfxFrameout->getScreenWidth()), Ratio(gfxFrameout->getScriptHeight(), gfxFrameout->getScreenHeight()));
		noEvent.mousePosSci = input.mousePosSci = mousePosSci;

		if (_hotRectanglesActive) {
			checkHotRectangles(mousePosSci);
		}
	} else {
#endif
		g_sci->_gfxScreen->adjustBackUpscaledCoordinates(mousePos.y, mousePos.x);
#if ENABLE_SCI32
	}
#endif

	noEvent.mousePos = input.mousePos = mousePos;

	if (!found || ev.type == Common::EVENT_MOUSEMOVE) {
		int modifiers = em->getModifierState();
		if (modifiers & Common::KBD_ALT)
			noEvent.modifiers |= kSciKeyModAlt;
		if (modifiers & Common::KBD_CTRL)
			noEvent.modifiers |= kSciKeyModCtrl;
		if (modifiers & Common::KBD_SHIFT)
			noEvent.modifiers |= kSciKeyModShift;

		return noEvent;
	}
	if (ev.type == Common::EVENT_QUIT || ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
		input.type = kSciEventQuit;
		return input;
	}

	int scummVMKeyFlags;

	switch (ev.type) {
	case Common::EVENT_KEYDOWN:
	case Common::EVENT_KEYUP:
		// Use keyboard modifiers directly in case this is a keyboard event
		scummVMKeyFlags = ev.kbd.flags;
		break;
	default:
		// Otherwise get them from EventManager
		scummVMKeyFlags = em->getModifierState();
		break;
	}

	// Caps lock and scroll lock are not handled here because we already
	// handle upper case keys elsewhere, and scroll lock doesn't seem to
	// ever be used
	input.modifiers = kSciKeyModNone;
	if (scummVMKeyFlags & Common::KBD_ALT)
		input.modifiers |= kSciKeyModAlt;
	if (scummVMKeyFlags & Common::KBD_CTRL)
		input.modifiers |= kSciKeyModCtrl;
	if (scummVMKeyFlags & Common::KBD_SHIFT)
		input.modifiers |= kSciKeyModShift;

	// Handle mouse events
	for (int i = 0; i < ARRAYSIZE(mouseEventMappings); i++) {
		if (mouseEventMappings[i].commonType == ev.type) {
			input.type = mouseEventMappings[i].sciType;
			// Sierra passed keyboard modifiers for mouse events, too.

			// Sierra also set certain modifiers within their mouse interrupt handler
			// This whole thing was probably meant for people using a mouse, that only featured 1 button
			// So the user was able to press Ctrl and click the mouse button to create a right click.
			switch (ev.type) {
			case Common::EVENT_RBUTTONDOWN: // right button
			case Common::EVENT_RBUTTONUP:
				input.modifiers |= kSciKeyModShift; // this value was hardcoded in the mouse interrupt handler
				break;
			case Common::EVENT_MBUTTONDOWN: // middle button
			case Common::EVENT_MBUTTONUP:
				input.modifiers |= kSciKeyModCtrl; // this value was hardcoded in the mouse interrupt handler
				break;
			default:
				break;
			}
			return input;
		}
	}

	// Handle keyboard events for the rest of the function
	if (ev.type != Common::EVENT_KEYDOWN && ev.type != Common::EVENT_KEYUP) {
		return noEvent;
	}

	// The IBM keyboard driver prior to SCI1.1 only sent keydown events to the
	// interpreter
	if (ev.type != Common::EVENT_KEYDOWN && getSciVersion() < SCI_VERSION_1_1) {
		return noEvent;
	}

	switch (g_sci->getLanguage()) {
	case Common::RU_RUS:
		input.character = ev.kbd.getINT16h00hKey(Common::kCodePage866);
		break;
	case Common::PL_POL:
		input.character = ev.kbd.getINT16h00hKey(Common::kWindows1250);
		break;
	case Common::HE_ISR:
		input.character = ev.kbd.getINT16h00hKey(Common::kWindows1255);
		break;
	default:
		input.character = ev.kbd.getINT16h00hKey(Common::kCodePage437);
		break;
	}
	if (input.character & 0xFF)
		input.character &= 0xFF;

	if (input.character >= 0x80 && input.character <= 0xFF) {
		// SSCI accepted all input scan codes, regardless of locale, and
		// just didn't display any characters that were missing from fonts
		// used by text input controls. We intentionally filter them out
		// entirely for non-multilingual games here instead, so we can have
		// better error detection for bugs in the text controls
		if (!_fontIsExtended) {
			return noEvent;
		}
	}

	// In SCI1.1, if only a modifier key is pressed, the IBM keyboard driver
	// sends an event the same as if a key had been released
	if (getSciVersion() != SCI_VERSION_1_1 && !input.character) {
		return noEvent;
	} else if (!input.character || ev.type == Common::EVENT_KEYUP) {
		input.type = kSciEventKeyUp;

		// SCI32 includes the released key character code in keyup messages, but
		// the IBM keyboard driver in SCI1.1 sends a special character value
		// instead. This is necessary to prevent at least Island of Dr Brain
		// from processing keyup events as though they were keydown events in
		// the word search puzzle
		if (getSciVersion() == SCI_VERSION_1_1) {
			input.character = 0x8000;
		}
	} else {
		input.type = kSciEventKeyDown;
	}

	return input;
}

void EventManager::updateScreen() {
	// Update the screen here, since it's called very often.
	// Throttle the screen update rate to 60fps.
	EngineState *s = g_sci->getEngineState();
	if (g_system->getMillis() - s->_screenUpdateTime >= 1000 / 60) {
		g_system->updateScreen();
		s->_screenUpdateTime = g_system->getMillis();
		// Throttle the checking of shouldQuit() to 60fps as well, since
		// Engine::shouldQuit() invokes 2 virtual functions
		// (EventManager::shouldQuit() and EventManager::shouldReturnToLauncher()),
		// which is very expensive to invoke constantly without any
		// throttling at all.
		if (g_engine->shouldQuit())
			s->abortScriptProcessing = kAbortQuitGame;
	}
}

SciEvent EventManager::getSciEvent(SciEventType mask) {
#ifdef ENABLE_SCI32
	SciEvent event = { kSciEventNone, kSciKeyModNone, 0, Common::Point(), Common::Point(), -1 };
#else
	SciEvent event = { kSciEventNone, kSciKeyModNone, 0, Common::Point() };
#endif

	if (getSciVersion() < SCI_VERSION_2) {
		updateScreen();
	}

	// Get all queued events from graphics driver
	do {
		event = getScummVMEvent();
		if (event.type != kSciEventNone)
			_events.push_back(event);
	} while (event.type != kSciEventNone);

	// Search for matching event in queue
	Common::List<SciEvent>::iterator iter = _events.begin();
	while (iter != _events.end() && !(iter->type & mask))
		++iter;

	if (iter != _events.end()) {
		// Event found
		event = *iter;

		// If not peeking at the queue, remove the event
		if (!(mask & kSciEventPeek))
			_events.erase(iter);
	} else {
		// No event found: we must return a kSciEventNone event.

		// Because event.type is kSciEventNone already here,
		// there is no need to change it.
	}

	return event;
}

void EventManager::flushEvents() {
	Common::EventManager *em = g_system->getEventManager();
	Common::Event event;
	while (em->pollEvent(event)) {}
	_events.clear();
}

#ifdef ENABLE_SCI32
void EventManager::setHotRectanglesActive(const bool active) {
	_hotRectanglesActive = active;
}

void EventManager::setHotRectangles(const Common::Array<Common::Rect> &rects) {
	_hotRects = rects;
	_activeRectIndex = -1;
}

void EventManager::checkHotRectangles(const Common::Point &mousePosition) {
	int lastActiveRectIndex = _activeRectIndex;
	_activeRectIndex = -1;

	for (int16 i = 0; i < (int16)_hotRects.size(); ++i) {
		if (_hotRects[i].contains(mousePosition)) {
			_activeRectIndex = i;
			if (i != lastActiveRectIndex) {
				SciEvent hotRectEvent;
				hotRectEvent.type = kSciEventHotRectangle;
				hotRectEvent.hotRectangleIndex = i;
				_events.push_front(hotRectEvent);
				break;
			}

			lastActiveRectIndex = _activeRectIndex;
		}
	}

	if (lastActiveRectIndex != _activeRectIndex && lastActiveRectIndex != -1) {
		_activeRectIndex = -1;
		SciEvent hotRectEvent;
		hotRectEvent.type = kSciEventHotRectangle;
		hotRectEvent.hotRectangleIndex = -1;
		_events.push_front(hotRectEvent);
	}
}
#endif

} // End of namespace Sci
