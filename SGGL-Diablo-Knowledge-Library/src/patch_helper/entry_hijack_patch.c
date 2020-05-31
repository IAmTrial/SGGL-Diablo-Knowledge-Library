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

static const unsigned char kEntryHijackBytes[] = {
  /* push 0 */
  0x68, 0x00, 0x00, 0x00, 0x00,

  /* call dummy_func */
  0xE8, 0x00, 0x00, 0x00, 0x00
};

__declspec(naked) static void __stdcall CleanupFunc(void** top_of_stack) {
  /* Allow the return address to go back to the original code. */
  ASM_X86(sub dword ptr [esp], 5);

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
    const PROCESS_INFORMATION* process_info,
    const struct PeHeader* pe_header
) {
  unsigned char* data_address;
  unsigned char* func_address;
  unsigned char* func_address_offset;

  BufferPatch_Init(
      buffer_patch,
      (void*) patch_address,
      sizeof(kEntryHijackBytes),
      kEntryHijackBytes,
      process_info
  );

  /*
  * Rewrite the dummy values with the intended values. Replaces the
  * push value with the data address and the call value with the
  * function address offset.
  */
  data_address = PeHeader_GetHardDataAddress(pe_header);
  func_address = data_address + sizeof(data_address);

  func_address_offset = (unsigned char*) (data_address + 4)
      - (size_t) patch_address
      - 5
      - 5;

  memcpy(&buffer_patch->patch_buffer[1], &data_address, sizeof(data_address));
  memcpy(
      &buffer_patch->patch_buffer[6],
      &func_address_offset,
      sizeof(data_address)
  );

  return buffer_patch;
}

void EntryHijackPatch_Deinit(struct BufferPatch* buffer_patch) {
  BufferPatch_Deinit(buffer_patch);
}
