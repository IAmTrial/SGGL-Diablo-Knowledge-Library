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

#include "game_address.h"

#include "../diablo/diablo_address.h"
#include "../diablo_ii/diablo_ii_address.h"
#include "../hellfire/hellfire_address.h"

void* GetEntryHijackPatchAddress(
    const struct PeHeader* pe_header,
    enum GameVersion game_version
) {
  /* Version starting from 1.14A don't work on Windows 9X. */
  if (game_version >= DIABLO_II_1_14A) {
    return 0;
  }

  switch (game_version) {
    case DIABLO_1_00: {
      return Diablo_1_00_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_1_02: {
      return Diablo_1_02_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_1_03: {
      return Diablo_1_03_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_1_04: {
      return Diablo_1_04_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_1_05: {
      return Diablo_1_05_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_1_07: {
      return Diablo_1_07_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_1_08: {
      return Diablo_1_08_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_1_09: {
      return Diablo_1_09_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_1_09B: {
      return Diablo_1_09B_GetEntryHijackPatchAddress(pe_header);
    }

    case HELLFIRE_1_00: {
      return Hellfire_1_00_GetEntryHijackPatchAddress(pe_header);
    }

    case HELLFIRE_1_01: {
      return Hellfire_1_01_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_BETA_1_02: {
      return Diablo_II_Beta1_02_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_STRESS_TEST_BETA_1_02: {
      return Diablo_II_BetaStressTest1_02_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_00: {
      return Diablo_II_1_00_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_01: {
      return Diablo_II_1_01_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_02: {
      return Diablo_II_1_02_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_03: {
      return Diablo_II_1_03_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_04B: {
      return Diablo_II_1_04B_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_04C: {
      return Diablo_II_1_04C_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_05: {
      return Diablo_II_1_05_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_05B: {
      return Diablo_II_1_05B_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_06: {
      return Diablo_II_1_06_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_06B: {
      return Diablo_II_1_06B_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_07_BETA: {
      return Diablo_II_1_07Beta_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_07: {
      return Diablo_II_1_07_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_08: {
      return Diablo_II_1_08_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_09: {
      return Diablo_II_1_09_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_09B: {
      return Diablo_II_1_09B_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_09D: {
      return Diablo_II_1_09D_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_10_BETA: {
      return Diablo_II_1_10Beta_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_10S_BETA: {
      return Diablo_II_1_10SBeta_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_10: {
      return Diablo_II_1_10_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_11: {
      return Diablo_II_1_11_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_11B: {
      return Diablo_II_1_11B_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_12A: {
      return Diablo_II_1_12A_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_13A_PTR: {
      return Diablo_II_1_13APtr_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_13C: {
      return Diablo_II_1_13C_GetEntryHijackPatchAddress(pe_header);
    }

    case DIABLO_II_1_13D: {
      return Diablo_II_1_13D_GetEntryHijackPatchAddress(pe_header);
    }

    default: {
      return 0;
    }
  }
}
