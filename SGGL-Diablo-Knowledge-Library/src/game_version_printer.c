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

#include "game_version_printer.h"

#include <stdio.h>

#include "helper/error_handling.h"
#include "game_version.h"

static const char* GetGameName(enum GameVersion game_version) {
  if (game_version >= DIABLO_1_00 && game_version <= DIABLO_1_09B) {
    return "Diablo";
  } else if (game_version >= HELLFIRE_1_00
      && game_version <= HELLFIRE_1_01) {
    return "Hellfire";
  } else if (game_version >= DIABLO_II_BETA_1_02
      && game_version <= DIABLO_II_1_14D) {
    return "Diablo II";
  } else if (game_version == VERSION_UNKNOWN) {
    return "Unknown game";
  } else {
    ExitOnGeneralFailure(
        L"Invalid game version state.",
        L"Error"
    );

    return NULL;
  }
}

static const char* GetVersionText(enum GameVersion game_version) {
  switch (game_version) {
    case DIABLO_1_00: {
      return "1.00";
    }

    case DIABLO_1_02: {
      return "1.02";
    }

    case DIABLO_1_03: {
      return "1.03";
    }

    case DIABLO_1_04: {
      return "1.04";
    }

    case DIABLO_1_05: {
      return "1.05";
    }

    case DIABLO_1_07: {
      return "1.07";
    }

    case DIABLO_1_08: {
      return "1.08";
    }

    case DIABLO_1_09: {
      return "1.09";
    }

    case DIABLO_1_09B: {
      return "1.09B";
    }

    case HELLFIRE_1_00: {
      return "1.00";
    }

    case HELLFIRE_1_01: {
      return "1.01";
    }

    case DIABLO_II_BETA_1_02: {
      return "Beta 1.02";
    }

    case DIABLO_II_STRESS_TEST_BETA_1_02: {
      return "Beta Stress Test 1.02";
    }

    case DIABLO_II_1_00: {
      return "1.00";
    }

    case DIABLO_II_1_01: {
      return "1.01";
    }

    case DIABLO_II_1_02: {
      return "1.02";
    }

    case DIABLO_II_1_03: {
      return "1.03";
    }

    case DIABLO_II_1_04: {
      return "1.04";
    }

    case DIABLO_II_1_04B: {
      return "1.04B";
    }

    case DIABLO_II_1_04C: {
      return "1.04C";
    }

    case DIABLO_II_1_05: {
      return "1.05";
    }

    case DIABLO_II_1_05B: {
      return "1.05B";
    }

    case DIABLO_II_1_06: {
      return "1.06";
    }

    case DIABLO_II_1_06B: {
      return "1.06B";
    }

    case DIABLO_II_1_07_BETA: {
      return "1.07 Beta";
    }

    case DIABLO_II_1_07: {
      return "1.07";
    }

    case DIABLO_II_1_08: {
      return "1.08";
    }

    case DIABLO_II_1_09: {
      return "1.09";
    }

    case DIABLO_II_1_09B: {
      return "1.09B";
    }

    case DIABLO_II_1_09C: {
      return "1.09C";
    }

    case DIABLO_II_1_09D: {
      return "1.09D";
    }

    case DIABLO_II_1_10_BETA: {
      return "1.10 Beta";
    }

    case DIABLO_II_1_10S_BETA: {
      return "1.10S Beta";
    }

    case DIABLO_II_1_10: {
      return "1.10";
    }

    case DIABLO_II_1_11: {
      return "1.11";
    }

    case DIABLO_II_1_11B: {
      return "1.11B";
    }

    case DIABLO_II_1_12A: {
      return "1.12A";
    }

    case DIABLO_II_1_13A_PTR: {
      return "1.13A";
    }

    case DIABLO_II_1_13C: {
      return "1.13C";
    }

    case DIABLO_II_1_13D: {
      return "1.13D";
    }

    case DIABLO_II_1_14A: {
      return "1.14A";
    }

    case DIABLO_II_1_14B: {
      return "1.14B";
    }

    case DIABLO_II_1_14C: {
      return "1.14C";
    }

    case DIABLO_II_1_14D: {
      return "1.14D";
    }

    default: {
      return "Invalid";
    }
  }
}

void PrintGameVersion(enum GameVersion game_version) {
  const char* game_name;
  const char* game_version_text;

  game_name = GetGameName(game_version);
  game_version_text = GetVersionText(game_version);

  printf("Game information: \n");
  printf("%s %s \n\n", game_name, game_version_text);
}
