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

#include "hellfire_game_version.h"

#include <stdlib.h>
#include <stddef.h>
#include <windows.h>

#include "../helper/file_info.h"
#include "../helper/short_version.h"

/*
* The order of the entries should be in numerical order of significant
* versions, due to the reliance on bsearch.
*/

static const struct ShortVersionStringAndGameVersionEntry
kHellfireProductVersionsToGameVersion[] = {
    /* 1, 0, 0, 0 */
    { { L"1, 0, 0, 0" }, HELLFIRE_1_00 },

    /* 1, 0, 1, 0 */
    { { L"1, 0, 1, 0" }, HELLFIRE_1_01 }
};

static enum GameVersion SearchGameVersionTable(
    const wchar_t* hellfire_file_version_str
) {
  const struct ShortVersionStringAndGameVersionEntry* search_result;

  struct ShortVersionStringAndGameVersionEntry
  hellfire_product_version_search_key = {
      { hellfire_file_version_str }
  };

  /* Search on the game executable product version. */
  search_result = (const struct ShortVersionStringAndGameVersionEntry*) bsearch(
      &hellfire_product_version_search_key,
      kHellfireProductVersionsToGameVersion,
      sizeof(kHellfireProductVersionsToGameVersion)
          / sizeof(kHellfireProductVersionsToGameVersion[0]),
      sizeof(kHellfireProductVersionsToGameVersion[0]),
      &ShortVersionStringAndGameVersionEntry_CompareKey
  );

  if (search_result != NULL) {
    return search_result->game_version;
  }

  return VERSION_UNKNOWN;
}

enum GameVersion Hellfire_FindGameVersion(
    const wchar_t* hellfire_file_path,
    size_t hellfire_file_path_len
) {
  const wchar_t* kFileVersionString = L"FileVersion";
  const size_t kFileVersionStringLen =
      (sizeof(L"FileVersion") / sizeof(kFileVersionString[0])) - 1;

  enum GameVersion running_game_version;
  wchar_t* file_version_str;

  file_version_str = ExtractFileStringValue(
      hellfire_file_path,
      L"FileVersion",
      kFileVersionStringLen
  );

  running_game_version = SearchGameVersionTable(file_version_str);

free_file_version_str:
  free(file_version_str);

  return running_game_version;
}
