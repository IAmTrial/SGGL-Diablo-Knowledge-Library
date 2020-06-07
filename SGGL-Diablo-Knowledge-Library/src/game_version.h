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

#ifndef SGGLDKL_GAME_VERSION_H_
#define SGGLDKL_GAME_VERSION_H_

#include <stddef.h>
#include <wchar.h>

enum GameVersion {
  VERSION_UNKNOWN = -1,

  DIABLO_1_00 = 1, DIABLO_1_02, DIABLO_1_03, DIABLO_1_04, DIABLO_1_05,
  DIABLO_1_07, DIABLO_1_08, DIABLO_1_09, DIABLO_1_09B,

  HELLFIRE_1_00, HELLFIRE_1_01,

  DIABLO_II_BETA_1_02, DIABLO_II_STRESS_TEST_BETA_1_02,
  DIABLO_II_1_00, DIABLO_II_1_01, DIABLO_II_1_02, DIABLO_II_1_03,
  DIABLO_II_1_04, DIABLO_II_1_04B, DIABLO_II_1_04C, DIABLO_II_1_05,
  DIABLO_II_1_05B, DIABLO_II_1_06, DIABLO_II_1_06B,

  DIABLO_II_1_07_BETA, DIABLO_II_1_07, DIABLO_II_1_08, DIABLO_II_1_09,
  DIABLO_II_1_09B, DIABLO_II_1_09C, DIABLO_II_1_09D, DIABLO_II_1_10_BETA,
  DIABLO_II_1_10S_BETA, DIABLO_II_1_10, DIABLO_II_1_11, DIABLO_II_1_11B,
  DIABLO_II_1_12A, DIABLO_II_1_13A_PTR, DIABLO_II_1_13C, DIABLO_II_1_13D,
  DIABLO_II_1_14A, DIABLO_II_1_14B, DIABLO_II_1_14C, DIABLO_II_1_14D
};

enum GameVersion GameVersion_DetermineRunningGameVersion(
    const wchar_t* game_path,
    size_t game_path_len
);

#endif /* SGGLDKL_GAME_VERSION_H_ */
