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

#include "buffer_patch.h"

#include <stdlib.h>

#include "error_handling.h"

struct BufferPatch* BufferPatch_Create(
    struct BufferPatch* buffer_patch,
    void* position,
    size_t buffer_size,
    const unsigned char* patch_buffer,
    const PROCESS_INFORMATION* process_info
) {
  BOOL is_read_process_memory_success;

  buffer_patch->position = position;
  buffer_patch->is_patched = 0;
  buffer_patch->buffer_size = buffer_size;
  buffer_patch->process_info = process_info;

  /*Make a copy of the patch buffer. */
  buffer_patch->patch_buffer = malloc(buffer_size);

  if (buffer_patch->patch_buffer == NULL) {
    ExitOnAllocationFailure();
  }

  memcpy(buffer_patch->patch_buffer, patch_buffer, buffer_size);

  /* Make a copy of the original data before modification. */
  buffer_patch->original_buffer = malloc(buffer_size);

  if (buffer_patch->original_buffer == NULL) {
    ExitOnAllocationFailure();
  }

  is_read_process_memory_success = ReadProcessMemory(
      process_info->hProcess,
      position,
      buffer_patch->original_buffer,
      buffer_size,
      NULL
  );

  if (!is_read_process_memory_success) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"ReadProcessMemory",
        GetLastError()
    );
  }

  return buffer_patch;
}

void BufferPatch_Destroy(struct BufferPatch* buffer_patch) {
  BufferPatch_Remove(buffer_patch);

  buffer_patch->position = NULL;
  buffer_patch->is_patched = 0;
  buffer_patch->buffer_size = 0;
  buffer_patch->process_info = NULL;

  free(buffer_patch->patch_buffer);
  free(buffer_patch->original_buffer);

  buffer_patch->patch_buffer = NULL;
  buffer_patch->original_buffer = NULL;
}

void BufferPatch_Apply(struct BufferPatch* buffer_patch) {
  BOOL is_write_process_memory_success;

  if (buffer_patch->is_patched) {
    return;
  }

  is_write_process_memory_success = WriteProcessMemory(
      buffer_patch->process_info->hProcess,
      buffer_patch->position,
      buffer_patch->patch_buffer,
      buffer_patch->buffer_size,
      NULL
  );

  if (!is_write_process_memory_success) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"WriteProcessMemory",
        GetLastError()
    );
  }

  buffer_patch->is_patched = 1;
}

void BufferPatch_Remove(struct BufferPatch* buffer_patch) {
  BOOL is_write_process_memory_success;

  if (!buffer_patch->is_patched) {
    return;
  }

  is_write_process_memory_success = WriteProcessMemory(
      buffer_patch->process_info->hProcess,
      buffer_patch->position,
      buffer_patch->original_buffer,
      buffer_patch->buffer_size,
      NULL
  );

  if (!is_write_process_memory_success) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"WriteProcessMemory",
        GetLastError()
    );
  }

  buffer_patch->is_patched = 0;
}
