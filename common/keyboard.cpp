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

#include "common/keyboard.h"
#include "common/util.h"

namespace Common {

#define UNDEFI 0

static const INT16hKeyMap g_int16h00h[] = {
	{ KEYCODE_ESCAPE       , 0x011B, 0x011B, 0x011B, UNDEFI },
	{ KEYCODE_1            , 0x0231, 0x0221, UNDEFI, 0x7800 },
	{ KEYCODE_2            , 0x0332, 0x0340, 0x0300, 0x7900 },
	{ KEYCODE_3            , 0x0433, 0x0423, UNDEFI, 0x7A00 },
	{ KEYCODE_4            , 0x0534, 0x0524, UNDEFI, 0x7B00 },
	{ KEYCODE_5            , 0x0635, 0x0625, UNDEFI, 0x7C00 },
	{ KEYCODE_6            , 0x0736, 0x075E, 0x071E, 0x7D00 },
	{ KEYCODE_7            , 0x0837, 0x0826, UNDEFI, 0x7E00 },
	{ KEYCODE_8            , 0x0938, 0x092A, UNDEFI, 0x7F00 },
	{ KEYCODE_9            , 0x0A39, 0x0A28, UNDEFI, 0x8000 },
	{ KEYCODE_0            , 0x0B30, 0x0B29, UNDEFI, 0x8100 },
	{ KEYCODE_MINUS        , 0x0C2D, 0x0C5F, 0x0C1F, 0x8200 },
	{ KEYCODE_EQUALS       , 0x0D3D, 0x0D2B, UNDEFI, 0x8300 },
	{ KEYCODE_BACKSPACE    , 0x0E08, 0x0E08, 0x0E7F, 0x0E00 },
	{ KEYCODE_TAB          , 0x0F09, 0x0F00, UNDEFI, UNDEFI },
	{ KEYCODE_q            , 0x1071, 0x1051, 0x1011, 0x1000 },
	{ KEYCODE_w            , 0x1177, 0x1157, 0x1117, 0x1100 },
	{ KEYCODE_e            , 0x1265, 0x1245, 0x1205, 0x1200 },
	{ KEYCODE_r            , 0x1372, 0x1352, 0x1312, 0x1300 },
	{ KEYCODE_t            , 0x1474, 0x1454, 0x1414, 0x1400 },
	{ KEYCODE_y            , 0x1579, 0x1559, 0x1519, 0x1500 },
	{ KEYCODE_u            , 0x1675, 0x1655, 0x1615, 0x1600 },
	{ KEYCODE_i            , 0x1769, 0x1749, 0x1709, 0x1700 },
	{ KEYCODE_o            , 0x186F, 0x184F, 0x180F, 0x1800 },
	{ KEYCODE_p            , 0x1970, 0x1950, 0x1910, 0x1900 },
	{ KEYCODE_LEFTBRACKET  , 0x1A5B, 0x1A7B, 0x1A1B, UNDEFI },
	{ KEYCODE_RIGHTBRACKET , 0x1B5D, 0x1B7D, 0x1B1D, UNDEFI },
	{ KEYCODE_RETURN       , 0x1C0D, 0x1C0D, 0x1C0A, UNDEFI },
	{ KEYCODE_LCTRL        , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_a            , 0x1E61, 0x1E41, 0x1E01, 0x1E00 },
	{ KEYCODE_s            , 0x1F73, 0x1F53, 0x1F13, 0x1F00 },
	{ KEYCODE_d            , 0x2064, 0x2044, 0x2004, 0x2000 },
	{ KEYCODE_f            , 0x2166, 0x2146, 0x2106, 0x2100 },
	{ KEYCODE_g            , 0x2267, 0x2247, 0x2207, 0x2200 },
	{ KEYCODE_h            , 0x2368, 0x2348, 0x2308, 0x2300 },
	{ KEYCODE_j            , 0x246A, 0x244A, 0x240A, 0x2400 },
	{ KEYCODE_k            , 0x256B, 0x254B, 0x250B, 0x2500 },
	{ KEYCODE_l            , 0x266C, 0x264C, 0x260C, 0x2600 },
	{ KEYCODE_SEMICOLON    , 0x273B, 0x273A, UNDEFI, UNDEFI },
	{ KEYCODE_QUOTE        , 0x2827, 0x2822, UNDEFI, UNDEFI },
	{ KEYCODE_BACKQUOTE    , 0x2960, 0x297E, UNDEFI, UNDEFI },
	{ KEYCODE_LSHIFT       , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_BACKSLASH    , 0x2B5C, 0x2B7C, 0x2B1C, UNDEFI },
	{ KEYCODE_z            , 0x2C7A, 0x2C5A, 0x2C1A, 0x2C00 },
	{ KEYCODE_x            , 0x2D78, 0x2D58, 0x2D18, 0x2D00 },
	{ KEYCODE_c            , 0x2E63, 0x2E43, 0x2E03, 0x2E00 },
	{ KEYCODE_v            , 0x2F76, 0x2F56, 0x2F16, 0x2F00 },
	{ KEYCODE_b            , 0x3062, 0x3042, 0x3002, 0x3000 },
	{ KEYCODE_n            , 0x316E, 0x314E, 0x310E, 0x3100 },
	{ KEYCODE_m            , 0x326D, 0x324D, 0x320E, 0x3200 },
	{ KEYCODE_COMMA        , 0x332C, 0x333C, UNDEFI, UNDEFI },
	{ KEYCODE_PERIOD       , 0x342E, 0x343E, UNDEFI, UNDEFI },
	{ KEYCODE_SLASH        , 0x352F, 0x353F, UNDEFI, UNDEFI },
	{ KEYCODE_RSHIFT       , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_KP_MULTIPLY  , 0x372A, 0x372A, UNDEFI, UNDEFI },
	{ KEYCODE_LALT         , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_SPACE        , 0x3920, 0x3920, 0x3920, 0x3920 },
	{ KEYCODE_CAPSLOCK     , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_F1           , 0x3B00, 0x5400, 0x5E00, 0x6800 },
	{ KEYCODE_F2           , 0x3C00, 0x5500, 0x5F00, 0x6900 },
	{ KEYCODE_F3           , 0x3D00, 0x5600, 0x6000, 0x6A00 },
	{ KEYCODE_F4           , 0x3E00, 0x5700, 0x6100, 0x6B00 },
	{ KEYCODE_F5           , 0x3F00, 0x5800, 0x6200, 0x6C00 },
	{ KEYCODE_F6           , 0x4000, 0x5900, 0x6300, 0x6D00 },
	{ KEYCODE_F7           , 0x4100, 0x6000, 0x6400, 0x6E00 },
	{ KEYCODE_F8           , 0x4200, 0x6100, 0x6500, 0x7F00 },
	{ KEYCODE_F9           , 0x4300, 0x6200, 0x6600, 0x7000 },
	{ KEYCODE_F10          , 0x4400, 0x6300, 0x6700, 0x7100 },
	{ KEYCODE_NUMLOCK      , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_SCROLLOCK    , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_KP7          , 0x4700, 0x4737, 0x7700, UNDEFI },
	{ KEYCODE_KP8          , 0x4800, 0x4838, UNDEFI, UNDEFI },
	{ KEYCODE_KP9          , 0x4900, 0x4939, 0x8400, UNDEFI },
	{ KEYCODE_KP_MINUS     , 0x4A2D, 0x4A2D, UNDEFI, UNDEFI },
	{ KEYCODE_KP4          , 0x4B00, 0x4B34, 0x7300, UNDEFI },
	{ KEYCODE_KP5          , UNDEFI, 0x4C35, UNDEFI, UNDEFI },
	{ KEYCODE_KP6          , 0x4D00, 0x4D36, 0x7400, UNDEFI },
	{ KEYCODE_KP_PLUS      , 0x4E2B, 0x4E2B, UNDEFI, UNDEFI },
	{ KEYCODE_KP1          , 0x4F00, 0x4F31, 0x7500, UNDEFI },
	{ KEYCODE_KP2          , 0x5000, 0x5032, UNDEFI, UNDEFI },
	{ KEYCODE_KP3          , 0x5100, 0x5133, 0x7600, UNDEFI },
	{ KEYCODE_KP0          , 0x5200, 0x5230, UNDEFI, UNDEFI },
	{ KEYCODE_KP_PERIOD    , 0x5300, 0x532E, UNDEFI, UNDEFI },
	{ KEYCODE_PRINT        , UNDEFI, UNDEFI, 0x7200, UNDEFI },
	{ KEYCODE_PAUSE        , UNDEFI, UNDEFI, 0x0000, UNDEFI },
	{ KEYCODE_F11          , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_F12          , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_HOME         , 0x4700, 0x4700, 0x7700, UNDEFI },
	{ KEYCODE_UP           , 0x4800, 0x4800, UNDEFI, UNDEFI },
	{ KEYCODE_PAGEUP       , 0x4900, 0x4900, 0x8400, UNDEFI },
	{ KEYCODE_LEFT         , 0x4B00, 0x4B00, 0x7300, UNDEFI },
	{ KEYCODE_RIGHT        , 0x4D00, 0x4D00, 0x7400, UNDEFI },
	{ KEYCODE_END          , 0x4F00, 0x4F00, 0x7500, UNDEFI },
	{ KEYCODE_DOWN         , 0x5000, 0x5000, UNDEFI, UNDEFI },
	{ KEYCODE_PAGEDOWN     , 0x5100, 0x5100, 0x7600, UNDEFI },
	{ KEYCODE_INSERT       , 0x5200, 0x5200, UNDEFI, UNDEFI },
	{ KEYCODE_DELETE       , 0x5300, 0x5300, UNDEFI, UNDEFI },
	{ KEYCODE_KP_DIVIDE    , 0x352F, 0x352F, UNDEFI, UNDEFI },
	{ KEYCODE_KP_ENTER     , 0x1C0D, 0x1C0D, 0x1C0A, UNDEFI },
	{ KEYCODE_RALT         , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_RCTRL        , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_INVALID      , UNDEFI, UNDEFI, UNDEFI, UNDEFI }
};

static const INT16hKeyMap g_int16h10h[] = {
	{ KEYCODE_ESCAPE       , 0x011B, 0x011B, 0x011B, 0x0100 },
	{ KEYCODE_1            , 0x0231, 0x0221, UNDEFI, 0x7800 },
	{ KEYCODE_2            , 0x0332, 0x0340, 0x0300, 0x7900 },
	{ KEYCODE_3            , 0x0433, 0x0423, UNDEFI, 0x7A00 },
	{ KEYCODE_4            , 0x0534, 0x0524, UNDEFI, 0x7B00 },
	{ KEYCODE_5            , 0x0635, 0x0625, UNDEFI, 0x7C00 },
	{ KEYCODE_6            , 0x0736, 0x075E, 0x071E, 0x7D00 },
	{ KEYCODE_7            , 0x0837, 0x0826, UNDEFI, 0x7E00 },
	{ KEYCODE_8            , 0x0938, 0x092A, UNDEFI, 0x7F00 },
	{ KEYCODE_9            , 0x0A39, 0x0A28, UNDEFI, 0x8000 },
	{ KEYCODE_0            , 0x0B30, 0x0B29, UNDEFI, 0x8100 },
	{ KEYCODE_MINUS        , 0x0C2D, 0x0C5F, 0x0C1F, 0x8200 },
	{ KEYCODE_EQUALS       , 0x0D3D, 0x0D2B, UNDEFI, 0x8300 },
	{ KEYCODE_BACKSPACE    , 0x0E08, 0x0E08, 0x0E7F, 0x0E00 },
	{ KEYCODE_TAB          , 0x0F09, 0x0F00, 0x9400, 0xA500 },
	{ KEYCODE_q            , 0x1071, 0x1051, 0x1011, 0x1000 },
	{ KEYCODE_w            , 0x1177, 0x1157, 0x1117, 0x1100 },
	{ KEYCODE_e            , 0x1265, 0x1245, 0x1205, 0x1200 },
	{ KEYCODE_r            , 0x1372, 0x1352, 0x1312, 0x1300 },
	{ KEYCODE_t            , 0x1474, 0x1454, 0x1414, 0x1400 },
	{ KEYCODE_y            , 0x1579, 0x1559, 0x1519, 0x1500 },
	{ KEYCODE_u            , 0x1675, 0x1655, 0x1615, 0x1600 },
	{ KEYCODE_i            , 0x1769, 0x1749, 0x1709, 0x1700 },
	{ KEYCODE_o            , 0x186F, 0x184F, 0x180F, 0x1800 },
	{ KEYCODE_p            , 0x1970, 0x1950, 0x1910, 0x1900 },
	{ KEYCODE_LEFTBRACKET  , 0x1A5B, 0x1A7B, 0x1A1B, 0x1A00 },
	{ KEYCODE_RIGHTBRACKET , 0x1B5D, 0x1B7D, 0x1B1D, 0x1B00 },
	{ KEYCODE_RETURN       , 0x1C0D, 0x1C0D, 0x1C0A, 0x1C00 },
	{ KEYCODE_LCTRL        , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_a            , 0x1E61, 0x1E41, 0x1E01, 0x1E00 },
	{ KEYCODE_s            , 0x1F73, 0x1F53, 0x1F13, 0x1F00 },
	{ KEYCODE_d            , 0x2064, 0x2044, 0x2004, 0x2000 },
	{ KEYCODE_f            , 0x2166, 0x2146, 0x2106, 0x2100 },
	{ KEYCODE_g            , 0x2267, 0x2247, 0x2207, 0x2200 },
	{ KEYCODE_h            , 0x2368, 0x2348, 0x2308, 0x2300 },
	{ KEYCODE_j            , 0x246A, 0x244A, 0x240A, 0x2400 },
	{ KEYCODE_k            , 0x256B, 0x254B, 0x250B, 0x2500 },
	{ KEYCODE_l            , 0x266C, 0x264C, 0x260C, 0x2600 },
	{ KEYCODE_SEMICOLON    , 0x273B, 0x273A, UNDEFI, 0x2700 },
	{ KEYCODE_QUOTE        , 0x2827, 0x2822, UNDEFI, 0x2800 },
	{ KEYCODE_BACKQUOTE    , 0x2960, 0x297E, UNDEFI, 0x2900 },
	{ KEYCODE_LSHIFT       , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_BACKSLASH    , 0x2B5C, 0x2B7C, 0x2B1C, 0x2B00 },
	{ KEYCODE_z            , 0x2C7A, 0x2C5A, 0x2C1A, 0x2C00 },
	{ KEYCODE_x            , 0x2D78, 0x2D58, 0x2D18, 0x2D00 },
	{ KEYCODE_c            , 0x2E63, 0x2E43, 0x2E03, 0x2E00 },
	{ KEYCODE_v            , 0x2F76, 0x2F56, 0x2F16, 0x2F00 },
	{ KEYCODE_b            , 0x3062, 0x3042, 0x3002, 0x3000 },
	{ KEYCODE_n            , 0x316E, 0x314E, 0x310E, 0x3100 },
	{ KEYCODE_m            , 0x326D, 0x324D, 0x320D, 0x3200 },
	{ KEYCODE_COMMA        , 0x332C, 0x333C, UNDEFI, 0x3300 },
	{ KEYCODE_PERIOD       , 0x342E, 0x343E, UNDEFI, 0x3400 },
	{ KEYCODE_SLASH        , 0x352F, 0x353F, UNDEFI, 0x3500 },
	{ KEYCODE_RSHIFT       , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_KP_MULTIPLY  , 0x372A, 0x372A, 0x9600, 0x3700 },
	{ KEYCODE_LALT         , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_SPACE        , 0x3920, 0x3920, 0x3920, 0x3920 },
	{ KEYCODE_CAPSLOCK     , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_F1           , 0x3B00, 0x5400, 0x5E00, 0x6800 },
	{ KEYCODE_F2           , 0x3C00, 0x5500, 0x5F00, 0x6900 },
	{ KEYCODE_F3           , 0x3D00, 0x5600, 0x6000, 0x6A00 },
	{ KEYCODE_F4           , 0x3E00, 0x5700, 0x6100, 0x6B00 },
	{ KEYCODE_F5           , 0x3F00, 0x5800, 0x6200, 0x6C00 },
	{ KEYCODE_F6           , 0x4000, 0x5900, 0x6300, 0x6D00 },
	{ KEYCODE_F7           , 0x4100, 0x6000, 0x6400, 0x6E00 },
	{ KEYCODE_F8           , 0x4200, 0x6100, 0x6500, 0x7F00 },
	{ KEYCODE_F9           , 0x4300, 0x6200, 0x6600, 0x7000 },
	{ KEYCODE_F10          , 0x4400, 0x6300, 0x6700, 0x7100 },
	{ KEYCODE_NUMLOCK      , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_SCROLLOCK    , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_KP7          , 0x4700, 0x4737, 0x7700, UNDEFI },
	{ KEYCODE_KP8          , 0x4800, 0x4838, 0x8D00, UNDEFI },
	{ KEYCODE_KP9          , 0x4900, 0x4939, 0x8400, UNDEFI },
	{ KEYCODE_KP_MINUS     , 0x4A2D, 0x4A2D, 0x8E00, 0x4A00 },
	{ KEYCODE_KP4          , 0x4B00, 0x4B34, 0x7300, UNDEFI },
	{ KEYCODE_KP5          , 0x4C00, 0x4C35, 0x8F00, UNDEFI },
	{ KEYCODE_KP6          , 0x4D00, 0x4D36, 0x7400, UNDEFI },
	{ KEYCODE_KP_PLUS      , 0x4E2B, 0x4E2B, 0x9000, 0x4E00 },
	{ KEYCODE_KP1          , 0x4F00, 0x4F31, 0x7500, UNDEFI },
	{ KEYCODE_KP2          , 0x5000, 0x5032, 0x9100, UNDEFI },
	{ KEYCODE_KP3          , 0x5100, 0x5133, 0x7600, UNDEFI },
	{ KEYCODE_KP0          , 0x5200, 0x5230, 0x9200, UNDEFI },
	{ KEYCODE_KP_PERIOD    , 0x5300, 0x532E, 0x9300, UNDEFI },
	{ KEYCODE_PRINT        , UNDEFI, UNDEFI, 0x7200, UNDEFI },
	{ KEYCODE_PAUSE        , UNDEFI, UNDEFI, 0x0000, UNDEFI },
	{ KEYCODE_F11          , 0x8500, 0x8700, 0x8900, 0x8B00 },
	{ KEYCODE_F12          , 0x8600, 0x8800, 0x8A00, 0x8C00 },
	{ KEYCODE_HOME         , 0x47E0, 0x47E0, 0x77E0, 0x9700 },
	{ KEYCODE_UP           , 0x48E0, 0x48E0, 0x8DE0, 0x9800 },
	{ KEYCODE_PAGEUP       , 0x49E0, 0x49E0, 0x84E0, 0x9900 },
	{ KEYCODE_LEFT         , 0x4BE0, 0x4BE0, 0x73E0, 0x9B00 },
	{ KEYCODE_RIGHT        , 0x4DE0, 0x4DE0, 0x74E0, 0x9D00 },
	{ KEYCODE_END          , 0x4FE0, 0x4FE0, 0x75E0, 0x9F00 },
	{ KEYCODE_DOWN         , 0x50E0, 0x50E0, 0x91E0, 0xA000 },
	{ KEYCODE_PAGEDOWN     , 0x51E0, 0x51E0, 0x76E0, 0xA100 },
	{ KEYCODE_INSERT       , 0x52E0, 0x52E0, 0x92E0, 0xA200 },
	{ KEYCODE_DELETE       , 0x53E0, 0x53E0, 0x93E0, 0xA300 },
	{ KEYCODE_KP_DIVIDE    , 0xE02F, 0xE02F, 0x9500, 0xA400 },
	{ KEYCODE_KP_ENTER     , 0xE00D, 0xE00D, 0xE00A, 0xA600 },
	{ KEYCODE_RALT         , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_RCTRL        , UNDEFI, UNDEFI, UNDEFI, UNDEFI },
	{ KEYCODE_INVALID      , UNDEFI, UNDEFI, UNDEFI, UNDEFI }
};

uint16 KeyState::getINT16h00hKey(const CodePage page) const {
	return mapKeyStateToINT16hKey(g_int16h00h, page);
}

uint16 KeyState::getINT16h10hKey(const CodePage page) const {
	return mapKeyStateToINT16hKey(g_int16h10h, page);
}

byte KeyState::getINT16h00hCharacter(const CodePage page) const {
	return mapKeyStateToINT16hKey(g_int16h00h, page) & 0xFF;
}

byte KeyState::getINT16h10hCharacter(const CodePage page) const {
	return mapKeyStateToINT16hKey(g_int16h10h, page) & 0xFF;
}

uint16 KeyState::mapKeyStateToINT16hKey(const INT16hKeyMap *mapPtr, const CodePage page) const {
	const bool keypad = (keycode >= KEYCODE_KP0 && keycode <= KEYCODE_KP_PERIOD);
	const uint32 *const conversionTable = getCodePageConversionTable(page);
	uint16 key = 0;
	byte f = flags;
	if ((f & KBD_NUM) && keypad)
		f ^= KBD_SHIFT;
	if ((f & KBD_CAPS) && isAlpha(keycode))
		f ^= KBD_SHIFT;

	while (mapPtr->keycode) {
		if (mapPtr->keycode == keycode) {
			if (f & KBD_ALT) {
				key = mapPtr->alt;
				break;
			} else if (f & KBD_CTRL) {
				key = mapPtr->ctrl;
				break;
			} else if (f & KBD_SHIFT) {
				key = mapPtr->shift;
				break;
			} else {
				key = mapPtr->normal;
				break;
			}
		}
		mapPtr++;
	}

	/**
	 * We need the Unicode value from SDL for locale and layout-dependent
	 * characters. However we need use it gingerly; control characters are not
	 * produced by SDL, and the backends will definitely not give us the caret
	 * notation control codes. Instead some backends may give us characters
	 * irrespective of modifier keys where the engines would not have expected
	 * them. We will avoid remapping the aforementioned keys. The Unicode
	 * character is mapped to the desired code page before replacing the lower
	 * byte of the 16-bit key. The scancode will most likely not correspond to
	 * the character anymore, but this is not an issue as the engines would
	 * never have used these locale-dependent keys (only the character).
	 */
	if (ascii &&
	    !(flags & KBD_CTRL) &&
	    !((keycode == ascii) && (flags & KBD_ALT)) &&
	    (isPrint(keycode) || keycode == KEYCODE_INVALID)) {

		key &= 0xFF00;
		if (conversionTable) {
			for (size_t i = 0; i < 256; i++) {
				if (conversionTable[i] == ascii) {
					key |= (uint16)i;
					break;
				}
			}
		}
	}

	return key;
}

} // End of namespace Common
