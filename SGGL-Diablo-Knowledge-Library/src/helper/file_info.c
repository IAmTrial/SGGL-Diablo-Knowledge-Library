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

#include "file_info.h"

#include "error_handling.h"

/* Struct taken from Microsoft's example. */
struct LANGANDCODEPAGE {
  WORD wLanguage;
  WORD wCodePage;
};

void ExtractFileInfo(
    VS_FIXEDFILEINFO* file_info,
    const wchar_t* file_path
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
      file_path,
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
      file_path,
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

wchar_t* ExtractFileStringValue(
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
