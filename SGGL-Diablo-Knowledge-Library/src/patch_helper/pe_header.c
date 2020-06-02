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

#include <stdio.h>
#include <stddef.h>

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
  FILE* file_stream;

  int is_fseek_fail;
  size_t num_fread_objects;

  char* file_path_mb;

  /* Store a copy of the file path in the PE header. */
  pe_header->file_path = malloc(
      (file_path_len + 1) * sizeof(pe_header->file_path[0])
  );

  if (pe_header->file_path == NULL) {
    ExitOnAllocationFailure();
  }

  wcscpy(pe_header->file_path, file_path);

  pe_header->file_path_len = file_path_len;

  /* Convert the path to multibyte characters. */
  file_path_mb = ConvertWideToMultibyte(NULL, pe_header->file_path);

  /* Open the file for reading, in byte mode. */
  file_stream = fopen(file_path_mb, "rb");

  if (file_stream == NULL) {
    ExitOnGeneralFailure(
        L"Could not open file for reading.",
        L"Error"
    );
  }

  /* Seek to the PE header pointer offset. */
  is_fseek_fail = fseek(file_stream, PE_HEADER_PTR_OFFSET, SEEK_SET);

  if (is_fseek_fail) {
    ExitOnGeneralFailure(
        L"Cannot seek to the file's target offset.",
        L"Error"
    );
  }

  /* Load the start address of the PE file. */
  num_fread_objects = fread(
     &pe_header->start_address,
     sizeof(pe_header->start_address),
     1,
     file_stream
  );

  if (num_fread_objects != 1) {
    ExitOnGeneralFailure(
        L"Number of fread objects does not match.",
        L"Error"
    );
  }

  /* Seek to the start address to extract info from the PE file. */
  is_fseek_fail = fseek(
      file_stream,
      ((const long) pe_header->start_address) + ENTRY_POINT_ADDRESS_OFFSET,
      SEEK_SET
  );

  if (is_fseek_fail) {
    ExitOnGeneralFailure(
        L"Cannot seek to the file's target offset.",
        L"Error"
    );
  }

  /* Read the entry point address. */
  num_fread_objects = fread(
     &pe_header->entry_point_address,
     sizeof(pe_header->entry_point_address),
     1,
     file_stream
  );

  if (num_fread_objects != 1) {
    ExitOnGeneralFailure(
        L"Number of fread objects does not match.",
        L"Error"
    );
  }

  /* Read the code base address. */
  num_fread_objects = fread(
     &pe_header->code_base_address,
     sizeof(pe_header->code_base_address),
     1,
     file_stream
  );

  if (num_fread_objects != 1) {
    ExitOnGeneralFailure(
        L"Number of fread objects does not match.",
        L"Error"
    );
  }

  /* Read the data base address. */
  num_fread_objects = fread(
     &pe_header->data_base_address,
     sizeof(pe_header->data_base_address),
     1,
     file_stream
  );

  if (num_fread_objects != 1) {
    ExitOnGeneralFailure(
        L"Number of fread objects does not match.",
        L"Error"
    );
  }

  /* Read the image base address. */
  num_fread_objects = fread(
     &pe_header->image_base_address,
     sizeof(pe_header->image_base_address),
     1,
     file_stream
  );

  if (num_fread_objects != 1) {
    ExitOnGeneralFailure(
        L"Number of fread objects does not match.",
        L"Error"
    );
  }

free_file_path_mb:
  free(file_path_mb);

  return pe_header;
}

void PeHeader_Deinit(struct PeHeader* pe_header) {
  pe_header->file_path_len = 0;
  pe_header->start_address = NULL;
  pe_header->entry_point_address = NULL;
  pe_header->code_base_address = NULL;
  pe_header->data_base_address = NULL;
  pe_header->image_base_address = NULL;

  free(pe_header->file_path);

  pe_header->file_path = NULL;
}

void* PeHeader_GetHardDataAddress(
    const struct PeHeader* pe_header
) {
  return (unsigned char*) pe_header->image_base_address
      + (size_t) pe_header->data_base_address;
}

void* PeHeader_GetHardEntryPointAddress(
    const struct PeHeader* pe_header
) {
  return (unsigned char*) pe_header->image_base_address
      + (size_t) pe_header->entry_point_address;
}
