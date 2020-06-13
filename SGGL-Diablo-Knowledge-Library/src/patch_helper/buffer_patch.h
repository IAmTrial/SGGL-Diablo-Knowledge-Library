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

#ifndef SGGLDKL_PATCH_HELPER_BUFFER_PATCH_H_
#define SGGLDKL_PATCH_HELPER_BUFFER_PATCH_H_

#include <stddef.h>
#include <windows.h>

struct BufferPatch {
  void* position;
  unsigned char is_patched;
  size_t buffer_size;
  unsigned char* patch_buffer;
  const PROCESS_INFORMATION* process_info;
  unsigned char* original_buffer;
};

struct BufferPatch* BufferPatch_Init(
    struct BufferPatch* buffer_patch,
    void* position,
    size_t buffer_size,
    const unsigned char* patch_buffer,
    const PROCESS_INFORMATION* process_info
);

void BufferPatch_Deinit(struct BufferPatch* buffer_patch);

void BufferPatch_Apply(struct BufferPatch* buffer_patch);

void BufferPatch_Remove(struct BufferPatch* buffer_patch);

#endif /* SGGLDKL_PATCH_HELPER_BUFFER_PATCH_H_ */
