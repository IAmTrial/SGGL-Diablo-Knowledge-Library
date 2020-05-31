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
  * -24: is_ready_to_execute, can be modified by SGGL
  * -28 to -64: reserved, for variables
  * -68: VirtualFree
  * -72: VirtualAlloc
  * -76: SuspendThread
  * -80: GetCurrentThread
  * -84: LoadLibraryA
  * -88 to -128: reserved, for kernel functions
  */
  ASM_X86(sub esp, 128);

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

  /* current_thread_handle = GetCurrentThread(); */
  ASM_X86(call dword ptr [ebp - 80]);
  ASM_X86(mov dword ptr [ebp - 8], eax);

  /* lib_path_size = 32; */
  ASM_X86(mov dword ptr [ebp - 16], 32);

  ASM_X86(jmp PayloadFunc_AllocPath);

ASM_X86_LABEL(PayloadFunc_ReallocPath)
  /* Free lib_path_ptr for reallocation. */
  ASM_X86(push 0x800);                  /* MEM_RELEASE */
  ASM_X86(push 0);
  ASM_X86(push dword ptr [ebp - 20]);   /* lib_path_ptr */
  ASM_X86(call dword ptr [ebp - 68]);   /* VirtualFree(...); */

  /* lib_path_size *= 2; */
  ASM_X86(shl dword ptr [ebp - 16], 1);

  /* Allocate space for the path. */
ASM_X86_LABEL(PayloadFunc_AllocPath)
  ASM_X86(push 0x4);                    /* PAGE_READWRITE */
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

  /* Function epilogue */
  ASM_X86(add esp, 128);
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

struct BufferPatch* PayloadPatch_Init(
    struct BufferPatch* buffer_patch,
    void* (*patch_address)(void),
    const PROCESS_INFORMATION* process_info
) {
  unsigned char* payload_func_bytes = &PayloadFunc;

  int memcpy_result;

  /* Determine the number of op bytes in the entry hijack function. */
  if (payload_func_size == 0) {
    for (payload_func_size = 0; ; payload_func_size += 1) {
      memcpy_result = memcmp(
          &payload_func_bytes[payload_func_size],
          kFuncEnd,
          sizeof(kFuncEnd)
      );

      if (memcpy_result == 0) {
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
