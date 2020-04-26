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

#include "diablo_game_version.h"

enum GameVersion Diablo_DetermineGameVersion(
    const VS_FIXEDFILEINFO* diablo_file_info,
    const VS_FIXEDFILEINFO* storm_file_info
) {
  DWORD diablo_product_version_major_num_left =
      (diablo_file_info->dwProductVersionMS >> 16) & 0xFFFF;
  DWORD diablo_product_version_major_num_right =
      (diablo_file_info->dwProductVersionMS >> 0) & 0xFFFF;
  DWORD diablo_product_version_minor_num_left =
      (diablo_file_info->dwProductVersionLS >> 16) & 0xFFFF;
  DWORD diablo_product_version_minor_num_right =
      (diablo_file_info->dwProductVersionLS >> 0) & 0xFFFF;

  DWORD storm_file_version_major_num_left =
      (storm_file_info->dwFileVersionMS >> 16) & 0xFFFF;
  DWORD storm_file_version_major_num_right =
      (storm_file_info->dwFileVersionMS >> 0) & 0xFFFF;
  DWORD storm_file_version_minor_num_left =
      (storm_file_info->dwFileVersionLS >> 16) & 0xFFFF;
  DWORD storm_file_version_minor_num_right =
      (storm_file_info->dwFileVersionLS >> 0) & 0xFFFF;

  if (diablo_product_version_major_num_left == 96) {
    if (diablo_product_version_major_num_right == 12) {
      if (diablo_product_version_minor_num_left == 26) {
        if (diablo_product_version_minor_num_right == 3) {
          /* 96, 12, 26, 3 */
          return DIABLO_1_00;
        }
      }
    }
  } else if (diablo_product_version_major_num_left == 97) {
    if (diablo_product_version_major_num_right == 4) {
      if (diablo_product_version_minor_num_left == 1) {
        if (diablo_product_version_minor_num_right == 1) {
          /* 97, 4, 1, 1 */
          return DIABLO_1_03;
        }
      }
    } else if (diablo_product_version_major_num_right == 5) {
      if (diablo_product_version_minor_num_left == 23) {
        if (diablo_product_version_minor_num_right == 1) {
          /* 97, 5, 23, 1 */
          return DIABLO_1_04;
        }
      }
    }
  }

  if (storm_file_version_major_num_left == 1998) {
    if (storm_file_version_major_num_right == 4) {
      if (storm_file_version_minor_num_left == 15) {
        if (storm_file_version_minor_num_right == 1) {
          /* 1998.4.15.1 */
          return DIABLO_1_05;
        }
      }
    } else if (storm_file_version_major_num_right == 8) {
      if (storm_file_version_minor_num_left == 11) {
        if (storm_file_version_minor_num_right == 1) {
          /* 1998.8.11.1 */
          return DIABLO_1_07;
        }
      }
    }
  }

  if (diablo_product_version_major_num_left == 1) {
    if (diablo_product_version_major_num_right == 0) {
      if (diablo_product_version_minor_num_left == 8) {
        if (diablo_product_version_minor_num_right == 1) {
          /* 1, 0, 8, 1 */
          return DIABLO_1_08;
        }
      } else if (diablo_product_version_minor_num_left == 9) {
        if (diablo_product_version_minor_num_right == 1) {
          /* 1, 0, 9, 1 */
          return DIABLO_1_09;
        } else if (diablo_product_version_minor_num_right == 2) {
          /* 1, 0, 9, 2 */
          return DIABLO_1_09B;
        }
      }
    }
  }

  return VERSION_UNKNOWN;
}
