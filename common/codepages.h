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

#ifndef COMMON_CODEPAGES_H
#define COMMON_CODEPAGES_H

namespace Common {

typedef unsigned int uint32;

enum CodePage {
	kCodePageInvalid = -1,
	kUtf8 = 0,
	kCodePage437,
	kCodePage850,
	kCodePage866,
	kWindows932,
	kWindows949,
	kWindows950,
	kWindows1250,
	kWindows1251,
	kWindows1252,
	kWindows1253,
	kWindows1254,
	kWindows1255,
	kWindows1256,
	kWindows1257
};

const uint32 *getCodePageConversionTable(const CodePage page);

} // End of namespace Common

#endif
