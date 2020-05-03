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

#include "game_version.h"

#include <stdlib.h>
#include <windows.h>

#include "diablo/diablo_game_version.h"
#include "diablo_ii/diablo_ii_game_version.h"
#include "helper/error_handling.h"
#include "helper/file_info.h"
#include "helper/game_version_finder.h"

/*
* The order of the entries should be in lexicographical order, due to
* the reliance on bsearch.
*/

static const struct ProductNameAndFindGameVersionFunctionEntry
find_version_func_table[] = {
    { L"Blizzard Entertainment Diablo", &Diablo_FindGameVersion },
    { L"BLizzard North Diablo 2", &Diablo_II_FindGameVersion },
    { L"Blizzard North Diablo II", &Diablo_II_FindGameVersion },
};

static enum GameVersion running_game_version = VERSION_UNKNOWN;

void InitGameVersion(const wchar_t* game_path, size_t game_path_len) {
  const wchar_t* kProductNameStr = L"ProductName";
  const size_t kProductNameLen =
      (sizeof(L"ProductName") / sizeof(kProductNameStr[0])) - 1;

  struct ProductNameAndFindGameVersionFunctionEntry search_key;
  const struct ProductNameAndFindGameVersionFunctionEntry* search_result;

  wchar_t* running_product_name;

  /* Initialize everything required for determining the game. */
  running_product_name = ExtractFileStringValue(
      game_path,
      kProductNameStr,
      kProductNameLen
  );

  search_key.product_name = running_product_name;

  /* Determine what to do based on the reported game name. */
  search_result =
      (const struct ProductNameAndFindGameVersionFunctionEntry*) bsearch(
          &search_key,
          find_version_func_table,
          sizeof(find_version_func_table)
              / sizeof(find_version_func_table[0]),
          sizeof(find_version_func_table[0]),
          &ProductNameAndFindGameVersionFunctionEntry_CompareKey
      );

  if (search_result == NULL) {
    running_game_version = VERSION_UNKNOWN;

    goto free_running_product_name;
  }

  running_game_version = search_result->game_version_find_func_ptr(
      game_path,
      game_path_len
  );

free_running_product_name:
  free(running_product_name);
}

void DeinitGameVersion(void) {
  running_game_version = 0;
}

enum GameVersion GetRunningGameVersion(void) {
  return running_game_version;
}
