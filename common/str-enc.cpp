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

#include "common/codepages.h"
#include "common/encoding.h"
#include "common/str.h"
#include "common/ustr.h"
#include "common/util.h"

namespace Common {

// //TODO: This is a quick and dirty converter. Refactoring needed:
// 1. This version is unsafe! There are no checks for end of buffer
//    near i++ operations.
// 2. Original version has an option for performing strict / nonstrict
//    conversion for the 0xD800...0xDFFF interval
// 3. Original version returns a result code. This version does NOT
//    insert 'FFFD' on errors & does not inform caller on any errors
//
// More comprehensive one lives in wintermute/utils/convert_utf.cpp
void String::decodeUTF8(U32String &dst) const {
	// The String class, and therefore the Font class as well, assume one
	// character is one byte, but in this case it's actually an UTF-8
	// string with up to 4 bytes per character. To work around this,
	// convert it to an U32String before drawing it, because our Font class
	// can handle that.
	uint i = 0;
	while (i < _size) {
		uint32 chr = 0;
		uint num = 1;

		if ((_str[i] & 0xF8) == 0xF0) {
			num = 4;
		} else if ((_str[i] & 0xF0) == 0xE0) {
			num = 3;
		} else if ((_str[i] & 0xE0) == 0xC0) {
			num = 2;
		}

		if (i - _size >= num) {
			switch (num) {
			case 4:
				chr |= (_str[i++] & 0x07) << 18;
				chr |= (_str[i++] & 0x3F) << 12;
				chr |= (_str[i++] & 0x3F) << 6;
				chr |= (_str[i++] & 0x3F);
				break;

			case 3:
				chr |= (_str[i++] & 0x0F) << 12;
				chr |= (_str[i++] & 0x3F) << 6;
				chr |= (_str[i++] & 0x3F);
				break;

			case 2:
				chr |= (_str[i++] & 0x1F) << 6;
				chr |= (_str[i++] & 0x3F);
				break;

			default:
				chr = (_str[i++] & 0x7F);
				break;
			}
		} else {
			break;
		}

		dst += chr;
	}
}

// //TODO: This is a quick and dirty converter. Refactoring needed:
// 1. Original version is more effective.
//    This version features buffer = (char)(...) + buffer; pattern that causes
//    unnecessary copying and reallocations, original code works with raw bytes
// 2. Original version has an option for performing strict / nonstrict
//    conversion for the 0xD800...0xDFFF interval
// 3. Original version returns a result code. This version inserts '0xFFFD' if
//    character does not fit in 4 bytes & does not inform caller on any errors
//
// More comprehensive one lives in wintermute/utils/convert_utf.cpp
void U32String::encodeUTF8(String &dst) const {
	static const uint8 firstByteMark[5] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0 };
	char writingBytes[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

	uint i = 0;
	while (i < _size) {
		unsigned short bytesToWrite = 0;
		const uint32 byteMask = 0xBF;
		const uint32 byteMark = 0x80;

		uint32 ch = _str[i++];
		if (ch < (uint32)0x80) {
			bytesToWrite = 1;
		} else if (ch < (uint32)0x800) {
			bytesToWrite = 2;
		} else if (ch < (uint32)0x10000) {
			bytesToWrite = 3;
		} else if (ch <= 0x0010FFFF) {
			bytesToWrite = 4;
		} else {
			bytesToWrite = 3;
			ch = 0x0000FFFD;
		}

		char *pBytes = writingBytes + (4 - bytesToWrite);

		switch (bytesToWrite) {
		case 4:
			pBytes[3] = (char)((ch | byteMark) & byteMask);
			ch >>= 6;
			// fallthrough
		case 3:
			pBytes[2] = (char)((ch | byteMark) & byteMask);
			ch >>= 6;
			// fallthrough
		case 2:
			pBytes[1] = (char)((ch | byteMark) & byteMask);
			ch >>= 6;
			// fallthrough
		case 1:
			pBytes[0] = (char)(ch | firstByteMark[bytesToWrite]);
			break;
		default:
			break;
		}

		dst += pBytes;
	}
}

/* This array must match the enum defined in codepages.h */
static char const *const g_codePageMap[] = {
	"UTF-8",        /* kUtf8 */
    "CP437",        /* kCodePage437 */
    "CP850",        /* kCodePage850 */
    "CP866",        /* kCodePage866 */
    "MS932",        /* kWindows932 */
    "MSCP949",      /* kWindows949 */
    "CP950",        /* kWindows950 */
	"WINDOWS-1250", /* kWindows1250 */
	"WINDOWS-1251", /* kWindows1251 */
	"WINDOWS-1252", /* kWindows1252 */
	"WINDOWS-1253", /* kWindows1253 */
	"WINDOWS-1254", /* kWindows1254 */
	"WINDOWS-1255", /* kWindows1255 */
	"WINDOWS-1256", /* kWindows1256 */
	"WINDOWS-1257"  /* kWindows1257 */
};

void String::decodeOneByte(U32String &dst, CodePage page) const {
	const uint32 *const conversionTable = getCodePageConversionTable(page);
	if (!conversionTable) {
		return;
	}

	for (uint i = 0; i < _size; ++i) {
		byte index = _str[i];
		dst += conversionTable[index];
	}
}

U32String String::decode(CodePage page) const {
	if (page == kCodePageInvalid ||
			page >= ARRAYSIZE(g_codePageMap)) {
		error("Invalid codepage");
	}
	char *result = Encoding::convert("UTF-32", g_codePageMap[page], *this);
	if (result) {
		U32String unicodeString((uint32 *)result);
		free(result);
		return unicodeString;
	}

	U32String unicodeString;
	if (page == kUtf8) {
		decodeUTF8(unicodeString);
	} else {
		decodeOneByte(unicodeString, page);
	}
	return unicodeString;
}



void U32String::encodeOneByte(String &dst, CodePage page) const {
	const uint32 *const conversionTable = getCodePageConversionTable(page);
	if (!conversionTable) {
		return;
	}

	for (uint i = 0; i < _size; ++i) {
		for (uint j = 0; j < 256; ++j) {
			if (conversionTable[j] == _str[i]) {
				dst += (char)j;
				break;
			}
		}
	}
}


String U32String::encode(CodePage page) const {
	if (page == kCodePageInvalid ||
			page >= ARRAYSIZE(g_codePageMap)) {
		error("Invalid codepage");
	}
	char *result = Encoding::convert(g_codePageMap[page], *this);
	if (result) {
		// Encodings in CodePage all use '\0' as string ending
		// That would be problematic if CodePage has UTF-16 or UTF-32
		String string(result);
		free(result);
		return string;
	}

	String string;
	if (page == kUtf8) {
		encodeUTF8(string);
	} else {
		encodeOneByte(string, page);
	}
	return string;
}



U32String convertToU32String(const char *str, CodePage page) {
	return String(str).decode(page);
}

U32String convertUtf8ToUtf32(const String &str) {
	return str.decode(kUtf8);
}

String convertFromU32String(const U32String &string, CodePage page) {
	return string.encode(page);
}

String convertUtf32ToUtf8(const U32String &u32str) {
	return u32str.encode(kUtf8);
}

} // End of namespace Common
