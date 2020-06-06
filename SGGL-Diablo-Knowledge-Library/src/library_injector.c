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

#include "library_injector.h"

#include <stdio.h>

#include "game_version.h"
#include "game_version_enum.h"
#include "helper/encoding.h"
#include "helper/error_handling.h"
#include "patch_helper/buffer_patch.h"
#include "patch_helper/entry_hijack_patch.h"
#include "patch_helper/game_address.h"
#include "patch_helper/injector_patches.h"
#include "patch_helper/pe_header.h"
#include "patch_helper/stack_data.h"

static struct PeHeader pe_header;

static void WaitForProcessSuspend(const PROCESS_INFORMATION* process_info) {
  DWORD suspend_thread_result;
  DWORD resume_thread_result;

#if !NDEBUG
  printf(
      "Waiting for the process %u, thread %u to suspend. \n",
      process_info->dwProcessId,
      process_info->dwThreadId
  );
#endif /* !NDEBUG */

  do {
    /*
    * Reduce CPU usage and give the thread some time to execute code.
    */
    Sleep(15);

    suspend_thread_result = SuspendThread(process_info->hThread);

    if (suspend_thread_result == -1) {
      ExitOnWindowsFunctionFailureWithLastError(
          L"SuspendThread",
          GetLastError()
      );
    }

    resume_thread_result = ResumeThread(process_info->hThread);

    if (resume_thread_result == -1) {
      ExitOnWindowsFunctionFailureWithLastError(
          L"ResumeThread",
          GetLastError()
      );
    }
  } while (resume_thread_result == 1);

#if !NDEBUG
  printf(
      "Waiting successful for the process %u, thread %u. \n",
      process_info->dwProcessId,
      process_info->dwThreadId
  );
#endif /* !NDEBUG */
}

static int InjectLibrariesToProcess(
    const PROCESS_INFORMATION* process_info,
    size_t num_libraries,
    const wchar_t** libraries_to_inject,
    const size_t* libraries_to_inject_lens
) {
  enum FuncConstant {
    VIRTUAL_PROTECT_REGION_SIZE = 2048
  };

  size_t i_library;

  void* entry_point_address;
  DWORD old_entry_point_protect;

  void* stack_data_address;

  struct StackData stack_data_copy;
  struct StackData compare_stack_data_copy;
  int compare_stack_data_result;

  char* library_to_inject_mb;
  size_t library_to_inject_mb_size;

  struct InjectorPatches injector_patches;

  DWORD resume_thread_result;
  BOOL is_read_process_memory_success;
  BOOL is_write_process_memory_success;
  size_t num_bytes_read_write_process_memory;
  BOOL is_virtual_protect_ex_success;

  entry_point_address = PeHeader_GetHardEntryPointAddress(&pe_header);

  /*
  * Change the access protection of the entry point to enable write
  * and execute.
  */

#if !NDEBUG
  printf("Changing entry point memory access permissions. \n");
#endif /* NDEBUG */

  is_virtual_protect_ex_success = VirtualProtectEx(
      process_info->hProcess,
      entry_point_address,
      VIRTUAL_PROTECT_REGION_SIZE,
      PAGE_EXECUTE_READWRITE,
      &old_entry_point_protect
  );

  if (!is_virtual_protect_ex_success) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"VirtualProtectEx",
        GetLastError()
    );
  }

#if !NDEBUG
  printf("Successfully changed entry point memory access permissions. \n");
#endif /* NDEBUG */

  InjectorPatches_Init(
      &injector_patches,
      &pe_header,
      process_info
  );

  /* Patch the entry function and add the payload to the game. */
  BufferPatch_Apply(&injector_patches.entry_hijack_patch);
  BufferPatch_Apply(&injector_patches.payload_patch);

#if !NDEBUG
  printf("Attach a debugger to the game process and then press enter. \n");
  getc(stdin);
#endif /* !NDEBUG */

  /* Resume game thread to get the game to the payload checkpoint. */
  resume_thread_result = ResumeThread(process_info->hThread);

  if (resume_thread_result == -1) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"ResumeThread",
        GetLastError()
    );
  }

  /*
  * Get the stack address. Runs in an spinlock because SuspendThread
  * is not yet available in the payload function.
  */
  stack_data_address = NULL;
  do {
    is_read_process_memory_success = ReadProcessMemory(
        process_info->hProcess,
        EntryHijackPatch_GetFreeSpaceAddress(
            &injector_patches.entry_hijack_patch
        ),
        &stack_data_address,
        sizeof(stack_data_address),
        &num_bytes_read_write_process_memory
    );

    if (!is_read_process_memory_success) {
      printf("Read: %u \n", num_bytes_read_write_process_memory);

      ExitOnWindowsFunctionFailureWithLastError(
          L"ReadProcessMemory",
          GetLastError()
      );
    }
  } while (stack_data_address == NULL);

#if !NDEBUG
  printf("Stack data address: %p \n", stack_data_address);
#endif /* NDEBUG */

  /* Read the initial stack data. */
  StackData_ReadFromProcess(
      &stack_data_copy,
      process_info,
      stack_data_address
  );

  /* Init the stack data. */
  stack_data_copy.num_libs = num_libraries;
  StackData_InitFuncs(&stack_data_copy);

  StackData_WriteToProcess(
      &stack_data_copy,
      process_info,
      stack_data_address
  );

  /*
  * Apply the cleanup patch, now that the game process is no longer in
  * the vanilla code space.
  */
  BufferPatch_Apply(&injector_patches.cleanup_patch);

  /*
  * End spinlock, which will resume execution. This needs to happen
  * the stack data initialization after due to partial write race
  * condition.
  */
  stack_data_copy.is_ready_to_execute = 1;

  StackData_WriteToProcess(
      &stack_data_copy,
      process_info,
      stack_data_address
  );

  /* Inject every library. */
  for (i_library = 0; i_library < num_libraries; i_library += 1) {

#if !NDEBUG
    printf("Injecting: %ls \n", libraries_to_inject[i_library]);
#endif /* NDEBUG */

    /* Since LoadLibraryA is being used, convert the string to multibyte. */
    library_to_inject_mb = ConvertWideToMultibyte(
        NULL,
        libraries_to_inject[i_library]
    );

#if !NDEBUG
    printf("Converted string: %s \n", library_to_inject_mb);
#endif /* NDEBUG */

    /* Check that the process has suspended itself. */
    WaitForProcessSuspend(process_info);

    /* If the buffer size is insufficient, then force the data to resize. */
    library_to_inject_mb_size = (libraries_to_inject_lens[i_library] + 1)
        * sizeof(library_to_inject_mb[0]);

    StackData_ReadFromProcess(
        &stack_data_copy,
        process_info,
        stack_data_address
    );

    while (stack_data_copy.lib_path_size < library_to_inject_mb_size) {

#if !NDEBUG
      printf(
          "Requesting lib path resize; requires size of %u, got %u \n",
          library_to_inject_mb_size,
          stack_data_copy.lib_path_size
      );
#endif /* NDEBUG */

      stack_data_copy.is_lib_resize_needed = 1;

      StackData_WriteToProcess(
          &stack_data_copy,
          process_info,
          stack_data_address
      );

      resume_thread_result = ResumeThread(process_info->hThread);

      if (resume_thread_result == -1) {
        ExitOnWindowsFunctionFailureWithLastError(
            L"ResumeThread",
            GetLastError()
        );
      }

      WaitForProcessSuspend(process_info);

      StackData_ReadFromProcess(
          &stack_data_copy,
          process_info,
          stack_data_address
      );
    }

#if !NDEBUG
    printf("Library path address: %p \n", stack_data_copy.lib_path);
    printf("Library path size: %u \n", stack_data_copy.lib_path_size);
#endif /* !NDEBUG */

    /*
    * Size is sufficient, so copy the library's path to the process's
    * free space.
    */

#if !NDEBUG
    printf("Writing to VirtualAlloc memory. \n");
#endif /* !NDEBUG */

    is_write_process_memory_success = WriteProcessMemory(
        process_info->hProcess,
        stack_data_copy.lib_path,
        library_to_inject_mb,
        library_to_inject_mb_size,
        NULL
    );

    if (!is_write_process_memory_success) {
      ExitOnWindowsFunctionFailureWithLastError(
          L"WriteProcessMemory",
          GetLastError()
      );
    }

#if !NDEBUG
    printf("Successfully written to VirtualAlloc memory. \n");
#endif /* !NDEBUG */

free_library_to_inject_mb:
    free(library_to_inject_mb);

    /* Library path has been copied, so resume the thread. */
    resume_thread_result = ResumeThread(process_info->hThread);

    if (resume_thread_result == -1) {
      ExitOnWindowsFunctionFailureWithLastError(
          L"ResumeThread",
          GetLastError()
      );
    }
  }

  /*
  * Wait for process to make one suspend stop, then wait for process
  * to jump to the cleanup func space.
  */
  WaitForProcessSuspend(process_info);

  resume_thread_result = ResumeThread(process_info->hThread);

  if (resume_thread_result == -1) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"ResumeThread",
        GetLastError()
    );
  }

  WaitForProcessSuspend(process_info);

  /* Read the current state of the stack for a later comparison. */
  StackData_ReadFromProcess(
      &stack_data_copy,
      process_info,
      stack_data_address
  );

  /* Restore the original code of the entry hijack and the payload. */
  BufferPatch_Remove(&injector_patches.payload_patch);
  BufferPatch_Remove(&injector_patches.entry_hijack_patch);

  /* Resume game thread, which will allow the game to continue like normal. */
  resume_thread_result = ResumeThread(process_info->hThread);

  if (resume_thread_result == -1) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"ResumeThread",
        GetLastError()
    );
  }

  /*
  * Infinite loop read the stack values and determine if the values
  * are no longer the same. This guarantees that the program is no
  * longer in the cleanup space and the original code can now be
  * restored.
  */
  do {
    StackData_ReadFromProcess(
        &compare_stack_data_copy,
        process_info,
        stack_data_address
    );

    compare_stack_data_result = memcmp(
        &compare_stack_data_copy,
        &stack_data_copy,
        sizeof(stack_data_copy)
    );
  } while (compare_stack_data_result == 0);

  BufferPatch_Remove(&injector_patches.cleanup_patch);

  /* Cleanup the patches. */
  InjectorPatches_Deinit(&injector_patches);

  /* Restore the access protection of the entry point. */

#if !NDEBUG
  printf("Restoring entry point memory access permissions. \n");
#endif /* !NDEBUG */

  is_virtual_protect_ex_success = VirtualProtectEx(
      process_info->hProcess,
      entry_point_address,
      VIRTUAL_PROTECT_REGION_SIZE,
      old_entry_point_protect,
      &old_entry_point_protect
  );

  if (!is_virtual_protect_ex_success) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"VirtualProtectEx",
        GetLastError()
    );
  }

#if !NDEBUG
  printf("Successfully restored entry point memory access permissions. \n");
#endif /* !NDEBUG */

  return 1;
}

void InitLibraryInjector(const wchar_t* game_path, size_t game_path_len) {
  PeHeader_Init(&pe_header, game_path, game_path_len);
}

void DeinitLibraryInjector(void) {
  PeHeader_Deinit(&pe_header);
}

int InjectLibraries(
    const wchar_t** libraries_to_inject,
    size_t num_libraries,
    const PROCESS_INFORMATION* processes_infos,
    size_t num_instances
) {
  size_t i_library;
  size_t i_process;

  enum GameVersion running_game_version = GetRunningGameVersion();

  size_t* libraries_to_inject_lens;

  unsigned char is_all_success;
  unsigned char is_current_success;

  /* Determine the lengths of the libraries to inject. */
  libraries_to_inject_lens = malloc(
      num_libraries * sizeof(libraries_to_inject_lens[0])
  );

  if (libraries_to_inject_lens == NULL) {
    return 0;
  }

  for (i_library = 0; i_library < num_libraries; i_library += 1) {
    libraries_to_inject_lens[i_library] = wcslen(
        libraries_to_inject[i_library]
    );
  }

  /* Inject libraries into each process. */
  is_all_success = 1;

  for (i_process = 0; i_process < num_instances; i_process += 1) {
    is_current_success = InjectLibrariesToProcess(
        &processes_infos[i_process],
        num_libraries,
        libraries_to_inject,
        libraries_to_inject_lens
    );

    is_all_success = is_all_success && is_current_success;
  }

free_libraries_to_inject_lens:
  free(libraries_to_inject_lens);

  return 1;
}
