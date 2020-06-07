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

#include "injector_patches.h"

#include <stdio.h>

#include "../game_version.h"
#include "cleanup_patch.h"
#include "entry_hijack_patch.h"
#include "game_address.h"
#include "payload_patch.h"
#include "pe_header.h"

struct InjectorPatches* InjectorPatches_Init(
    struct InjectorPatches* injector_patches,
    const struct PeHeader* pe_header,
    const PROCESS_INFORMATION* process_info,
    enum GameVersion game_version
) {
  void* (*cleanup_patch_address)(void);

  unsigned char* entry_hijack_patch_address;
  unsigned char* payload_patch_address;

  cleanup_patch_address = PeHeader_GetHardEntryPointAddress(pe_header);

  CleanupPatch_Init(
      &injector_patches->cleanup_patch,
      cleanup_patch_address,
      process_info
  );

  entry_hijack_patch_address = GetEntryHijackPatchAddress(
      pe_header,
      game_version
  );

#if !NDEBUG
  printf("Entry hijack patch address: %p \n", entry_hijack_patch_address);
#endif /* !NDEBUG */

  EntryHijackPatch_Init(
      &injector_patches->entry_hijack_patch,
      (void* (*)(void)) entry_hijack_patch_address,
      process_info,
      pe_header
  );

  payload_patch_address =
      (unsigned char*) injector_patches->entry_hijack_patch.position
          + EntryHijackPatch_GetSize();

  PayloadPatch_Init(
      &injector_patches->payload_patch,
      (void* (*)(void)) payload_patch_address,
      cleanup_patch_address,
      process_info
  );

  return injector_patches;
}

void InjectorPatches_Deinit(struct InjectorPatches* injector_patches) {
  PayloadPatch_Deinit(&injector_patches->payload_patch);
  EntryHijackPatch_Deinit(&injector_patches->entry_hijack_patch);
  CleanupPatch_Deinit(&injector_patches->cleanup_patch);
}
