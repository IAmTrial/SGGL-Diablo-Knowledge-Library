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

static const unsigned char kEntryHijackBytes[] = {
  /* push 0 */
  0x68, 0x00, 0x00, 0x00, 0x00,

  /* call dummy_func */
  0xE8, 0x00, 0x00, 0x00, 0x00,

  /*
  * Set the return to be the at the position of the push. Upon return,
  * the code should be the original.
  *
  * sub dword ptr [esp], 10
  */
  0x83, 0x2C, 0x24, 0x0A,

  /* jmp (beyond the free space) */
  0xEB, 0x04,

  /* Free space for one pointer. */
  0x00, 0x00, 0x00, 0x00
};

struct BufferPatch* EntryHijackPatch_Init(
    struct BufferPatch* entry_hijack_patch,
    void* (*patch_address)(void),
    const PROCESS_INFORMATION* process_info,
    const struct PeHeader* pe_header
) {
  unsigned char* free_space_address;

  BufferPatch_Init(
      entry_hijack_patch,
      (void*) patch_address,
      EntryHijackPatch_GetSize(),
      kEntryHijackBytes,
      process_info
  );

  free_space_address = (unsigned char*) patch_address
      + EntryHijackPatch_GetSize()
      - sizeof(void*);

  memcpy(
      &entry_hijack_patch->patch_buffer[1],
      &free_space_address,
      sizeof(free_space_address)
  );

  return entry_hijack_patch;
}

void EntryHijackPatch_Deinit(struct BufferPatch* entry_hijack_patch) {
  BufferPatch_Deinit(entry_hijack_patch);
}

void* EntryHijackPatch_GetFreeSpaceAddress(
    const struct BufferPatch* entry_hijack_patch
) {
  return (unsigned char*) entry_hijack_patch->position
      + EntryHijackPatch_GetFreeSpaceOffset();
}

size_t EntryHijackPatch_GetSize(void) {
  return sizeof(kEntryHijackBytes);
}

size_t EntryHijackPatch_GetFreeSpaceOffset(void) {
  return EntryHijackPatch_GetSize() - sizeof(void*);
}
