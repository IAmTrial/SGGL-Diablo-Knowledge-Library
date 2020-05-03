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
