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

#include "diablo_ii/diablo_ii_address.h"
#include "game_version.h"
#include "game_version_enum.h"
#include "helper/encoding.h"
#include "helper/error_handling.h"
#include "patch_helper/buffer_patch.h"
#include "patch_helper/entry_hijack_patch.h"
#include "patch_helper/pe_header.h"

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
* -28 to -64: reserved, for variables
* -68: VirtualFree
* -72: VirtualAlloc
* -76: SuspendThread
* -80: GetCurrentThread
* -84: LoadLibraryA
* -88 to -128: reserved, for kernel functions
*/
#pragma pack(push, 1)
struct StackData {
  unsigned int reserved_kernel_func_ptr[(132 - 88) / 4];

  HMODULE (WINAPI *LoadLibraryA_ptr)(LPCSTR);
  HANDLE (WINAPI *GetCurrentThread_ptr)(void);
  DWORD (WINAPI *SuspendThread_ptr)(HANDLE);
  void* (WINAPI *VirtualAlloc_ptr)(void*, DWORD, DWORD, DWORD);
  BOOL (WINAPI *VirtualFree_ptr)(void*, DWORD, DWORD);

  unsigned int reserved_variable_ptr[(64 - 24) / 4];

  int is_ready_to_execute;
  unsigned char* lib_path_ptr;
  size_t lib_path_size;
  int is_lib_resize_needed;
  DWORD current_thread_handle;
  size_t num_libs;
};
#pragma pack(pop)

static struct PeHeader pe_header;

static void* GetEntryHijackPatchAddress(
    const struct PeHeader* pe_header
) {
  enum GameVersion running_game_version;

  running_game_version = GetRunningGameVersion();

  /* Version starting from 1.14A don't work on Windows 9X. */
  if (running_game_version >= DIABLO_II_1_14A
      && running_game_version <= DIABLO_II_1_14D) {
    return 0;
  }

  switch (running_game_version) {
    case DIABLO_II_1_13C: {
      return Diablo_II_1_13C_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_13D: {
      return Diablo_II_1_13D_GetEntryHijackPatchAddress(pe_header);
    }
  }
}

static void WaitForProcessSuspend(const PROCESS_INFORMATION* process_info) {
  DWORD suspend_thread_result;
  DWORD resume_thread_result;

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
  } while (resume_thread_result == 0);
}

static void StackData_InitFuncs(struct StackData* stack_data) {
  stack_data->LoadLibraryA_ptr = &LoadLibraryA;
  stack_data->GetCurrentThread_ptr = &GetCurrentThread;
  stack_data->SuspendThread_ptr = &SuspendThread;
  stack_data->VirtualAlloc_ptr = &VirtualAlloc;
  stack_data->VirtualFree_ptr = &VirtualFree;
}

static int InjectLibrariesToProcess(
    const PROCESS_INFORMATION* process_info,
    size_t num_libraries,
    const wchar_t** libraries_to_inject,
    const size_t* libraries_to_inject_lens
) {
  const unsigned char kNullBuffer[] = { 0x00, 0x00, 0x00, 0x00 };

  size_t i_library;

  void* data_address;
  void* entry_hijack_patch_address;
  void* stack_data_address;
  DWORD old_data_address_protect;
  DWORD old_stack_data_protect;
  struct StackData stack_data_copy;
  char* library_to_inject_mb;
  size_t library_to_inject_mb_size;
  size_t num_bytes_read_write_process_memory;

  struct BufferPatch entry_hijack_patch;
  struct BufferPatch payload_patch;
  struct BufferPatch space_data_patch;

  DWORD resume_thread_result;
  BOOL is_read_process_memory_success;
  BOOL is_write_process_memory_success;
  BOOL is_virtual_protect_ex_success;

  printf("%u \n", sizeof(stack_data_copy));

  /* Get the address of the data and the address of the patch location. */
  data_address = PeHeader_GetHardDataAddress(&pe_header);
  entry_hijack_patch_address = GetEntryHijackPatchAddress(&pe_header);

  /* Change the access protection of the data to enable write and execute. */
  is_virtual_protect_ex_success = VirtualProtectEx(
      process_info->hProcess,
      data_address,
      1024,
      PAGE_EXECUTE_READWRITE,
      &old_data_address_protect
  );

  if (!is_virtual_protect_ex_success) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"VirtualProtectEx",
        GetLastError()
    );
  }

  /* Initialize the patches. */
  PayloadPatch_Init(
      &payload_patch,
      (void* (*)(void)) ((unsigned char*) data_address + sizeof(void*)),
      process_info
  );

  BufferPatch_Init(
      &null_data_patch,
      (void*) entry_hijack_patch_address,
      sizeof(data_address),
      kNullBuffer,
      process_info
  );

  /*
  * Entry hijack comes last, because we want to deinit it first so the
  * original game code execution can be resumed.
  */
  EntryHijackPatch_Init(
      &entry_hijack_patch,
      (void* (*)(void)) entry_hijack_patch_address,
      process_info,
      &pe_header
  );

  /* Patch the entry function and add the payload to the game. */
  BufferPatch_Apply(&entry_hijack_patch);
  BufferPatch_Apply(&payload_patch);
  BufferPatch_Apply(&null_data_patch);


  /* Resume game thread. */
  resume_thread_result = ResumeThread(process_info->hThread);

  if (resume_thread_result == -1) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"ResumeThread",
        GetLastError()
    );
  }


  /* Get the stack address. */
  stack_data_address = NULL;
  do {
    is_read_process_memory_success = ReadProcessMemory(
        process_info->hProcess,
        data_address,
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

  MessageBoxA(0, "mnbnmnb", "", MB_OK);

  /*
  * Change the access protection of the stack data to enable write and
  * execute.
  */
  is_virtual_protect_ex_success = VirtualProtectEx(
      process_info->hProcess,
      stack_data_address,
      1024,
      PAGE_READWRITE,
      &old_stack_data_protect
  );


  /* Read the initial stack data. */
  is_read_process_memory_success = ReadProcessMemory(
      process_info->hProcess,
      stack_data_address,
      &stack_data_copy,
      sizeof(stack_data_copy),
      &num_bytes_read_write_process_memory
  );

  if (!is_read_process_memory_success) {
    printf("Read: %u \n", num_bytes_read_write_process_memory);

    ExitOnWindowsFunctionFailureWithLastError(
        L"ReadProcessMemory",
        GetLastError()
    );
  }

  /* Init the stack data. */
  stack_data_copy.num_libs = num_libraries;
  stack_data_copy.is_ready_to_execute = 1;
  StackData_InitFuncs(&stack_data_copy);

  is_write_process_memory_success = WriteProcessMemory(
      process_info->hProcess,
      stack_data_address,
      &stack_data_copy,
      sizeof(stack_data_copy),
      &num_bytes_read_write_process_memory
  );

  if (!is_write_process_memory_success) {
    printf("Written: %u \n", num_bytes_read_write_process_memory);

    ExitOnWindowsFunctionFailureWithLastError(
        L"WriteProcessMemory",
        GetLastError()
    );
  }

  /* Inject every library. */
  for (i_library = 0; i_library < num_libraries; i_library += 1) {
    /* Since LoadLibraryA is being used, convert the string to multibyte. */
    library_to_inject_mb = ConvertWideToMultibyte(
        NULL,
        libraries_to_inject[i_library]
    );

    /* Check that the process has suspended itself. */
    WaitForProcessSuspend(process_info);

    /* If the buffer size is insufficient, then force the data to resize. */
    library_to_inject_mb_size = (libraries_to_inject_lens[i_library] + 1)
        * sizeof(library_to_inject_mb[0]);

    while (stack_data_copy.lib_path_size < library_to_inject_mb_size) {
      stack_data_copy.is_lib_resize_needed = 1;

      resume_thread_result = ResumeThread(process_info->hThread);

      if (resume_thread_result == -1) {
        ExitOnWindowsFunctionFailureWithLastError(
            L"ResumeThread",
            GetLastError()
        );
      }

      WaitForProcessSuspend(process_info);

      is_read_process_memory_success = ReadProcessMemory(
          process_info->hProcess,
          stack_data_address,
          &stack_data_copy,
          sizeof(stack_data_copy),
          NULL
      );

      if (!is_read_process_memory_success) {
        ExitOnWindowsFunctionFailureWithLastError(
            L"ReadProcessMemory",
            GetLastError()
        );
      }
    }

    /*
    * Size is sufficient, so copy the library's path to the process's
    * free space.
    */
    is_write_process_memory_success = WriteProcessMemory(
        process_info->hProcess,
        stack_data_copy.lib_path_ptr,
        library_to_inject_mb,
        (libraries_to_inject_lens[i_library] + 1)
            * sizeof(library_to_inject_mb[0]),
        NULL
    );

    if (!is_write_process_memory_success) {
      ExitOnWindowsFunctionFailureWithLastError(
          L"WriteProcessMemory",
          GetLastError()
      );
    }

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

  /* Restore the access protection of the stack data. */
  is_virtual_protect_ex_success = VirtualProtectEx(
      process_info->hProcess,
      data_address,
      1024,
      old_stack_data_protect,
      &old_stack_data_protect
  );

  if (!is_virtual_protect_ex_success) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"VirtualProtectEx",
        GetLastError()
    );
  }

  /* Restore the entry code to the original. */
deinit_entry_hijack_patch:
  EntryHijackPatch_Deinit(&entry_hijack_patch);

  ResumeThread(process_info->hThread);

  /* Cleanup remaining patches. */
deinit_null_data_patch:
  BufferPatch_Deinit(&null_data_patch);

deinit_payload_patch:
  PayloadPatch_Deinit(&payload_patch);

  /* Restore the access protection of the data address. */
  is_virtual_protect_ex_success = VirtualProtectEx(
      process_info->hProcess,
      data_address,
      1024,
      old_data_address_protect,
      &old_data_address_protect
  );

  if (!is_virtual_protect_ex_success) {
    ExitOnWindowsFunctionFailureWithLastError(
        L"VirtualProtectEx",
        GetLastError()
    );
  }

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
