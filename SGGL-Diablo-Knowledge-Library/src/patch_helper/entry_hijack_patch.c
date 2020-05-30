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

static size_t payload_func_size = 0;

__declspec(naked) static void __stdcall PayloadFunc(void** top_of_stack) {
  /* Allow the return address to go back to the original code. */
  ASM_X86(sub dword ptr [esp], 5);

  /* Function prologue */
  ASM_X86(pushad);
  ASM_X86(mov ebp, esp);

  /*
  * Stack:
  * 36: pointer to stack allocated data a.k.a. top_of_stack
  * 32: return address
  * 28 to 0: pushad
  * -4: num_libs, needs to be inited by SGGL
  * -8: current_thread_handle
  * -12: is_lib_resize_needed, can be modified by SGGL
  * -16: lib_path_size, can be read by SGGL
  * -20: lib_path_ptr, data inside can be modified by SGGL
  */
  ASM_X86(sub esp, 20);

  /* *top_of_stack = esp; */
  ASM_X86(mov esi, dword ptr [ebp + 36]);
  ASM_X86(mov dword ptr [esi], esp);

  /* is_lib_resize_needed = 1; */
  ASM_X86(mov dword ptr [ebp - 12], 1);

  /* current_thread_handle = GetCurrentThread(); */
  ASM_X86(call ASM_X86_FUNC(GetCurrentThread));
  ASM_X86(mov dword ptr [ebp - 8], eax);

  /* lib_path_size = 32; */
  ASM_X86(mov dword ptr [ebp - 16], 32);

  ASM_X86(jmp PayloadFunc_AllocPath);

ASM_X86_LABEL(PayloadFunc_ReallocPath)
  /* Free lib_path_ptr for reallocation. */
  ASM_X86(push 0x800);                  /* MEM_RELEASE */
  ASM_X86(push 0);
  ASM_X86(push dword ptr [ebp - 20]);   /* lib_path_ptr */
  ASM_X86(call ASM_X86_FUNC(VirtualFree));

  /* lib_path_size *= 2; */
  ASM_X86(shl dword ptr [ebp - 16], 1);

  /* Allocate space for the path. */
ASM_X86_LABEL(PayloadFunc_AllocPath)
  ASM_X86(push 0x4);                    /* PAGE_READWRITE */
  ASM_X86(push 0x3000);                 /* MEM_COMMIT | MEM_RESERVE */
  ASM_X86(push dword ptr [ebp - 16]);   /* lib_path_size */
  ASM_X86(push 0);                      /* NULL */
  ASM_X86(call ASM_X86_FUNC(VirtualAlloc));

  /* lib_path_ptr = VirtualAlloc(...); */
  ASM_X86(mov dword ptr [ebp - 20], eax);

  /* is_lib_resize_needed = 0; */
  ASM_X86(mov dword ptr [ebp - 12], 0);

  /* Suspend current thread until SGGL wakes it up. */
ASM_X86_LABEL(PayloadFunc_WaitForNextIteration)
  ASM_X86(push dword ptr [ebp - 8]);
  ASM_X86(call ASM_X86_FUNC(SuspendThread));

  /* Check if reallocation is needed. */
  ASM_X86(cmp dword ptr [ebp - 12], 0);
  ASM_X86(jne PayloadFunc_ReallocPath);

  /* Check num_libs and exit if no more libs left. */
  ASM_X86(cmp dword ptr [ebp - 4], 0);
  ASM_X86(je PayloadFunc_End);

  /* Load library. */
  ASM_X86(push dword ptr [ebp - 20]);
  ASM_X86(call ASM_X86_FUNC(LoadLibraryA));

  ASM_X86(dec dword ptr [ebp - 4]);
  ASM_X86(jmp PayloadFunc_WaitForNextIteration);

ASM_X86_LABEL(PayloadFunc_End)
  /* Free lib_path_ptr. */
  ASM_X86(push 0x800);                  /* MEM_RELEASE */
  ASM_X86(push 0);
  ASM_X86(push dword ptr [ebp - 20]);   /* lib_path_ptr */
  ASM_X86(call ASM_X86_FUNC(VirtualFree));

  /* Function epilogue */
  ASM_X86(add esp, 20);
  ASM_X86(popad);

  ASM_X86(ret 4);

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

  func_address_offset = (unsigned char*) (data_address + 4)
      - (size_t) patch_address
      - 5;

  memcpy(&buffer_patch->patch_buffer[1], data_address, sizeof(data_address));
  memcpy(
      &buffer_patch->patch_buffer[6],
      data_address + 4,
      sizeof(data_address)
  );

  return buffer_patch;
}

void EntryHijackPatch_Deinit(struct BufferPatch* buffer_patch) {
  BufferPatch_Deinit(buffer_patch);
}

struct BufferPatch* PayloadPatch_Init(
    struct BufferPatch* buffer_patch,
    void* (*patch_address)(void),
    const PROCESS_INFORMATION* process_info
) {
  /* Determine the number of op bytes in the entry hijack function. */
  if (payload_func_size == 0) {
    for (payload_func_size = 0; ; payload_func_size += 1) {
      if (memcmp(&PayloadFunc, kFuncEnd, sizeof(kFuncEnd)) == 0) {
        break;
      }
    }
  }

  BufferPatch_Init(
      buffer_patch,
      (void*) patch_address,
      payload_func_size,
      (void*) &PayloadFunc,
      process_info
  );

  return buffer_patch;
}

void PayloadPatch_Deinit(struct BufferPatch* buffer_patch) {
  BufferPatch_Deinit(buffer_patch);
}
