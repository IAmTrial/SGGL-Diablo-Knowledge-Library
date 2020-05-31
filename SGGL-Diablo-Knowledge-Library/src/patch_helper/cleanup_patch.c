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

#include "cleanup_patch.h"

#include <string.h>

#include "../asm_x86_macro.h"

static const unsigned char kFuncEnd[] = {
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
};

__declspec(naked) static void __stdcall CleanupFunc(void** top_of_stack) {
  /* Hex for 8 0x90, which is used to detect the end of the function. */
  ASM_X86(nop);
  ASM_X86(nop);
  ASM_X86(nop);
  ASM_X86(nop);
  ASM_X86(nop);
  ASM_X86(nop);
  ASM_X86(nop);
  ASM_X86(nop);
}

static void InitFuncSize(size_t* func_size) {
  const unsigned char* cleanup_func_bytes;
  int memcmp_result;

  cleanup_func_bytes = (const unsigned char*) &CleanupFunc;

  do {
    memcmp_result = memcmp(
        &cleanup_func_bytes[*func_size],
        kFuncEnd,
        sizeof(kFuncEnd)
    );

    *func_size += 1;
  } while (memcmp_result != 0);

  *func_size -= 1;
}

struct BufferPatch* CleanupPatch_Init(
    struct BufferPatch* buffer_patch,
    void* (*patch_address)(void),
    const PROCESS_INFORMATION* process_info
) {
  BufferPatch_Init(
      buffer_patch,
      (void*) patch_address,
      CleanupPatch_GetSize(),
      (void*) &CleanupFunc,
      process_info
  );

  return buffer_patch;
}

void CleanupPatch_Deinit(struct BufferPatch* buffer_patch) {
  BufferPatch_Deinit(buffer_patch);
}

size_t CleanupPatch_GetSize(void) {
  static size_t cleanup_func_size;

  if (cleanup_func_size == 0) {
    InitFuncSize(&cleanup_func_size);
  }

  return cleanup_func_size;
}
