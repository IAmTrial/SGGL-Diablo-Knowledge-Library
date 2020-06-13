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

#ifndef SGGLDKL_PATCH_HELPER_STACK_DATA_H_
#define SGGLDKL_PATCH_HELPER_STACK_DATA_H_

#include <stddef.h>
#include <windows.h>

/*
* This struct must be completely synced with the stack data in
* entry_hijack_patch->PayloadFunc. Note these values should be
* offset +4 from the description.
*
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
#pragma pack(push, 1)
struct StackData {
  unsigned int reserved_local_jump_offsets[(192 - 128) / 4];
  unsigned int reserved_kernel_func_ptr[(128 - 84) / 4];

  HMODULE (WINAPI *LoadLibraryA_ptr)(LPCSTR);
  HANDLE (WINAPI *GetCurrentThread_ptr)(void);
  DWORD (WINAPI *SuspendThread_ptr)(HANDLE);
  void* (WINAPI *VirtualAlloc_ptr)(void*, DWORD, DWORD, DWORD);
  BOOL (WINAPI *VirtualFree_ptr)(void*, DWORD, DWORD);

  unsigned int reserved_variable_ptr[(64 - 28) / 4];

  int is_ready_to_exit;
  int is_ready_to_execute;
  char* lib_path;
  size_t lib_path_size;
  int is_lib_resize_needed;
  DWORD current_thread_handle;
  size_t num_libs;
};
#pragma pack(pop)

void StackData_InitFuncs(struct StackData* stack_data);

void StackData_ReadFromProcess(
    struct StackData* stack_data,
    const PROCESS_INFORMATION* process_info,
    const void* stack_data_address
);

void StackData_WriteToProcess(
    const struct StackData* stack_data,
    const PROCESS_INFORMATION* process_info,
    void* stack_data_address
);

#endif /* SGGLDKL_PATCH_HELPER_STACK_DATA_H_ */
