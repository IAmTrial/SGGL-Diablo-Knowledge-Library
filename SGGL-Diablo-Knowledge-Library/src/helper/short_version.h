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

#ifndef SGGLDKL_HELPER_SHORT_VERSION_H_
#define SGGLDKL_HELPER_SHORT_VERSION_H_

#include <windows.h>

struct ShortVersion {
  DWORD major_left;
  DWORD major_right;
  DWORD minor_left;
  DWORD minor_right;
};

int ShortVersion_CompareAll(
    const struct ShortVersion* version1,
    const struct ShortVersion* version2
) {
  struct ShortVersion diff = { 0 };

  diff.major_left = version1->major_left - version2->major_left;
  if (diff.major_left != 0) {
    return diff.major_left;
  }

  diff.major_right = version1->major_right - version2->major_right;
  if (diff.major_right != 0) {
    return diff.major_right;
  }

  diff.minor_left = version1->minor_left - version2->minor_left;
  if (diff.minor_left != 0) {
    return diff.minor_left;
  }

  diff.minor_right = version1->minor_right - version2->minor_right;
  if (diff.minor_right != 0) {
    return diff.minor_right;
  }

  return 0;
}

#endif /* SGGLDKL_HELPER_SHORT_VERSION_H_ */
