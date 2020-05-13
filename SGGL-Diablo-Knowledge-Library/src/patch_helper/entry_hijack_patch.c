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

#include "entry_hijack_patch.h"

#include <string.h>

#include "../asm_x86_macro.h"

static const unsigned char kFuncEnd[] = {
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
};

static size_t entry_hijack_func_size = 0;

static void Reentry(void) {
  /* Intentionally left empty. It's to act as a label */
}

__declspec(naked) static void __cdecl EntryHijackFunc_Init(void) {
  ASM_X86(pushad);

  ASM_X86(sub esp, 12);    /* sizeof("SGGLDIL.dll") */

  /* Store the srting "SGGLDIL.dll" */
  ASM_X86(mov byte ptr [esp], 'S');
  ASM_X86(mov byte ptr [esp + 1], 'G');
  ASM_X86(mov byte ptr [esp + 2], 'G');
  ASM_X86(mov byte ptr [esp + 3], 'L');
  ASM_X86(mov byte ptr [esp + 4], 'D');
  ASM_X86(mov byte ptr [esp + 5], 'I');
  ASM_X86(mov byte ptr [esp + 6], 'L');
  ASM_X86(mov byte ptr [esp + 7], '.');
  ASM_X86(mov byte ptr [esp + 8], 'd');
  ASM_X86(mov byte ptr [esp + 9], 'l');
  ASM_X86(mov byte ptr [esp + 10], 'l');
  ASM_X86(mov byte ptr [esp + 11], '\0');

  /* Load the library. */
  ASM_X86(push esp);
  ASM_X86(call ASM_X86_FUNC(LoadLibraryA));

  ASM_X86(add esp, 12);
  ASM_X86(popad);

  /* This needs to be manually added, and is dependent on the Game itself. */
  ASM_X86(jmp ASM_X86_FUNC(Reentry));

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

struct BufferPatch* EntryHijackPatch_Init(
    struct BufferPatch* buffer_patch,
    void* (*patch_address)(void),
    void* (*reentry_address)(void),
    const PROCESS_INFORMATION* process_info
) {
  void* (*reentry_jump_offset)(void);
  size_t jump_offset_index;

  /* Determine the number of op bytes in the entry hijack function. */
  for (entry_hijack_func_size = 0; ; entry_hijack_func_size += 1) {
    if (memcmp(&EntryHijackFunc_Init, kFuncEnd, sizeof(kFuncEnd)) == 0) {
      break;
    }
  }

  BufferPatch_Init(
      buffer_patch,
      (void*) patch_address,
      entry_hijack_func_size,
      (void*) &EntryHijackFunc_Init,
      process_info
  );

  /* Replace the Reentry jump offset with the intended address. */
  reentry_jump_offset = (unsigned char*) reentry_address
      - (unsigned char*) patch_address
      - entry_hijack_func_size;

  jump_offset_index = entry_hijack_func_size - sizeof(reentry_address);

  memcpy(
      &buffer_patch->patch_buffer[jump_offset_index],
      reentry_jump_offset,
      sizeof(reentry_jump_offset)
  );

  return buffer_patch;
}

void JumpTrapdoorPatch_Deinit(struct BufferPatch* buffer_patch) {
  BufferPatch_Deinit(buffer_patch);
}
