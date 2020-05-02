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
#include <windows.h>

#include "diablo/diablo_game_version.h"
#include "error_handling.h"

/* Struct taken from Microsoft's example. */
struct LANGANDCODEPAGE {
  WORD wLanguage;
  WORD wCodePage;
};

static enum GameVersion running_game_version = VERSION_UNKNOWN;
static wchar_t* running_product_name;

static void ExtractFileInfo(
    VS_FIXEDFILEINFO* file_info,
    const wchar_t* game_path
) {
  DWORD ignored;

  void* file_version_info;
  DWORD file_version_info_size;

  BOOL is_get_file_version_info_success;
  BOOL is_ver_query_value_success;

  VS_FIXEDFILEINFO* temp_file_info;
  UINT temp_file_info_size;

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
      L"\\",
      (void**) &temp_file_info,
      &temp_file_info_size
  );

  if (!is_ver_query_value_success) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"VerQueryValueW",
        GetLastError()
    );
  }

  /* Copy the file info into the parameter. */
  *file_info = *temp_file_info;

free_file_version_info:
  free(file_version_info);
}

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

  const wchar_t* kStormDllFileName = L"storm.dll";
  const size_t kStormDllFileNameLen =
      (sizeof(L"storm.dll") / sizeof(kStormDllFileName[0])) - 1;

  VS_FIXEDFILEINFO game_file_info;
  VS_FIXEDFILEINFO storm_file_info;

  wchar_t* storm_file_path;

  /* Initialize everything required for determining the game. */
  running_product_name = ExtractFileStringValue(
      game_path,
      kProductNameStr,
      kProductNameLen
  );

  /* Determine what to do based on the reported game name. */
  if (wcscmp(running_product_name, L"Blizzard Entertainment Diablo") == 0) {
    /* Diablo has to use Storm.dll and Diablo.exe to determine the version. */

    /* Set up the path as Storm.dll from the same directory. */
    storm_file_path = malloc(
        (game_path_len + kStormDllFileNameLen) * sizeof(storm_file_path[0])
    );

    if (storm_file_path == NULL) {
      ExitOnAllocationFailure();
    }

    wcscpy(storm_file_path, game_path);
    PathRemoveFileSpecW(storm_file_path);
    PathAppendW(storm_file_path, kStormDllFileName);

    ExtractFileInfo(&storm_file_info, storm_file_path);

    free(storm_file_path);

    running_game_version = Diablo_DetermineGameVersion(
        &game_file_info,
        &storm_file_info
    );
  }
}

void DeinitGameVersion(void) {
  free(running_product_name);
  running_game_version = 0;
}

enum GameVersion GetRunningGameVersion(void) {
  return running_game_version;
}
