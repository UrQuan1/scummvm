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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/codepages.h"
#include "common/encoding.h"
#include "common/textconsole.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/endian.h"
#include <errno.h>

#ifdef USE_ICONV

#include <iconv.h>

#endif // USE_ICONV

namespace Common {

String addUtfEndianness(const String &str) {
	if (str.equalsIgnoreCase("utf-16") || str.equalsIgnoreCase("utf-32")) {
#ifdef SCUMM_BIG_ENDIAN
		return str + "BE";
#else
		return str + "LE";
#endif
	} else
		return String(str);
}

Encoding::Encoding(const String &to, const String &from)
	: _to(to)
	, _from(from) {
}

char *Encoding::switchEndian(const char *string, int length, int bitCount) {
	assert(bitCount % 8 == 0);
	assert(length % (bitCount / 8) == 0);
	char *newString = (char *)malloc(length);
	if (!newString) {
		warning("Could not allocate memory for string conversion");
		return nullptr;
	}
	if (bitCount == 16) {
		int characterCount = length / 2;
		for(int i = 0; i < characterCount ; i++)
			((uint16 *)newString)[i] = SWAP_BYTES_16(((const uint16 *)string)[i]);
		return newString;
	} else if (bitCount == 32) {
		int characterCount = length / 4;
		for(int i = 0; i < characterCount ; i++)
			((uint32 *)newString)[i] = SWAP_BYTES_32(((const uint32 *)string)[i]);
		return newString;
	} else {
		free(newString);
		return nullptr;
	}
}

char *Encoding::convert(const char *string, size_t size) {
	return convertWithTransliteration(_to, _from, string, size);
}

char *Encoding::convert(const String &to, const String &from, const char *string, size_t size) {
	return convertWithTransliteration(to, from, string, size);
}

char *Encoding::convertWithTransliteration(const String &to, const String &from, const char *string, size_t length) {
	if (from.equalsIgnoreCase(to)) {
		// don't convert, just copy the string and return it
		char *result = (char *)calloc(sizeof(char), length + 4);

		if (!result) {
			warning("Could not allocate memory for string conversion");
			return nullptr;
		}

		memcpy(result, string, length);
		return result;
	}

	if ((addUtfEndianness(to).equalsIgnoreCase("utf-16be") &&
			addUtfEndianness(from).equalsIgnoreCase("utf-16le")) ||
			(addUtfEndianness(to).equalsIgnoreCase("utf-16le") &&
			addUtfEndianness(from).equalsIgnoreCase("utf-16be")) ||
			(addUtfEndianness(to).equalsIgnoreCase("utf-32be") &&
			addUtfEndianness(from).equalsIgnoreCase("utf-32le")) ||
			(addUtfEndianness(to).equalsIgnoreCase("utf-32le") &&
			addUtfEndianness(from).equalsIgnoreCase("utf-32be"))) {
		// The encoding is the same, we just need to switch the endianness
		if (to.hasPrefixIgnoreCase("utf-16"))
			return switchEndian(string, length, 16);
		else
			return switchEndian(string, length, 32);
	}

	char *newString = nullptr;
	String newFrom = from;
	size_t newLength = length;

	if (from.equalsIgnoreCase("iso-8859-5") &&
			!to.hasPrefixIgnoreCase("utf")) {
		// There might be some cyrillic characters, which need to be transliterated.
		newString = transliterateCyrillic(string);

		if (!newString)
			return nullptr;

		newFrom = "ASCII";
	}

	if (from.hasPrefixIgnoreCase("utf") &&
			!to.hasPrefixIgnoreCase("utf") &&
			!to.equalsIgnoreCase("iso-8859-5")) {
		// There might be some cyrillic characters, which need to be transliterated.
		char *tmpString;
		if (from.hasPrefixIgnoreCase("utf-32")) {
			tmpString = nullptr;
		} else {
			tmpString = conversion("UTF-32", from, string, length);
			if (!tmpString)
				return nullptr;
			// find out the length in bytes of the tmpString
			int i;

			for (i = 0; ((const uint32 *)tmpString)[i]; i++)
				;

			newLength = i * 4;
			newFrom = "UTF-32";
		}

		if (tmpString != nullptr) {
			newString = (char *)transliterateUTF32((const uint32 *)tmpString, newLength);
			free(tmpString);
		} else {
			newString = (char *)transliterateUTF32((const uint32 *)string, newLength);
		}

		if (!newString)
			return nullptr;
	}

	char *result;
	if (newString != nullptr) {
		result = conversion(to, newFrom, newString, newLength);
		free(newString);
	} else {
		result = conversion(to, newFrom, string, newLength);
	}

	return result;
}

char *Encoding::conversion(const String &to, const String &from, const char *string, size_t length) {
	char *result = nullptr;
#ifdef USE_ICONV
	result = convertIconv(addUtfEndianness(to).c_str(), addUtfEndianness(from).c_str(), string, length);
#endif // USE_ICONV
	if (result == nullptr)
		result = g_system->convertEncoding(addUtfEndianness(to).c_str(),
				addUtfEndianness(from).c_str(), string, length);

	if (result == nullptr) {
		result = convertConversionTable(addUtfEndianness(to).c_str(), addUtfEndianness(from).c_str(), string, length);
	}

	return result;
}

char *Encoding::convertIconv(const char *to, const char *from, const char *string, size_t length) {
#ifdef USE_ICONV

	String toTranslit = String(to) + "//TRANSLIT";
	iconv_t iconvHandle = iconv_open(toTranslit.c_str(), from);

	if (iconvHandle == (iconv_t)-1)
		return nullptr;

	size_t inSize = length;
	size_t stringSize = inSize > 4 ? inSize : 4;
	size_t outSize = stringSize;


#ifdef ICONV_USES_CONST
	const char *src = string;
#else
	char *src = new char[length];
	char *originalSrc = src;
	memcpy(src, string, length);
#endif // ICONV_USES_CONST

	char *buffer = (char *)calloc(sizeof(char), stringSize);
	if (!buffer) {
#ifndef ICONV_USES_CONST
		delete[] originalSrc;
#endif // ICONV_USES_CONST
		iconv_close(iconvHandle);
		warning ("Cannot allocate memory for converting string");
		return nullptr;
	}
	char *dst = buffer;
	bool error = false;

	while (true) {
		if (iconv(iconvHandle, &src, &inSize, &dst, &outSize) == ((size_t)-1)) {
			// from SDL's implementation of SDL_iconv_string (slightly altered)
			if (errno == E2BIG) {
				char *oldString = buffer;
				stringSize *= 2;
				buffer = (char *)realloc(buffer, stringSize);
				if (!buffer) {
					warning("Cannot allocate memory for converting string");
					error = true;
					break;
				}
				dst = buffer + (dst - oldString);
				outSize = stringSize - (dst - buffer);
				memset(dst, 0, outSize);
			} else {
				error = true;
				break;
			}
		} else {
			// we've successfully finished, after the last call with NULLs
			if (inSize == 0 && src == NULL) {
				break;
			}
		}
		if (inSize == 0) {
			// we're at the end - call one last time with NULLs
			src = NULL;
		}
	}

	// Add a zero character to the end. Hopefuly UTF32 uses the most bytes from
	// all possible encodings, so add 4 zero bytes.
	buffer = (char *)realloc(buffer, stringSize + 4);
	memset(buffer + stringSize, 0, 4);

#ifndef ICONV_USES_CONST
	delete[] originalSrc;
#endif // ICONV_USES_CONST

	iconv_close(iconvHandle);
	if (error) {
		if (buffer)
			free(buffer);
		return nullptr;
	}
	return buffer;
#else
	return nullptr;
#endif //USE_ICONV
}

struct ConversionTable {
	const char *name;
	const uint32 *table;
};

const ConversionTable g_encodingConversionTables[] = {
    {"cp850", getCodePageConversionTable(kCodePage850)},
    {"cp437", getCodePageConversionTable(kCodePage437)},
	{nullptr, nullptr}
};

char *Encoding::convertConversionTable(const char *to, const char *from, const char *string, size_t length) {
	const uint32 *table = nullptr;
	for (const ConversionTable *i = g_encodingConversionTables; i->name != nullptr; i++) {
		if (String(from).equalsIgnoreCase(i->name)) {
			table = i->table;
		}
	}
	if (table != nullptr) {
		uint32 *utf32Result = (uint32 *)calloc(sizeof(uint32), length + 1);
		if (!utf32Result) {
			warning("Could not allocate memory for encoding conversion");
			return nullptr;
		}
		for (unsigned i = 0; i < length; i++) {
			utf32Result[i] = table[(unsigned char)string[i]];
		}
		char *finalResult = convert(to, "utf-32", (char *)utf32Result, length * 4);
		free(utf32Result);
		return finalResult;
	}

	for (const ConversionTable *i = g_encodingConversionTables; i->name != nullptr; i++) {
		if (String(to).equalsIgnoreCase(i->name)) {
			table = i->table;
		}
	}
	if (table != nullptr) {
		uint32 *utf32Result = (uint32 *)convert("utf-32", from, string, length);
		if (String(from).hasPrefixIgnoreCase("utf-16"))
			length /= 2;
		if (String(from).hasPrefixIgnoreCase("utf-32"))
			length /= 4;
		char *finalResult = (char *)calloc(sizeof(char), length +1);
		if (!finalResult) {
			warning("Could not allocate memory for encoding conversion");
			return nullptr;
		}
		for (unsigned i = 0; i < length; i++) {
			for (unsigned j = 0; j < 257; j++) {
				if (j == 256) {
					// We have some character, that isn't a part of cp850, so
					// we replace it with '?' to remain consistent with iconv
					// and SDL
					finalResult[i] = '?';
				} else if (utf32Result[i] == table[j]){
					finalResult[i] = j;
					break;
				}
			}
		}
		free(utf32Result);
		return finalResult;
	}
	return nullptr;
}

static char g_cyrillicTransliterationTable[] = {
	' ', 'E', 'D', 'G', 'E', 'Z', 'I', 'I', 'J', 'L', 'N', 'C', 'K', '-', 'U', 'D',
	'A', 'B', 'V', 'G', 'D', 'E', 'Z', 'Z', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'R', 'S', 'T', 'U', 'F', 'H', 'C', 'C', 'S', 'S', '\"', 'Y', '\'', 'E', 'U', 'A',
	'a', 'b', 'v', 'g', 'd', 'e', 'z', 'z', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
	'r', 's', 't', 'u', 'f', 'h', 'c', 'c', 's', 's', '\"', 'y', '\'', 'e', 'u', 'a',
	'N', 'e', 'd', 'g', 'e', 'z', 'i', 'i', 'j', 'l', 'n', 'c', 'k', '?', 'u', 'd',
};

char *Encoding::transliterateCyrillic(const char *string) {
	char *result = (char *)malloc(strlen(string) + 1);
	if (!result) {
		warning("Could not allocate memory for encoding conversion");
		return nullptr;
	}
	for(unsigned i = 0; i <= strlen(string); i++) {
		if ((unsigned char)string[i] >= 160)
			result[i] = g_cyrillicTransliterationTable[(unsigned char)string[i] - 160];
		else
			result[i] = string[i];
	}
	return result;
}

uint32 *Encoding::transliterateUTF32(const uint32 *string, size_t length) {
	uint32 *result = (uint32 *)malloc(length + 4);
	if (!result) {
		warning("Could not allocate memory for encoding conversion");
		return nullptr;
	}
	for(unsigned i = 0; i <= length / 4; i++) {
		if (string[i] >= 0x410 && string[i] <= 0x450)
			result[i] = g_cyrillicTransliterationTable[string[i] - 160 - 864];
		else
			result[i] = string[i];
	}
	return result;
}

size_t Encoding::stringLength(const char *string, const String &encoding) {
	if (encoding.hasPrefixIgnoreCase("UTF-16")) {
		const uint16 *i = (const uint16 *)string;
		for (;*i != 0; i++) {}
		return (const char *)i - string;
	} else if (encoding.hasPrefixIgnoreCase("UTF-32")) {
		const uint32 *i = (const uint32 *)string;
		for (;*i != 0; i++) {}
		return (const char *)i - string;
	} else {
		const char *i = string;
		for (;*i != 0; i++) {}
		return i - string;
	}
}

}
