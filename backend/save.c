#include <arena.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "game.h"
#include "save.h"

#define base 85

// 65535 is reserved
static const uint16_t PasswordVersion = 0;

// TODO: Store screen variable data
struct __attribute__((packed, scalar_storage_order("little-endian"))) SaveData {
  uint16_t version;
  RoomCoordSave x;
  RoomCoordSave y;
};


static inline char GetChar(uint_fast8_t val) {
  if (base < val) {
    return ' ';
  }

  return '!' + val;
}

static inline uint_fast8_t GetVal(char c) {
  if ('!' > c || base + '!' <= c) {
    return UINT_FAST8_MAX;
  }

  return c - '!';
}


char *SaveState(struct GameState *state) {
  if (!state || !state->startedGame || !state->roomInfo) {
    return NULL;
  }

  struct SaveData data = {
    .version = PasswordVersion,
    .x = state->roomInfo->x,
    .y = state->roomInfo->y
  };

  unsigned char *pData = (unsigned char *)&data;
  size_t dataSize = sizeof data;
  // TODO: This overestimates if dataSize is not a multiple of 4
  size_t passwordSize = 5 * ceil(dataSize / 4.) + 1;
  char *password = arena_alloc(&state->arena, passwordSize);
  if (!password) {
    return NULL;
  }

  char *passwordPos = password;

  uint_fast32_t powers[] = {
    pow(base, 0),
    pow(base, 1),
    pow(base, 2),
    pow(base, 3),
    pow(base, 4)
  };

  for (size_t i = 0; i < dataSize; i += 4) {
    uint_fast32_t value = 0;
    size_t rem = dataSize - i % dataSize;
    size_t quot = rem < 4 ? rem : 4;
    memcpy(&value, pData + i, quot);

    uint_fast8_t digit4 = value / powers[4];
    uint_fast32_t value4 = value - digit4 * powers[4];
    if (digit4 >= base) {
      return NULL;
    }

    uint_fast8_t digit3 = value4 / powers[3];
    uint_fast32_t value3 = value4 - digit3 * powers[3];
    if (digit3 >= base) {
      return NULL;
    }

    uint_fast8_t digit2 = value3 / powers[2];
    uint_fast32_t value2 = value3 - digit2 * powers[2];
    if (digit2 >= base) {
      return NULL;
    }

    uint_fast8_t digit1 = value2 / powers[1];
    uint_fast32_t value1 = value2 - digit1 * powers[1];
    if (digit1 >= base) {
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
  *passwordPos = '\0';

  return password;
}

bool LoadState(const struct GameInfo *info, struct GameState *state, const char *password) {
  if (!state || state->startedGame || !password) {
    return false;
  }

  struct SaveData *data;
  size_t passwordSize = strlen(password);
  if (passwordSize % 5 != 0) {
    return false;
  }

  uint_fast32_t powers[] = {
    pow(base, 0),
    pow(base, 1),
    pow(base, 2),
    pow(base, 3),
    pow(base, 4)
  };

  size_t dataSize = 4 * passwordSize / 5;
  uint32_t *decodedData = arena_alloc(&state->arena, dataSize);
  if (!decodedData) {
    return false;
  }

  for (size_t i = 0; i < passwordSize; i += 5) {
    uint_fast8_t digit0 = GetVal(password[i]);
    uint_fast8_t digit1 = GetVal(password[i + 1]);
    uint_fast8_t digit2 = GetVal(password[i + 2]);
    uint_fast8_t digit3 = GetVal(password[i + 3]);
    uint_fast8_t digit4 = GetVal(password[i + 4]);
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
