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

#include "pe_header.h"

#include <stdlib.h>

#include "../helper/encoding.h"
#include "../helper/error_handling.h"

enum Constant {
  PE_HEADER_PTR_OFFSET = 0x3C,
  ENTRY_POINT_ADDRESS_OFFSET = 0x28
};

struct PeHeader* PeHeader_Init(
    struct PeHeader* pe_header,
    const wchar_t* file_path,
    size_t file_path_len
) {
  char* mb_file_path;
  LOADED_IMAGE* loaded_image;

  pe_header->file_path_len = file_path_len;

  pe_header->file_path = malloc(
      (file_path_len + 1) * sizeof(pe_header->file_path[0])
  );

  if (pe_header->file_path == NULL) {
    ExitOnAllocationFailure();
  }

  mb_file_path = ConvertWideToMultibyte(
      NULL,
      file_path
  );

  loaded_image = ImageLoad(mb_file_path, NULL);

  memcpy(
      &pe_header->nt_headers,
      loaded_image->FileHeader,
      sizeof(pe_header->nt_headers)
  );

unload_image:
  ImageUnload(loaded_image);

free_mb_file_path:
  free(mb_file_path);

  return pe_header;
}

void PeHeader_Deinit(struct PeHeader* pe_header) {
  pe_header->file_path_len = 0;

  free(pe_header->file_path);
  pe_header->file_path = NULL;
}

void* PeHeader_GetHardDataAddress(
    const struct PeHeader* pe_header
) {
  return (unsigned char*) pe_header->nt_headers.OptionalHeader.ImageBase
      + pe_header->nt_headers.OptionalHeader.BaseOfData;
}

void* PeHeader_GetHardEntryPointAddress(
    const struct PeHeader* pe_header
) {
  return (unsigned char*) pe_header->nt_headers.OptionalHeader.ImageBase
      + pe_header->nt_headers.OptionalHeader.AddressOfEntryPoint;
}
