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

#include "file_signature.h"

#include <stddef.h>
#include <string.h>

int FileSignature_CompareAll(
    const struct FileSignature* signature1,
    const struct FileSignature* signature2
) {
  int file_path_diff;
  int offset_diff;
  int signature_diff;

  file_path_diff = wcscmp(
      signature1->file_path,
      signature2->file_path
  );

  if (file_path_diff != 0) {
    return file_path_diff;
  }

  offset_diff = signature1->offset - signature2->offset;

  if (offset_diff != 0) {
    return offset_diff;
  }

  signature_diff = memcmp(
      signature1->signature,
      signature2->signature,
      sizeof(signature1->signature)
  );

  return signature_diff;
}

int GameVersionSignature_CompareSignature(
    const struct GameVersionSignature* entry1,
    const struct GameVersionSignature* entry2
) {
  return FileSignature_CompareAll(
      &entry1->file_signature,
      &entry2->file_signature
  );
}

int GuessCorrectionSignature_CompareGuess(
    const struct GuessCorrectionSignature* entry1,
    const struct GuessCorrectionSignature* entry2
) {
  return entry1->guessed_version - entry2->guessed_version;
}
