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

#ifndef SGGLDKL_HELPER_PE_HEADER_H_
#define SGGLDKL_HELPER_PE_HEADER_H_

#include <stddef.h>
#include <wchar.h>

struct PeHeader {
  wchar_t* file_path;
  size_t file_path_len;

  void* start_address;

  void* entry_point_address;
  void* code_base_address;
  void* data_base_address;
  void* image_base_address;
};

struct PeHeader* PeHeader_Init(
    struct PeHeader* pe_header,
    const wchar_t* file_path,
    size_t file_path_len
);

void PeHeader_Deinit(struct PeHeader* pe_header);

#endif /* SGGLDKL_HELPER_PE_HEADER_H_ */