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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "common/translation.h"
#include "audio/mixer.h"

#include "scumm/debugger.h"
#include "scumm/dialogs.h"
#include "scumm/insane/insane.h"
#include "scumm/imuse/imuse.h"
#ifdef ENABLE_HE
#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"
#endif
#include "scumm/resource.h"
#include "scumm/scumm_v0.h"
#include "scumm/scumm_v6.h"
#include "scumm/scumm_v8.h"
#include "scumm/sound.h"



namespace Scumm {

enum MouseButtonStatus {
	msDown = 1,
	msClicked = 2
};

uint16 ScummEngine::convertKey(const Common::KeyState &lastKeyHit) const {
	uint16 key = lastKeyHit.getINT16h10hKey();
	if (!key)
		return 0;

	// WORKAROUND: We cannot keep track of the Num Lock state on all platforms,
	// and so we need to hardcode the important cases here - bugs #10558, #11227.
	if (lastKeyHit.keycode >= Common::KEYCODE_KP0 && lastKeyHit.keycode <= Common::KEYCODE_KP9) {
		if (_game.id == GID_INDY3) {
			return '0' + lastKeyHit.keycode - Common::KEYCODE_KP0;
		} else if (_game.id == GID_INDY4 || _game.id == GID_PASS) {
			return (key >> 8) + 256;
		}
	}

	return key & 0xFF ? key &= 0xFF : (key >> 8) + 256;
}

#ifdef ENABLE_HE
void ScummEngine_v80he::parseEvent(Common::Event event) {
	ScummEngine::parseEvent(event);

	// Keyboard is controlled via variable
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		if (event.kbd.keycode == Common::KEYCODE_LEFT)
			VAR(VAR_KEY_STATE) |= 1;

		if (event.kbd.keycode == Common::KEYCODE_RIGHT)
			VAR(VAR_KEY_STATE) |= 2;

		if (event.kbd.keycode == Common::KEYCODE_UP)
			VAR(VAR_KEY_STATE) |= 4;

		if (event.kbd.keycode == Common::KEYCODE_DOWN)
			VAR(VAR_KEY_STATE) |= 8;

		if (event.kbd.keycode == Common::KEYCODE_LSHIFT || event.kbd.keycode == Common::KEYCODE_RSHIFT)
			VAR(VAR_KEY_STATE) |= 16;

		if (event.kbd.keycode == Common::KEYCODE_LCTRL || event.kbd.keycode == Common::KEYCODE_RCTRL)
			VAR(VAR_KEY_STATE) |= 32;
		break;

	case Common::EVENT_KEYUP:
		if (event.kbd.keycode == Common::KEYCODE_LEFT)
			VAR(VAR_KEY_STATE) &= ~1;

		if (event.kbd.keycode == Common::KEYCODE_RIGHT)
			VAR(VAR_KEY_STATE) &= ~2;

		if (event.kbd.keycode == Common::KEYCODE_UP)
			VAR(VAR_KEY_STATE) &= ~4;

		if (event.kbd.keycode == Common::KEYCODE_DOWN)
			VAR(VAR_KEY_STATE) &= ~8;

		if (event.kbd.keycode == Common::KEYCODE_LSHIFT || event.kbd.keycode == Common::KEYCODE_RSHIFT)
			VAR(VAR_KEY_STATE) &= ~16;

		if (event.kbd.keycode == Common::KEYCODE_LCTRL || event.kbd.keycode == Common::KEYCODE_RCTRL)
			VAR(VAR_KEY_STATE) &= ~32;
		break;

	default:
		break;
	}
}
#endif

void ScummEngine::parseEvent(Common::Event event) {
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		if (event.kbd.keycode >= Common::KEYCODE_0 && event.kbd.keycode <= Common::KEYCODE_9 &&
			((event.kbd.hasFlags(Common::KBD_ALT) && canSaveGameStateCurrently()) ||
			(event.kbd.hasFlags(Common::KBD_CTRL) && canLoadGameStateCurrently()))) {
			_saveLoadSlot = event.kbd.keycode - Common::KEYCODE_0;

			//  don't overwrite autosave (slot 0)
			if (_saveLoadSlot == 0)
				_saveLoadSlot = 10;

			_saveLoadDescription = Common::String::format("Quicksave %d", _saveLoadSlot);
			_saveLoadFlag = (event.kbd.hasFlags(Common::KBD_ALT)) ? 1 : 2;
			_saveTemporaryState = false;
		} else if (event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_f) {
			_fastMode ^= 1;
		} else if (event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_g) {
			_fastMode ^= 2;
		} else if (event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_s) {
			_res->resourceStats();
		} else if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformSegaCD &&
			event.kbd.keycode >= Common::KEYCODE_UP && event.kbd.keycode <= Common::KEYCODE_LEFT) {
			// WORKAROUND: This enables dialog choices to be scrolled through in the
			// SegaCD version of MI. Values are taken from script-14.
			// See bug report #1193185 for details.
			_keyPressed = Common::KeyState(Common::KEYCODE_INVALID, event.kbd.keycode - Common::KEYCODE_UP + 54);
		} else {
			// Normal key press, pass on to the game.
			_keyPressed = event.kbd;
		}
		_keyDownMap[event.kbd.keycode] = true;
		break;

	case Common::EVENT_KEYUP:
		_keyDownMap[event.kbd.keycode] = false;
		break;

	// We update the mouse position whenever the mouse moves or a click occurs.
	// The latter is done to accomodate systems with a touchpad / pen controller.
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_MOUSEMOVE:
		if (event.type == Common::EVENT_LBUTTONDOWN)
			_leftBtnPressed |= msClicked|msDown;
		else if (event.type == Common::EVENT_RBUTTONDOWN)
			_rightBtnPressed |= msClicked|msDown;
		_mouse.x = event.mouse.x;
		_mouse.y = event.mouse.y;

		if (_renderMode == Common::kRenderHercA || _renderMode == Common::kRenderHercG) {
			_mouse.x -= (kHercWidth - _screenWidth * 2) / 2;
			_mouse.x >>= 1;
			_mouse.y = _mouse.y * 4 / 7;
		} else if (_useCJKMode && _textSurfaceMultiplier == 2) {
			_mouse.x >>= 1;
			_mouse.y >>= 1;
		}
		break;
	case Common::EVENT_LBUTTONUP:
		_leftBtnPressed &= ~msDown;
		break;

	case Common::EVENT_RBUTTONUP:
		_rightBtnPressed &= ~msDown;
		break;

	// The following two cases enable dialog choices to be scrolled
	// through in the SegaCD version of MI. Values are taken from script-14.
	// See bug report #1193185 for details.
	case Common::EVENT_WHEELDOWN:
		if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformSegaCD)
			_keyPressed = Common::KeyState(Common::KEYCODE_INVALID, 55);	// '7'
		break;

	case Common::EVENT_WHEELUP:
		if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformSegaCD)
			_keyPressed = Common::KeyState(Common::KEYCODE_INVALID, 54);	// '6'
		break;

	default:
		break;
	}
}

void ScummEngine::parseEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		parseEvent(event);
	}
}

#ifdef ENABLE_HE
void ScummEngine_v90he::clearClickedStatus() {
	ScummEngine::clearClickedStatus();
	if (_game.heversion >= 98) {
		_logicHE->processKeyStroke(_mouseAndKeyboardStat);
	}
}

void ScummEngine_v90he::processInput() {
	ScummEngine::processInput();
	if (_game.heversion >= 98) {
		_logicHE->processKeyStroke(_mouseAndKeyboardStat);
	}
}
#endif

void ScummEngine::clearClickedStatus() {
	_keyPressed.reset();

	_mouseAndKeyboardStat = 0;
	_leftBtnPressed &= ~msClicked;
	_rightBtnPressed &= ~msClicked;
}

bool ScummEngine::isOriginalMenuKey() const {
	return ((_mouseAndKeyboardStat == SCUMM_KEY_SHIFT_F1 && _game.platform == Common::kPlatformC64) ||
	        (_mouseAndKeyboardStat == SCUMM_KEY_CTRL_L && _game.platform == Common::kPlatformApple2GS) ||
	        (_mouseAndKeyboardStat == SCUMM_KEY_F5 && _game.version >= 1 && _game.version <= 3 &&
	         _game.platform != Common::kPlatformMacintosh && _game.platform != Common::kPlatformPCEngine));
}

bool ScummEngine::isMainMenuKey() const {
	// Always enable F5.
	return ((_mouseAndKeyboardStat == SCUMM_KEY_F1 && _game.version >= 5) ||
	        (_mouseAndKeyboardStat == SCUMM_KEY_F5));
}

bool ScummEngine::isRestartKey() const {
	// Always enable F8.
	return ((_mouseAndKeyboardStat == SCUMM_KEY_SHIFT_F7 && _game.platform == Common::kPlatformC64) ||
	        (_mouseAndKeyboardStat == SCUMM_KEY_CTRL_R && _game.platform == Common::kPlatformApple2GS) ||
	        (_mouseAndKeyboardStat == SCUMM_KEY_F8));
}

bool ScummEngine::isQuitKey() const {
	return ((_mouseAndKeyboardStat == SCUMM_KEY_ALT_X && _game.version >= 3) ||
	        (_mouseAndKeyboardStat == SCUMM_KEY_CTRL_C && _game.version >= 4) ||
	        (_mouseAndKeyboardStat == SCUMM_KEY_ALT_Q && _game.version >= 5));
}

bool ScummEngine::isCutsceneExitKey() const {
	// Always enable Escape.
	return ((_mouseAndKeyboardStat == SCUMM_KEY_F7 && _game.id == GID_MANIAC && _game.platform == Common::kPlatformC64) ||
	        (_mouseAndKeyboardStat == SCUMM_KEY_RETURN && _game.id == GID_ZAK && _game.platform == Common::kPlatformC64) ||
	        (_mouseAndKeyboardStat == SCUMM_KEY_F4 && _game.id == GID_MANIAC && _game.version >= 1 && _game.platform != Common::kPlatformNES) ||
	        (_mouseAndKeyboardStat == SCUMM_KEY_ESCAPE));
}

bool ScummEngine::isSnapScrollKey() const {
	return ((_mouseAndKeyboardStat == SCUMM_KEY_SHIFT_S && _game.version == 2) ||
	        (_mouseAndKeyboardStat == SCUMM_KEY_ALT_I && _game.version == 3) ||
	        (_mouseAndKeyboardStat == SCUMM_KEY_CTRL_R && _game.version == 4));
}

void ScummEngine_v0::processInput() {
	// F1 - F3
	if (_keyPressed.keycode >= Common::KEYCODE_F1 && _keyPressed.keycode <= Common::KEYCODE_F3) {
		switchActor(_keyPressed.keycode - Common::KEYCODE_F1);
	}

	ScummEngine::processInput();
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::processInput() {
	ScummEngine::processInput();

	if (_skipVideo && !_smushActive) {
		abortCutscene();
		_skipVideo = false;
	}
}
#endif

void ScummEngine::processInput() {
	Common::KeyState lastKeyHit = _keyPressed;
	_keyPressed.reset();

	//
	// Clip the mouse coordinates, and compute _virtualMouse.x (and clip it, too)
	//
	if (_mouse.x < 0)
		_mouse.x = 0;
	if (_mouse.x > _screenWidth-1)
		_mouse.x = _screenWidth-1;
	if (_mouse.y < 0)
		_mouse.y = 0;
	if (_mouse.y > _screenHeight-1)
		_mouse.y = _screenHeight-1;

	VirtScreen *vs = &_virtscr[kMainVirtScreen];
	_virtualMouse.x = _mouse.x + vs->xstart;
	_virtualMouse.y = _mouse.y - vs->topline;
	if (_game.version >= 7)
		_virtualMouse.y += _screenTop;

	if (_virtualMouse.y < 0)
		_virtualMouse.y = -1;
	if (_virtualMouse.y >= vs->h)
		_virtualMouse.y = -1;

	//
	// Determine the mouse button state.
	//
	_mouseAndKeyboardStat = 0;

	if ((_leftBtnPressed & msClicked) && (_rightBtnPressed & msClicked) && _game.version >= 4) {
		// Pressing both mouse buttons is treated as if you pressed
		// the cutscene exit key (ESC) in V4+ games. That mimicks
		// the behavior of the original engine where pressing both
		// mouse buttons also skips the current cutscene.
		_mouseAndKeyboardStat = SCUMM_KEY_ESCAPE;
	} else if ((_rightBtnPressed & msClicked) && (_game.version <= 3 && _game.id != GID_LOOM)) {
		// Pressing right mouse button is treated as if you pressed
		// the cutscene exit key (ESC) in V0-V3 games. That mimicks
		// the behavior of the original engine where pressing right
		// mouse button also skips the current cutscene.
		_mouseAndKeyboardStat = SCUMM_KEY_ESCAPE;
	} else if (_leftBtnPressed & msClicked) {
		_mouseAndKeyboardStat = MBS_LEFT_CLICK;
	} else if (_rightBtnPressed & msClicked) {
		_mouseAndKeyboardStat = MBS_RIGHT_CLICK;
	}

	if (_game.version >= 6) {
		VAR(VAR_LEFTBTN_HOLD) = (_leftBtnPressed & msDown) != 0;
		VAR(VAR_RIGHTBTN_HOLD) = (_rightBtnPressed & msDown) != 0;

		if (_game.heversion >= 72) {
			// HE72 introduced a flag for whether or not this is a click
			// or the player is continuing to hold the button down.
			// 0x80 signifies that the button is continuing to be held down
			// Backyard Soccer needs this in order to function
			if (VAR(VAR_LEFTBTN_HOLD) && !(_leftBtnPressed & msClicked))
				VAR(VAR_LEFTBTN_HOLD) |= 0x80;

			if (VAR(VAR_RIGHTBTN_HOLD) && !(_rightBtnPressed & msClicked))
				VAR(VAR_RIGHTBTN_HOLD) |= 0x80;
		} else if (_game.version >= 7) {
			VAR(VAR_LEFTBTN_DOWN) = (_leftBtnPressed & msClicked) != 0;
			VAR(VAR_RIGHTBTN_DOWN) = (_rightBtnPressed & msClicked) != 0;
		}
	}

	_leftBtnPressed &= ~msClicked;
	_rightBtnPressed &= ~msClicked;

	if (_mouseAndKeyboardStat || (lastKeyHit.keycode == Common::KEYCODE_INVALID && lastKeyHit.ascii == 0))
		return;

	_mouseAndKeyboardStat = convertKey(lastKeyHit);

	if (_mouseAndKeyboardStat == SCUMM_KEY_RETURN &&
	    _cursor.state > 0 && _game.version <= 6) {
		_mouseAndKeyboardStat = MBS_LEFT_CLICK;
		return;
	} else if (_mouseAndKeyboardStat == SCUMM_KEY_TAB &&
		_cursor.state > 0 && _game.version <= 6 && _game.version >= 4) {
		_mouseAndKeyboardStat = MBS_RIGHT_CLICK;
		return;
	}

	processKeyboard();
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v8::processKeyboard() {
	// F1 (the trigger for the original save/load dialog) is mapped to F5
	if (!(_game.features & GF_DEMO) && _mouseAndKeyboardStat == SCUMM_KEY_F1) {
		_mouseAndKeyboardStat = SCUMM_KEY_F5;
	}

	// Alt-F5 should bring up the original save/load dialog, so map it to F1.
	if (!(_game.features & GF_DEMO) && _mouseAndKeyboardStat == SCUMM_KEY_ALT_F5) {
		_mouseAndKeyboardStat = SCUMM_KEY_F1;
	}

	// If a key script was specified (a V8 feature), and it's trigger
	// key was pressed, run it. Usually used to display the built-in menu.
	if (_keyScriptNo && (_keyScriptKey == _mouseAndKeyboardStat)) {
		runScript(_keyScriptNo, 0, 0, 0);
		return;
	}

	// Fall back to V7 behavior
	ScummEngine_v7::processKeyboard();
}

void ScummEngine_v7::processKeyboard() {
	// VAR_VERSION_KEY (usually ctrl-v) is used in COMI, Dig and FT to trigger
	// a version dialog, unless VAR_VERSION_KEY is set to 0. However, the COMI
	// version string is hard coded in the engine, hence we don't invoke
	// versionDialog for it. Dig/FT version strings are partly hard coded, too.
	if (_game.id != GID_CMI && _mouseAndKeyboardStat == VAR(VAR_VERSION_KEY)) {
		versionDialog();

	} else if (_mouseAndKeyboardStat == VAR(VAR_CUTSCENEEXIT_KEY)) {
		// Skip cutscene (or active SMUSH video).
		if (_smushActive) {
			if (_game.id == GID_FT)
				_insane->escapeKeyHandler();
			else
				_smushVideoShouldFinish = true;
			_skipVideo = true;
		} else {
			abortCutscene();
		}

	} else {
		// Fall back to V6 behavior
		ScummEngine_v6::processKeyboard();
	}
}
#endif

void ScummEngine_v6::processKeyboard() {
	if (_mouseAndKeyboardStat == SCUMM_KEY_CTRL_T) {
		_voiceMode++;
		if (_voiceMode > 2)
			_voiceMode = 0;

		switch (_voiceMode) {
		case 0:
			ConfMan.setBool("speech_mute", false);
			ConfMan.setBool("subtitles", false);
			messageDialog(_("Voice Only"));
			break;
		case 1:
			ConfMan.setBool("speech_mute", false);
			ConfMan.setBool("subtitles", true);
			messageDialog(_("Voice and Text"));
			break;
		case 2:
			ConfMan.setBool("speech_mute", true);
			ConfMan.setBool("subtitles", true);
			messageDialog(_("Text Display Only"));
			break;
		default:
			break;
		}

		// We need to sync the current sound settings here to make sure that
		// we actually update the mute state of speech properly.
		syncSoundSettings();

		return;
	}

	// Fall back to default behavior
	ScummEngine::processKeyboard();
}

void ScummEngine_v2::processKeyboard() {
	// F7 restarts immediately in the C64 demo.
	if (_game.platform == Common::kPlatformC64 && _game.features & GF_DEMO &&
	    _roomResource != 0x2D && _mouseAndKeyboardStat == SCUMM_KEY_F7) {
		restart();
		return;
	}

	// Fall back to default behavior
	ScummEngine::processKeyboard();

	// Prepare savegame for the original save/load dialog.
	if (isOriginalMenuKey()) {
		prepareSavegame();
		if (_game.id == GID_MANIAC && _game.version == 0) {
			runScript(2, 0, 0, 0);
		}
		if (_game.id == GID_MANIAC && _game.platform == Common::kPlatformNES) {
			runScript(163, 0, 0, 0);
		}
	}

	if (VAR_KEYPRESS != 0xFF && _mouseAndKeyboardStat) {
		if (_mouseAndKeyboardStat >= SCUMM_KEY_F1 && _mouseAndKeyboardStat <= SCUMM_KEY_F12) {
			// Convert F-Keys for V1/V2 games (they start at 1)
			VAR(VAR_KEYPRESS) = _mouseAndKeyboardStat - 314;
		} else {
			VAR(VAR_KEYPRESS) = _mouseAndKeyboardStat;
		}
	}
}

void ScummEngine_v3::processKeyboard() {
	// Fall back to default behavior
	ScummEngine::processKeyboard();

	// On F5 prepare savegame for the original save/load dialog.
	if (isOriginalMenuKey()) {
		prepareSavegame();
	}
}

void ScummEngine::processKeyboard() {
	// Enable the following five special keys conditionally:
	bool restartKeyEnabled = (VAR_RESTART_KEY == 0xFF || VAR(VAR_RESTART_KEY) != 0);
	bool pauseKeyEnabled = (VAR_PAUSE_KEY == 0xFF || VAR(VAR_PAUSE_KEY) != 0);
	bool talkstopKeyEnabled = (VAR_TALKSTOP_KEY == 0xFF || VAR(VAR_TALKSTOP_KEY) != 0);
	bool cutsceneExitKeyEnabled = (VAR_CUTSCENEEXIT_KEY == 0xFF || VAR(VAR_CUTSCENEEXIT_KEY) != 0);
	bool mainmenuKeyEnabled = (VAR_MAINMENU_KEY == 0xFF || VAR(VAR_MAINMENU_KEY) != 0);
	bool snapScrollKeyEnabled = (_game.version >= 2 && _game.version <= 4);

	// In FM-TOWNS games F8 / restart is always enabled
	if (_game.platform == Common::kPlatformFMTowns)
		restartKeyEnabled = true;

	// For games which use VAR_MAINMENU_KEY, disable the mainmenu key if
	// requested by the scripts. We make an exception for COMI (i.e.
	// forcefully always enable it there), as that always disables it.
	if (_game.id == GID_CMI)
		mainmenuKeyEnabled = true;

	// Display global main menu
	if (mainmenuKeyEnabled && isMainMenuKey() && !isOriginalMenuKey()) {
		if (VAR_SAVELOAD_SCRIPT != 0xFF && _currentRoom != 0)
			runScript(VAR(VAR_SAVELOAD_SCRIPT), 0, 0, 0);

		openMainMenuDialog();

		if (VAR_SAVELOAD_SCRIPT2 != 0xFF && _currentRoom != 0)
			runScript(VAR(VAR_SAVELOAD_SCRIPT2), 0, 0, 0);

	} else if (restartKeyEnabled && isRestartKey()) {
		confirmRestartDialog();
		// Reset the keyboard state to avoid triggering the script-based dialog.
		_mouseAndKeyboardStat = 0;

	} else if (restartKeyEnabled && isQuitKey()) {
		confirmExitDialog();

	} else if (pauseKeyEnabled && _mouseAndKeyboardStat == SCUMM_KEY_PAUSE) {
		pauseGame();

	} else if (talkstopKeyEnabled && _mouseAndKeyboardStat == SCUMM_KEY_TALK_STOP) {
		_talkDelay = 0;
		if (_sound->_sfxMode & 2)
			stopTalk();

	} else if (cutsceneExitKeyEnabled && isCutsceneExitKey()) {
		abortCutscene();

	} else if (snapScrollKeyEnabled && isSnapScrollKey()) {
		_snapScroll ^= 1;
		if (_snapScroll) {
			messageDialog(_("Horizontal Screen Snap"));
		} else {
			messageDialog(_("Horizontal Screen Scroll"));
		}

		if (VAR_CAMERA_FAST_X != 0xFF)
			VAR(VAR_CAMERA_FAST_X) = _snapScroll;

		// Change music volume
	} else if (_mouseAndKeyboardStat == SCUMM_KEY_MUSIC_VOLUME_DEC ||
	           _mouseAndKeyboardStat == SCUMM_KEY_MUSIC_VOLUME_INC) {
		int vol = ConfMan.getInt("music_volume") / 16;
		if (_mouseAndKeyboardStat == SCUMM_KEY_MUSIC_VOLUME_INC && vol < 16)
			vol++;
		else if (_mouseAndKeyboardStat == SCUMM_KEY_MUSIC_VOLUME_DEC && vol > 0)
			vol--;

		// Display the music volume
		ValueDisplayDialog dlg(this, _("Music Volume: "), 0, 16, vol,
		                       SCUMM_KEY_MUSIC_VOLUME_INC, SCUMM_KEY_MUSIC_VOLUME_DEC);
		vol = runDialog(dlg);

		vol *= 16;
		if (vol > Audio::Mixer::kMaxMixerVolume)
			vol = Audio::Mixer::kMaxMixerVolume;

		ConfMan.setInt("music_volume", vol);
		syncSoundSettings();

		// Change text speed
	} else if (_mouseAndKeyboardStat == SCUMM_KEY_TEXT_SPEED_DEC ||
	           _mouseAndKeyboardStat == SCUMM_KEY_TEXT_SPEED_INC) {
		if (_mouseAndKeyboardStat == SCUMM_KEY_TEXT_SPEED_INC && _defaultTalkDelay > 0)
			_defaultTalkDelay--;
		else if (_mouseAndKeyboardStat == SCUMM_KEY_TEXT_SPEED_DEC && _defaultTalkDelay < 9)
			_defaultTalkDelay++;

		// Display the talk speed
		ValueDisplayDialog dlg(this, _("Text Speed: "), 0, 9, 9 - _defaultTalkDelay,
		                       SCUMM_KEY_TEXT_SPEED_INC, SCUMM_KEY_TEXT_SPEED_DEC);
		_defaultTalkDelay = 9 - runDialog(dlg);

		// Save the new talkspeed value to ConfMan
		setTalkSpeed(_defaultTalkDelay);

		if (VAR_CHARINC != 0xFF)
			VAR(VAR_CHARINC) = _defaultTalkDelay;
	}
}

} // End of namespace Scumm
