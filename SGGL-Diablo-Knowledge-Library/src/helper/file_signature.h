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

#ifndef SGGLDKL_HELPER_FILE_SIGNATURE_H_
#define SGGLDKL_HELPER_FILE_SIGNATURE_H_

#include <wchar.h>

#include "../game_version.h"

struct FileSignature {
  const wchar_t* file_path;
  int offset;
  unsigned char signature[4];
};

struct GameVersionSignature {
  struct FileSignature file_signature;
  enum GameVersion game_version;
};

struct GuessCorrectionSignature {
  enum GameVersion guessed_version;
  struct GameVersionSignature game_version_signature;
};

int FileSignature_CompareAll(
    const struct FileSignature* signature1,
    const struct FileSignature* signature2
);

int FileSignature_CompareAsVoidAll(
    const void* signature1,
    const void* signature2
);

int GameVersionSignature_CompareSignature(
    const struct GameVersionSignature* entry1,
    const struct GameVersionSignature* entry2
);

int GameVersionSignature_CompareAsVoidSignature(
    const void* entry1,
    const void* entry2
);

int GuessCorrectionSignature_CompareGuess(
    const struct GuessCorrectionSignature* entry1,
    const struct GuessCorrectionSignature* entry2
);

int GuessCorrectionSignature_CompareAsVoidGuess(
    const void* entry1,
    const void* entry2
);

#endif /* SGGLDKL_HELPER_FILE_SIGNATURE_H_ */
