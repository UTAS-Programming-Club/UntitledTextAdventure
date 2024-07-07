#include <arena.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "game.h"
#include "save.h"

#define BASE 85
#define STARTING_CHAR '!'
#define STARTING_CHAR_STR "!"

// 65535 is reserved
static const uint16_t PasswordVersion = 0;

// TODO: Store screen variable data
struct __attribute__((packed, scalar_storage_order("little-endian"))) SaveData {
  uint16_t version;
  RoomCoordSave x;
  RoomCoordSave y;
};


// Assumes ascii
static inline char GetChar(uint_fast8_t val) {
  if (BASE < val) {
    return ' ';
  }

  return STARTING_CHAR + val;
}

// Assumes ascii
static inline uint_fast8_t GetVal(char c) {
  if (STARTING_CHAR > c || BASE + STARTING_CHAR <= c) {
    return UINT_FAST8_MAX;
  }

  return c - STARTING_CHAR;
}


// Encode state to ascii string using base 85 with 4 bytes to 5 chars
char *SaveState(struct GameState *state) {
  if (!state || !state->startedGame || !state->roomInfo) {
    return NULL;
  }

  struct SaveData data = {
    .version = PasswordVersion,
    .x = state->roomInfo->x,
    .y = state->roomInfo->y
  };

  uint8_t *pData = (uint8_t *)&data;
  size_t dataSize = sizeof data;
  // Note that this overestimates if dataSize is not a multiple of 4
  // This is corrected by reversing the order of each group of chars
  // so that the password ends with ! which can then be stripped out
  // to get a password that is shorter by 0 to 4 chars.
  size_t passwordSize = 5 * ceil(dataSize / 4.) + 1;
  char *password = arena_alloc(&state->arena, passwordSize);
  if (!password) {
    return NULL;
  }

  char *passwordPos = password;

  uint_fast32_t powers[] = {
    pow(BASE, 0),
    pow(BASE, 1),
    pow(BASE, 2),
    pow(BASE, 3),
    pow(BASE, 4)
  };

  for (size_t i = 0; i < dataSize; i += 4) {
    uint_fast32_t value = 0;
    size_t rem = dataSize - i % dataSize;
    size_t quot = rem < 4 ? rem : 4;
    memcpy(&value, pData + i, quot);

    uint_fast8_t digit4 = value / powers[4];
    uint_fast32_t value4 = value - digit4 * powers[4];
    if (digit4 >= BASE) {
      return NULL;
    }

    uint_fast8_t digit3 = value4 / powers[3];
    uint_fast32_t value3 = value4 - digit3 * powers[3];
    if (digit3 >= BASE) {
      return NULL;
    }

    uint_fast8_t digit2 = value3 / powers[2];
    uint_fast32_t value2 = value3 - digit2 * powers[2];
    if (digit2 >= BASE) {
      return NULL;
    }

    uint_fast8_t digit1 = value2 / powers[1];
    uint_fast32_t value1 = value2 - digit1 * powers[1];
    if (digit1 >= BASE) {
      return NULL;
    }

    uint_fast8_t digit0 = value1 / powers[0];
    uint_fast32_t value0 = value1 - digit0 * powers[0];
    if (0 != value0) {
      return NULL;
    }

    *(passwordPos++) = GetChar(digit0);
    *(passwordPos++) = GetChar(digit1);
    *(passwordPos++) = GetChar(digit2);
    *(passwordPos++) = GetChar(digit3);
    *(passwordPos++) = GetChar(digit4);
  }

  while(passwordPos[-1] == STARTING_CHAR) {
    --passwordPos;
  }
  *passwordPos = '\0';

  return password;
}

bool LoadState(const struct GameInfo *info, struct GameState *state, const char *password) {
  if (!state || state->startedGame || !password) {
    return false;
  }

  struct SaveData *data;
  size_t passwordSize = strlen(password);
  size_t dataSize = 4 * passwordSize / 5;
  uint32_t *decodedData = arena_alloc(&state->arena, dataSize);
  if (!decodedData) {
    return false;
  }

  uint_fast32_t powers[] = {
    pow(BASE, 0),
    pow(BASE, 1),
    pow(BASE, 2),
    pow(BASE, 3),
    pow(BASE, 4)
  };

  for (size_t i = 0; i < passwordSize; i += 5) {
    char data[5] = STARTING_CHAR_STR STARTING_CHAR_STR STARTING_CHAR_STR STARTING_CHAR_STR STARTING_CHAR_STR;
    uint8_t *pData = (uint8_t *)&data;
    size_t rem = passwordSize - i % passwordSize;
    size_t quot = rem < 5 ? rem : 5;
    memcpy(pData, password + i, quot);

    uint_fast8_t digit0 = GetVal(pData[0]);
    uint_fast8_t digit1 = GetVal(pData[1]);
    uint_fast8_t digit2 = GetVal(pData[2]);
    uint_fast8_t digit3 = GetVal(pData[3]);
    uint_fast8_t digit4 = GetVal(pData[4]);
    if (UINT_FAST8_MAX == digit0 || UINT_FAST8_MAX == digit1 || UINT_FAST8_MAX == digit2
        || UINT_FAST8_MAX == digit3 || UINT_FAST8_MAX == digit4) {
      return false;
    }

    decodedData[i / 5] = digit4 * powers[4] + digit3 * powers[3] + digit2 * powers[2] + digit1 * powers[1] + digit0 * powers[0];
  }

  // These should be the same but the current system can overestimate the number of bytes required
  if (sizeof data < dataSize) {
    return false;
  }
  data = (struct SaveData *)decodedData;

  if (PasswordVersion != data->version) {
    return false;
  }

  // TODO: Setup stats
  // TODO: Setup equipment

  state->roomInfo = GetGameRoom(info, data->x, data->y);
  state->startedGame = true;

  return true;
}
