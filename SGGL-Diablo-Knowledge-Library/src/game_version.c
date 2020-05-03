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

#include <shlwapi.h>
#include <stdlib.h>
#include <windows.h>

#include "diablo/diablo_game_version.h"
#include "diablo_ii/diablo_ii_game_version.h"
#include "helper/error_handling.h"
#include "helper/game_version_finder.h"

/* Struct taken from Microsoft's example. */
struct LANGANDCODEPAGE {
  WORD wLanguage;
  WORD wCodePage;
};

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
static wchar_t* running_product_name;

static wchar_t* ExtractFileStringValue(
    const wchar_t* game_path,
    const wchar_t* string_name,
    size_t string_name_len
) {
  DWORD ignored;

  void* file_version_info;
  DWORD file_version_info_size;

  BOOL is_get_file_version_info_success;
  BOOL is_ver_query_value_success;

  struct LANGANDCODEPAGE* lang_buffer;
  UINT lang_buffer_size;

  wchar_t* temp_file_string_value;

  wchar_t* file_string_value;
  UINT file_string_value_size;

  wchar_t* file_string_sub_block;
  size_t file_string_sub_block_capacity;

  int swprintf_result;

  /* Check version size. */
  file_version_info_size = GetFileVersionInfoSizeW(
      game_path,
      &ignored
  );

  if (file_version_info_size == 0) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"GetFileVersionInfoSizeW",
        GetLastError()
    );
  }

  /* Get the file version info.*/
  file_version_info = malloc(file_version_info_size);

  if (file_version_info == NULL) {
    ExitOnAllocationFailure();
  }

  is_get_file_version_info_success = GetFileVersionInfoW(
      game_path,
      ignored,
      file_version_info_size,
      file_version_info
  );

  if (!is_get_file_version_info_success) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"GetFileVersionInfoW",
        GetLastError()
    );
  }

  /* Gather all of the information into the specified buffer. */
  is_ver_query_value_success = VerQueryValueW(
      file_version_info,
      L"\\VarFileInfo\\Translation",
      (void**) &lang_buffer,
      &lang_buffer_size
  );

  if (!is_ver_query_value_success) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"VerQueryValueW",
        GetLastError()
    );
  }

  /* Format text into the file string sub block. */
  file_string_sub_block_capacity = string_name_len + 8;
  file_string_sub_block = NULL;

  do {
    file_string_sub_block_capacity *= 2;
    file_string_sub_block = realloc(
        file_string_sub_block,
        file_string_sub_block_capacity * sizeof(file_string_sub_block[0])
    );

    swprintf_result = swprintf(
        file_string_sub_block,
        file_string_sub_block_capacity,
        L"\\StringFileInfo\\%04x%04x\\%ls",
        lang_buffer[0].wLanguage,
        lang_buffer[0].wCodePage,
        string_name
    );
  } while (swprintf_result == -1);

  /* Query the file string value. */
  is_ver_query_value_success = VerQueryValueW(
      file_version_info,
      file_string_sub_block,
      (void**) &temp_file_string_value,
      &file_string_value_size
  );

  if (!is_ver_query_value_success) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"VerQueryValueW",
        GetLastError()
    );
  }

  file_string_value = malloc(file_string_value_size);
  wcscpy(file_string_value, temp_file_string_value);

free_file_version_info:
  free(file_version_info);

  return file_string_value;
}

void InitGameVersion(const wchar_t* game_path, size_t game_path_len) {
  const wchar_t* kProductNameStr = L"ProductName";
  const size_t kProductNameLen =
      (sizeof(L"ProductName") / sizeof(kProductNameStr[0])) - 1;

  const struct ProductNameAndFindGameVersionFunctionEntry* search_result;

  /* Initialize everything required for determining the game. */
  running_product_name = ExtractFileStringValue(
      game_path,
      kProductNameStr,
      kProductNameLen
  );

  /* Determine what to do based on the reported game name. */
  search_result =
      (const struct ProductNameAndFindGameVersionFunctionEntry*) bsearch(
          running_product_name,
          find_version_func_table,
          sizeof(find_version_func_table)
              / sizeof(find_version_func_table[0]),
          sizeof(find_version_func_table[0]),
          &ProductNameAndFindGameVersionFunctionEntry_CompareKey
      );

  running_game_version = search_result->game_version_find_func_ptr(
      game_path,
      game_path_len
  );
}

void DeinitGameVersion(void) {
  free(running_product_name);
  running_game_version = 0;
}

enum GameVersion GetRunningGameVersion(void) {
  return running_game_version;
}
