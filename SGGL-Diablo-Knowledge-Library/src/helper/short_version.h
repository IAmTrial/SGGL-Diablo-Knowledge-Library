/**
 * SlashGaming Game Loader - Diablo Knowledge Library
 * Copyright (C) 2020  Mir Drualga
 *
 * This file is part of SlashGaming Game Loader - Diablo Knowledge Library.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Additional permissions under GNU Affero General Public License version 3
 *  section 7
 *
 *  If you modify this Program, or any covered work, by linking or combining
 *  it with any program (or a modified version of that program and its
 *  libraries), containing parts covered by the terms of an incompatible
 *  license, the licensors of this Program grant you additional permission
 *  to convey the resulting work.
 */

#ifndef SGGLDKL_HELPER_SHORT_VERSION_H_
#define SGGLDKL_HELPER_SHORT_VERSION_H_

#include <wchar.h>
#include <windows.h>

#include "../game_version.h"

struct ShortVersion {
  DWORD major_left;
  DWORD major_right;
  DWORD minor_left;
  DWORD minor_right;
};

struct ShortVersionString {
  const wchar_t* version_str;
};

struct ShortVersionAndGameVersionEntry {
  struct ShortVersion short_version;
  enum GameVersion game_version;
};

struct ShortVersionStringAndGameVersionEntry {
  struct ShortVersionString short_version_str;
  enum GameVersion game_version;
};

int ShortVersion_CompareAll(
    const struct ShortVersion* version1,
    const struct ShortVersion* version2
);

int ShortVersion_CompareAsVoidAll(
    const void* version1,
    const void* version2
);

int ShortVersionString_CompareAll(
    const struct ShortVersionString* version_str1,
    const struct ShortVersionString* version_str2
);

int ShortVersionString_CompareAsVoidAll(
    void* version_str1,
    void* version_str2
);

int ShortVersionAndGameVersionEntry_CompareKey(
    const struct ShortVersionAndGameVersionEntry* entry1,
    const struct ShortVersionAndGameVersionEntry* entry2
);

int ShortVersionAndGameVersionEntry_CompareAsVoidKey(
    const void* entry1,
    const void* entry2
);

int ShortVersionStringAndGameVersionEntry_CompareKey(
    const struct ShortVersionStringAndGameVersionEntry* entry1,
    const struct ShortVersionStringAndGameVersionEntry* entry2
);

int ShortVersionStringAndGameVersionEntry_CompareAsVoidKey(
    const void* entry1,
    const void* entry2
);

#endif /* SGGLDKL_HELPER_SHORT_VERSION_H_ */
