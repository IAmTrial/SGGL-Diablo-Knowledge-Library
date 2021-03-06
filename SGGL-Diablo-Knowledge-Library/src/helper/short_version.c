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

#include "short_version.h"

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

int ShortVersion_CompareAsVoidAll(
    const void* version1,
    const void* version2
) {
  return ShortVersion_CompareAll(
      (const struct ShortVersion*) version1,
      (const struct ShortVersion*) version2
  );
}

int ShortVersionString_CompareAll(
    const struct ShortVersionString* version_str1,
    const struct ShortVersionString* version_str2
) {
  return wcscmp(version_str1->version_str, version_str2->version_str);
}

int ShortVersionString_CompareAsVoidAll(
    void* version_str1,
    void* version_str2
) {
  return ShortVersionString_CompareAll(
      (const struct ShortVersionString*) version_str1,
      (const struct ShortVersionString*) version_str2
  );
}

int ShortVersionAndGameVersionEntry_CompareKey(
    const struct ShortVersionAndGameVersionEntry* entry1,
    const struct ShortVersionAndGameVersionEntry* entry2
) {
  return ShortVersion_CompareAll(
      &entry1->short_version,
      &entry2->short_version
  );
}

int ShortVersionAndGameVersionEntry_CompareAsVoidKey(
    const void* entry1,
    const void* entry2
) {
  return ShortVersionAndGameVersionEntry_CompareKey(
      (const struct ShortVersionAndGameVersionEntry*) entry1,
      (const struct ShortVersionAndGameVersionEntry*) entry2
  );
}

int ShortVersionStringAndGameVersionEntry_CompareKey(
    const struct ShortVersionStringAndGameVersionEntry* entry1,
    const struct ShortVersionStringAndGameVersionEntry* entry2
) {
  return ShortVersionString_CompareAll(
      &entry1->short_version_str,
      &entry2->short_version_str
  );
}

int ShortVersionStringAndGameVersionEntry_CompareAsVoidKey(
    const void* entry1,
    const void* entry2
) {
  return ShortVersionStringAndGameVersionEntry_CompareKey(
      (const struct ShortVersionStringAndGameVersionEntry*) entry1,
      (const struct ShortVersionStringAndGameVersionEntry*) entry2
  );
}
