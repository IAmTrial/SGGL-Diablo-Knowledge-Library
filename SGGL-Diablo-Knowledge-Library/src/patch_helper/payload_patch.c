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

#include "payload_patch.h"

#include <stddef.h>

#include "../asm_x86_macro.h"

static const unsigned char kFuncEnd[] = {
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
};

__declspec(naked) static void __stdcall PayloadFunc(void** top_of_stack) {
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
  * -24: is_ready_to_execute, can be modified by SGGL
  * -28: is_ready_to_exit, can be modified by SGGL
  * -32 to -64: reserved, for variables
  * -68: VirtualFree
  * -72: VirtualAlloc
  * -76: SuspendThread
  * -80: GetCurrentThread
  * -84: LoadLibraryA
  * -88 to -128: reserved, for kernel functions
  * -132 to -192: reserved, for local jump offsets
  */
  ASM_X86(sub esp, 192);

  /*
  * This must occur before the *top_of_stack is init to prevent
  * infinite loop race condition!
  */
  ASM_X86(mov dword ptr [ebp - 24], 0);

  /* *top_of_stack = esp; */
  ASM_X86(mov esi, dword ptr [ebp + 36]);
  ASM_X86(mov dword ptr [esi], esp);

ASM_X86_LABEL(SpinlockWaitForInitReady)
  ASM_X86(cmp dword ptr [ebp - 24], 0);
  ASM_X86(je SpinlockWaitForInitReady);

  /* is_lib_resize_needed = 1; */
  ASM_X86(mov dword ptr [ebp - 12], 1);

  /* is_ready_to_exit = 0; */
  ASM_X86(mov dword ptr [ebp - 28], 0);

  /* current_thread_handle = GetCurrentThread(); */
  ASM_X86(call dword ptr [ebp - 80]);
  ASM_X86(mov dword ptr [ebp - 8], eax);

  /* lib_path_size = 32; */
  ASM_X86(mov dword ptr [ebp - 16], 32);

  ASM_X86(jmp PayloadFunc_AllocPath);

ASM_X86_LABEL(PayloadFunc_ReallocPath)
  /* Free lib_path_ptr for reallocation. */
  ASM_X86(push 0x00000800);             /* MEM_RELEASE */
  ASM_X86(push 0);
  ASM_X86(push dword ptr [ebp - 20]);   /* lib_path_ptr */
  ASM_X86(call dword ptr [ebp - 68]);   /* VirtualFree(...); */

  /* lib_path_size *= 2; */
  ASM_X86(shl dword ptr [ebp - 16], 1);

  /* Allocate space for the path. */
ASM_X86_LABEL(PayloadFunc_AllocPath)
  ASM_X86(push 0x00000004);             /* PAGE_READWRITE */
  ASM_X86(push 0x00003000);             /* MEM_COMMIT | MEM_RESERVE */
  ASM_X86(push dword ptr [ebp - 16]);   /* lib_path_size */
  ASM_X86(push 0);                      /* NULL */
  ASM_X86(call dword ptr [ebp - 72]);   /* VirtualAlloc(...); */

  /* lib_path_ptr = VirtualAlloc(...); */
  ASM_X86(mov dword ptr [ebp - 20], eax);

  /* is_lib_resize_needed = 0; */
  ASM_X86(mov dword ptr [ebp - 12], 0);

  /* Suspend current thread until SGGL wakes it up. */
ASM_X86_LABEL(PayloadFunc_WaitForNextIteration)
  ASM_X86(push dword ptr [ebp - 8]);
  ASM_X86(call dword ptr [ebp - 76]);   /* SuspendThread(...); */

  /* Check if reallocation is needed. */
  ASM_X86(cmp dword ptr [ebp - 12], 0);
  ASM_X86(jne PayloadFunc_ReallocPath);

  /* Check num_libs and exit if no more libs left. */
  ASM_X86(cmp dword ptr [ebp - 4], 0);
  ASM_X86(je PayloadFunc_End);

  /* Load library. */
  ASM_X86(push dword ptr [ebp - 20]);
  ASM_X86(call dword ptr [ebp - 84]);   /* LoadLibraryA(...); */

  ASM_X86(dec dword ptr [ebp - 4]);
  ASM_X86(jmp PayloadFunc_WaitForNextIteration);

ASM_X86_LABEL(PayloadFunc_End)
  /* Free lib_path_ptr. */
  ASM_X86(push 0x00000800);             /* MEM_RELEASE */
  ASM_X86(push 0);
  ASM_X86(push dword ptr [ebp - 20]);   /* lib_path_ptr */
  ASM_X86(call dword ptr [ebp - 68]);   /* VirtualFree(...); */

  /*
  * Jump to the cleanup function. These are 5 bytes of dummies to
  * ensure there is space for the 4 byte jump op.
  */
  ASM_X86(int 3);
  ASM_X86(int 3);
  ASM_X86(int 3);
  ASM_X86(int 3);
  ASM_X86(int 3);

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
  const unsigned char* payload_func_bytes;
  int memcmp_result;

  payload_func_bytes = (const unsigned char*) &PayloadFunc;

  do {
    memcmp_result = memcmp(
        &payload_func_bytes[*func_size],
        kFuncEnd,
        sizeof(kFuncEnd)
    );

    *func_size += 1;
  } while (memcmp_result != 0);

  *func_size -= 1;
}

struct BufferPatch* PayloadPatch_Init(
    struct BufferPatch* payload_patch,
    void* (*patch_address)(void),
    void* (*cleanup_func_address)(void),
    const PROCESS_INFORMATION* process_info
) {
  unsigned char* cleanup_func_offset;
  size_t i_end_jmp_op;

  BufferPatch_Init(
      payload_patch,
      (void*) patch_address,
      PayloadPatch_GetSize(),
      (void*) &PayloadFunc,
      process_info
  );

  /* Set the last bytes of the ppatch buffer to jump to the cleanup function. */
  i_end_jmp_op = PayloadPatch_GetSize() - sizeof(void*) - 1;

  payload_patch->patch_buffer[i_end_jmp_op] = 0xE9;

  cleanup_func_offset = (unsigned char*) cleanup_func_address
      - (size_t) patch_address
      - PayloadPatch_GetSize();

  memcpy(
      &payload_patch->patch_buffer[i_end_jmp_op + 1],
      &cleanup_func_offset,
      sizeof(cleanup_func_offset)
  );

  return payload_patch;
}

void PayloadPatch_Deinit(struct BufferPatch* payload_patch) {
  BufferPatch_Deinit(payload_patch);
}

size_t PayloadPatch_GetSize(void) {
  static size_t payload_func_size = 0;

  if (payload_func_size == 0) {
    InitFuncSize(&payload_func_size);
  }

  return payload_func_size;
}
