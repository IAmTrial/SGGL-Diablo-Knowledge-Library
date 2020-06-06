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

#include "stack_data.h"

#include <stdio.h>

#include "../helper/error_handling.h"

void StackData_InitFuncs(struct StackData* stack_data) {
  stack_data->LoadLibraryA_ptr = &LoadLibraryA;
  stack_data->GetCurrentThread_ptr = &GetCurrentThread;
  stack_data->SuspendThread_ptr = &SuspendThread;
  stack_data->VirtualAlloc_ptr = &VirtualAlloc;
  stack_data->VirtualFree_ptr = &VirtualFree;
}

void StackData_ReadFromProcess(
    struct StackData* stack_data,
    const PROCESS_INFORMATION* process_info,
    const void* stack_data_address
) {
  BOOL is_read_process_memory_success;
  size_t num_bytes_read;

  is_read_process_memory_success = ReadProcessMemory(
      process_info->hProcess,
      stack_data_address,
      stack_data,
      sizeof(*stack_data),
      &num_bytes_read
  );

  if (!is_read_process_memory_success) {
    printf("Read: %u \n", num_bytes_read);

    ExitOnWindowsFunctionFailureWithLastError(
        L"ReadProcessMemory",
        GetLastError()
    );
  }
}

void StackData_WriteToProcess(
    const struct StackData* stack_data,
    const PROCESS_INFORMATION* process_info,
    void* stack_data_address
) {
  BOOL is_write_process_memory_success;
  size_t num_bytes_written;

  is_write_process_memory_success = WriteProcessMemory(
      process_info->hProcess,
      stack_data_address,
      stack_data,
      sizeof(*stack_data),
      &num_bytes_written
  );

  if (!is_write_process_memory_success) {
    printf("Written: %u \n", num_bytes_written);

    ExitOnWindowsFunctionFailureWithLastError(
        L"WriteProcessMemory",
        GetLastError()
    );
  }
}
