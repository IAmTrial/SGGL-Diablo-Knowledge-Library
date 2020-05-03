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

#include "diablo_game_version.h"

#include <shlwapi.h>
#include <stdlib.h>
#include <windows.h>

#include "../error_handling.h"
#include "../helper/file_info.h"
#include "../helper/short_version.h"

/*
* The order of the entries should be in numerical order of significant
* versions, due to the reliance on bsearch.
*/

static const struct ShortVersionAndGameVersionEntry
kDiabloProductVersionsToGameVersion[] = {
    /* 1, 0, 8, 1 */
    { { 1, 0, 8, 1 }, DIABLO_1_08 },

    /* 1, 0, 9, 1 */
    { { 1, 0, 9, 1 }, DIABLO_1_09 },

    /* 1, 0, 9, 2 */
    { { 1, 0, 9, 1 }, DIABLO_1_09B },

    /* 96, 12, 26, 3 */
    { { 96, 12, 26, 3 }, DIABLO_1_00 },

    /* 97, 4, 1, 1 */
    { { 97, 4, 1, 1 }, DIABLO_1_03 },

    /* 97, 5, 23, 1 */
    { { 97, 5, 23, 1 }, DIABLO_1_04 }
};

static const struct ShortVersionAndGameVersionEntry
kStormFileVersionsToGameVersion[] = {
    /* 1998.4.15.1 */
    { { 1998, 4, 15, 1 }, DIABLO_1_05 },

    /* 1998.8.11.1 */
    { { 1998, 8, 11, 1 }, DIABLO_1_07 }
};

static wchar_t* GetStormPath(
    const wchar_t* diablo_file_path,
    size_t diablo_file_path_len
) {
  const wchar_t* kStormDllFileName = L"storm.dll";
  const size_t kStormDllFileNameLen =
      (sizeof(L"storm.dll") / sizeof(kStormDllFileName[0])) - 1;

  wchar_t* storm_file_path;

  storm_file_path = malloc(
      (diablo_file_path_len + kStormDllFileNameLen) * sizeof(storm_file_path[0])
  );

  if (storm_file_path == NULL) {
    ExitOnAllocationFailure();
  }

  wcscpy(storm_file_path, diablo_file_path);
  PathRemoveFileSpecW(storm_file_path);
  PathAppendW(storm_file_path, kStormDllFileName);

  return storm_file_path;
}

static enum GameVersion SearchGameVersionTable(
    const VS_FIXEDFILEINFO* diablo_file_info,
    const VS_FIXEDFILEINFO* storm_file_info
) {
  struct ShortVersionAndGameVersionEntry* search_result;

  struct ShortVersionAndGameVersionEntry diablo_product_version_search_key = {
      {
          (diablo_file_info->dwProductVersionMS >> 16) & 0xFFFF,
          (diablo_file_info->dwProductVersionMS >> 0) & 0xFFFF,
          (diablo_file_info->dwProductVersionLS >> 16) & 0xFFFF,
          (diablo_file_info->dwProductVersionLS >> 0) & 0xFFFF
      },
      VERSION_UNKNOWN
  };

  struct ShortVersionAndGameVersionEntry storm_file_version_search_key = {
      {
          (storm_file_info->dwFileVersionMS >> 16) & 0xFFFF,
          (storm_file_info->dwFileVersionMS >> 0) & 0xFFFF,
          (storm_file_info->dwFileVersionLS >> 16) & 0xFFFF,
          (storm_file_info->dwFileVersionLS >> 0) & 0xFFFF
      },
      VERSION_UNKNOWN
  };

  /* Search on the game executable product version. */
  search_result = (struct ShortVersionAndGameVersionEntry*) bsearch(
      &diablo_product_version_search_key,
      kDiabloProductVersionsToGameVersion,
      sizeof(kDiabloProductVersionsToGameVersion)
          / sizeof(kDiabloProductVersionsToGameVersion[0]),
      sizeof(kDiabloProductVersionsToGameVersion[0]),
      &ShortVersionAndGameVersionEntry_CompareKey
  );

  if (search_result != NULL) {
    return search_result->game_version;
  }

  /* Search on the Storm.dll library file version. */
  search_result = (struct ShortVersionAndGameVersionEntry*) bsearch(
      &storm_file_version_search_key,
      kStormFileVersionsToGameVersion,
      sizeof(kStormFileVersionsToGameVersion)
          / sizeof(kStormFileVersionsToGameVersion[0]),
      sizeof(kStormFileVersionsToGameVersion[0]),
      &ShortVersionAndGameVersionEntry_CompareKey
  );

  if (search_result != NULL) {
    return search_result->game_version;
  }

  return VERSION_UNKNOWN;
}

enum GameVersion Diablo_FindGameVersion(
    const wchar_t* diablo_file_path,
    size_t diablo_file_path_len
) {
  wchar_t* storm_file_path;

  VS_FIXEDFILEINFO diablo_file_info;
  VS_FIXEDFILEINFO storm_file_info;

  /* Diablo has to use Storm.dll and Diablo.exe to determine the version. */
  storm_file_path = GetStormPath(diablo_file_path, diablo_file_path_len);

  ExtractFileInfo(&diablo_file_info, diablo_file_path);
  ExtractFileInfo(&storm_file_info, storm_file_path);

  free(storm_file_path);

  return SearchGameVersionTable(&diablo_file_info, &storm_file_info);
}
