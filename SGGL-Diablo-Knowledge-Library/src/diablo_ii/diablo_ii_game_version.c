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

#include "diablo_ii_game_version.h"

#include <shlwapi.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "../helper/encoding.h"
#include "../helper/error_handling.h"
#include "../helper/file_info.h"
#include "../helper/file_path.h"
#include "../helper/file_signature.h"
#include "../helper/short_version.h"

/*
* The order of the entries should be in numerical order of significant
* versions, due to the reliance on bsearch.
*/

static const struct ShortVersionAndGameVersionEntry
kGameFileVersionsToGameVersion[] = {
    /* 1.0.0.1 */
    { { 1, 0, 0, 1 }, DIABLO_II_1_01 },

    /* 1.0.2.0 */
    { { 1, 0, 2, 0 }, DIABLO_II_1_02 },

    /* 1.0.3.0 */
    { { 1, 0, 3, 0 }, DIABLO_II_1_03 },

    /* 1.0.4.0 */
    { { 1, 0, 4, 0 }, DIABLO_II_1_04 },

    /* 1.0.4.1 */
    { { 1, 0, 4, 1 }, DIABLO_II_1_04B },

    /* 1.0.4.2 */
    { { 1, 0, 4, 2 }, DIABLO_II_1_04C },

    /* 1.0.5.0 */
    { { 1, 0, 5, 0 }, DIABLO_II_1_05 },

    /* 1.0.5.1 */
    { { 1, 0, 5, 1 }, DIABLO_II_1_05B },

    /* 1.0.6.0 */
    { { 1, 0, 6, 0 }, DIABLO_II_1_06B },

    /* 1.0.7.0 */
    { { 1, 0, 7, 0 }, DIABLO_II_1_07 },

    /* 1.0.8.28 */
    { { 1, 0, 8, 28 }, DIABLO_II_1_08 },

    /* 1.0.8.28 */
    { { 1, 0, 8, 28 }, DIABLO_II_1_08 },

    /* 1.0.9.19 */
    { { 1, 0, 9, 19 }, DIABLO_II_1_09 },

    /* 1.0.9.20 */
    { { 1, 0, 9, 20 }, DIABLO_II_1_09B },

    /* 1.0.9.21 */
    { { 1, 0, 9, 21 }, DIABLO_II_1_09C },

    /* 1.0.9.22 */
    { { 1, 0, 9, 22 }, DIABLO_II_1_09D },

    /* 1.0.10.9 */
    { { 1, 0, 10, 9 }, DIABLO_II_1_10_BETA },

    /* 1.0.10.10 */
    { { 1, 0, 10, 10 }, DIABLO_II_1_10S_BETA },

    /* 1.0.10.39 */
    { { 1, 0, 10, 39 }, DIABLO_II_1_10 },

    /* 1.0.11.45 */
    { { 1, 0, 11, 45 }, DIABLO_II_1_11 },

    /* 1.0.11.46 */
    { { 1, 0, 11, 46 }, DIABLO_II_1_11B },

    /* 1.0.12.49 */
    { { 1, 0, 12, 49 }, DIABLO_II_1_12A },

    /* 1.0.13.55 */
    { { 1, 0, 13, 55 }, DIABLO_II_1_13A_PTR },

    /* 1.0.13.60 */
    { { 1, 0, 13, 60 }, DIABLO_II_1_13C },

    /* 1.0.13.64 */
    { { 1, 0, 13, 64 }, DIABLO_II_1_13D },

    /* 1.14.0.64 */
    { { 1, 14, 0, 64 }, DIABLO_II_1_14A },

    /* 1.14.1.68 */
    { { 1, 14, 1, 68 }, DIABLO_II_1_14B },

    /* 1.14.2.70 */
    { { 1, 14, 2, 70 }, DIABLO_II_1_14C },

    /* 1.14.3.71 */
    { { 1, 14, 3, 71 }, DIABLO_II_1_14D }
};

static const struct GuessCorrectionSignature kGuessCorrectionTable[] = {
    {
        DIABLO_II_1_06B,
        {
            {
                L"storm.dll",
                0xF0,
                { 0x43, 0x0C, 0xD6, 0x3A }
            },
            DIABLO_II_1_06
        }
    },
    {
        DIABLO_II_1_07,
        {
            {
                L"storm.dll",
                0xF8,
                { 0x32, 0xA6, 0xDC, 0x3A }
            },
            DIABLO_II_1_07_BETA
        }
    }
};

/* The elements need to be ordered by file path, offset, then signature. */
static const struct GameVersionSignature k1001GameVersionSignatureTable[] = {
    {
        {
            L"storm.dll",
            0xF0,
            { 0x25, 0x47, 0x52, 0x39 }
        },
        DIABLO_II_1_01
    },
    {
        {
            L"storm.dll",
            0xF0,
            { 0x79, 0xBD, 0x20, 0x39 }
        },
        DIABLO_II_1_02_STRESS_TEST_BETA
    },
    {
        {
            L"storm.dll",
            0xF0,
            { 0xB7, 0x70, 0xD0, 0x38 }
        },
        DIABLO_II_1_02_BETA
    },
    {
        {
            L"storm.dll",
            0xF0,
            { 0xBC, 0xC7, 0x2E, 0x39 }
        },
        DIABLO_II_1_00
    },
};

static enum GameVersion DetermineGameVersionByData(
    const wchar_t* game_file_path,
    size_t game_file_path_len,
    enum GameVersion guessed_game_version
) {
  const struct GuessCorrectionSignature search_key = {
      guessed_game_version
  };

  const struct GuessCorrectionSignature* search_result;
  const struct GameVersionSignature* game_version_signature;

  unsigned char check_buffer[4];
  size_t actual_num_check_bytes;

  const size_t kExpectedNumCheckBytes = sizeof(check_buffer);
  const size_t kExpectedNumCheckElems =
      kExpectedNumCheckBytes / sizeof(check_buffer[0]);

  wchar_t* adjacent_file_path;
  FILE* game_file_stream;

  int is_fseek_fail;
  int is_fclose_fail;

  int compare_result;

  /* Search the table for the data info entry. */
  search_result = (const struct GuessCorrectionSignature*) bsearch(
      &search_key,
      kGuessCorrectionTable,
      sizeof(kGuessCorrectionTable) / sizeof(kGuessCorrectionTable[0]),
      sizeof(kGuessCorrectionTable[0]),
      &GuessCorrectionSignature_CompareGuess
  );

  /* It's not found, so the initial guess was likely correct. */
  if (search_result == NULL) {
    return guessed_game_version;
  }

  game_version_signature = &search_result->game_version_signature;

  /* Open the file for reading. */
  adjacent_file_path = GetAdjacentFilePath(
      game_file_path,
      game_file_path_len,
      game_version_signature->file_signature.file_path,
      wcslen(game_version_signature->file_signature.file_path)
  );

  game_file_stream = _wfopen(adjacent_file_path, L"rb");

  if (game_file_stream == NULL) {
    ExitOnGeneralFailure(
        L"Could not open file for reading.",
        L"File Could Not Be Opened"
    );
  }

  /* Seek to the pos */
  is_fseek_fail = fseek(
      game_file_stream,
      game_version_signature->file_signature.offset,
      SEEK_SET
  );

  if (is_fseek_fail) {
    ExitOnGeneralFailure(
        L"Cannot seek to the file's target offset.",
        L"Game Version Check Failure"
    );
  }

  actual_num_check_bytes = fread(
      check_buffer,
      sizeof(check_buffer[0]),
      kExpectedNumCheckElems,
      game_file_stream
  );

  if (actual_num_check_bytes != kExpectedNumCheckElems) {
    ExitOnGeneralFailure(
        L"Number of check bytes does not match.",
        L"Game Version Check Failure"
    );
  }

  is_fclose_fail = fclose(game_file_stream);

  if (is_fclose_fail) {
    ExitOnGeneralFailure(
        L"Failed to close the file stream.",
        L"File Stream Failure"
    );
  }

free_adjacent_file_path:
  free(adjacent_file_path);

  /* Check the bytes for each possible version. */
  compare_result = memcmp(
      check_buffer,
      game_version_signature->file_signature.signature,
      sizeof(game_version_signature->file_signature.signature)
  );

  return (compare_result == 0)
      ? game_version_signature->game_version
      : guessed_game_version;
}

static enum GameVersion Determine1001GameVersionByData(
    const wchar_t* game_file_path,
    size_t game_file_path_len
) {
  enum Constant {
    CHECK_POSITION = 0xF0
  };

  const wchar_t* kStormFileName = L"storm.dll";
  const size_t kStormFileNameLen =
      (sizeof(L"storm.dll") / sizeof(kStormFileName[0])) - 1;

  struct GameVersionSignature search_key;
  const struct GameVersionSignature* search_result;

  const size_t kExpectedNumCheckBytes =
      sizeof(search_key.file_signature.signature);
  const size_t kExpectedNumCheckElems = kExpectedNumCheckBytes
      / sizeof(search_key.file_signature.signature[0]);

  wchar_t* storm_file_path;
  FILE* game_file_stream;

  int is_fseek_fail;
  int is_fclose_fail;

  size_t actual_num_check_bytes;

  /* Open the file for reading. */
  storm_file_path = GetAdjacentFilePath(
      game_file_path,
      game_file_path_len,
      kStormFileName,
      kStormFileNameLen
  );

  game_file_stream = _wfopen(storm_file_path, L"rb");

  if (game_file_stream == NULL) {
    ExitOnGeneralFailure(
        L"Could not open Storm.dll for reading.",
        L"File Could Not Be Opened"
    );
  }

  /* Seek to the pos */
  is_fseek_fail = fseek(
      game_file_stream,
      CHECK_POSITION,
      SEEK_SET
  );

  if (is_fseek_fail) {
    ExitOnGeneralFailure(
        L"Cannot seek to the file's target offset.",
        L"Game Version Check Failure"
    );
  }

  /* Read the bytes that will be checked. */
  actual_num_check_bytes = fread(
      search_key.file_signature.signature,
      sizeof(search_key.file_signature.signature[0]),
      kExpectedNumCheckBytes,
      game_file_stream
  );

  if (actual_num_check_bytes != kExpectedNumCheckElems) {
    ExitOnGeneralFailure(
        L"Number of check bytes does not match.",
        L"Game Version Check Failure"
    );
  }

  /* Close the file, now that the bytes have been read. */
  is_fclose_fail = fclose(game_file_stream);

  if (is_fclose_fail) {
    ExitOnGeneralFailure(
        L"Failed to close the file stream.",
        L"File Stream Failure"
    );
  }

  /* Check the bytes for each possible version. */
  search_key.file_signature.file_path = kStormFileName;
  search_key.file_signature.offset = CHECK_POSITION;

  search_result = (const struct GameVersionSignature*) bsearch(
      &search_key,
      k1001GameVersionSignatureTable,
      sizeof(k1001GameVersionSignatureTable)
          / sizeof(k1001GameVersionSignatureTable[0]),
      sizeof(k1001GameVersionSignatureTable[0]),
      &GameVersionSignature_CompareSignature
  );

  if (search_result == NULL) {
    return VERSION_UNKNOWN;
  }

free_storm_file_path:
  free(storm_file_path);

  return search_result->game_version;
}

static enum GameVersion SearchGameFileInfoTable(
    const VS_FIXEDFILEINFO* game_file_info
) {
  const struct ShortVersionAndGameVersionEntry search_key = {
      {
          (game_file_info->dwFileVersionMS >> 16) & 0xFFFF,
          (game_file_info->dwFileVersionMS >> 0) & 0xFFFF,
          (game_file_info->dwFileVersionLS >> 16) & 0xFFFF,
          (game_file_info->dwFileVersionLS >> 0) & 0xFFFF
      }
  };

  const struct ShortVersionAndGameVersionEntry* search_result;

  search_result = (const struct ShortVersionAndGameVersionEntry*) bsearch(
      &search_key,
      kGameFileVersionsToGameVersion,
      sizeof(kGameFileVersionsToGameVersion)
          / sizeof(kGameFileVersionsToGameVersion[0]),
      sizeof(kGameFileVersionsToGameVersion[0]),
      &ShortVersionAndGameVersionEntry_CompareKey
  );

  if (search_result == NULL) {
    return VERSION_UNKNOWN;
  }

  return search_result->game_version;
}

enum GameVersion Diablo_II_FindGameVersion(
    const wchar_t* game_file_path,
    size_t game_file_path_len
) {
  VS_FIXEDFILEINFO game_file_info;
  enum GameVersion first_guess_game_version;

  /* Extract the file info from the game executable. */
  ExtractFileInfo(&game_file_info, game_file_path);

  /*
  * Perform a search of the game version in the table. This will not
  * cover all cases, as some versions share file versions.
  */
  first_guess_game_version = SearchGameFileInfoTable(&game_file_info);

  /*
  * File version 1.0.0.1 is shared across prerelease and release
  * versions, so special case is needed.
  */
  if (first_guess_game_version == DIABLO_II_1_01) {
    return Determine1001GameVersionByData(
        game_file_path,
        game_file_path_len
    );
  }

  return DetermineGameVersionByData(
      game_file_path,
      game_file_path_len,
      first_guess_game_version
  );
}
