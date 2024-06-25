#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "game.h"
#include "save.h"

static const char AllowedPasswordChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

// 65535 is reserved
static const uint16_t PasswordVersion = 0;

// TODO: Store screen variable data
__attribute__((__packed__)) struct SaveData {
  uint16_t version;
  RoomCoordSave x;
  RoomCoordSave y;
};

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
  size_t passwordSize = ceil(8. / 6 * dataSize) + 1;
  char *password = arena_alloc(&state->arena, passwordSize);

  // log_2(64) = 6 =/= 8 so using 6 * 4 = 8 * 3 = 24
  size_t i = 0;
  size_t j = 0;
  if (dataSize - i >= 3) {
    for (; dataSize - i >= 3; i += 3, j += 4) {
      // aaaaaaaa bbbbbbbb cccccccc -> aaaaaa aabbbb bbbbcc cccccc
      uint8_t hex1 = pData[i] & 0x3F /* 0b00111111 */;
      uint8_t hex2 = (pData[i + 1] & 0xF /* 0b00001111 */) << 2 | pData[i] >> 6;
      uint8_t hex3 = (pData[i + 2] & 0x3 /* 0b00000011 */) << 4 | pData[i + 1] >> 4;
      uint8_t hex4 = pData[i + 2] >> 2;

      password[j] = AllowedPasswordChars[hex1];
      password[j + 1] = AllowedPasswordChars[hex2];
      password[j + 2] = AllowedPasswordChars[hex3];
      password[j + 3] = AllowedPasswordChars[hex4];
    }
  }
  if (dataSize - i == 2) {
    // aaaaaaaa bbbbbbbb -> aaaaaa aabbbb bbbb00
    uint8_t hex1 = pData[i] & 0x3F /* 0b00111111 */;
    uint8_t hex2 = (pData[i + 1] & 0xF /* 0b00001111 */) << 2 | pData[i] >> 6;
    uint8_t hex3 = pData[i + 1] >> 4;

    password[j] = AllowedPasswordChars[hex1];
    password[j + 1] = AllowedPasswordChars[hex2];
    password[j + 2] = AllowedPasswordChars[hex3];

    j += 3;
  } else if (dataSize - i == 1) {
    // aaaaaaaa -> aaaaaa aa0000
    uint8_t hex1 = pData[i] & 0x3F /* 0b00111111 */;
    uint8_t hex2 = pData[i] >> 6;

    password[j] = AllowedPasswordChars[hex1];
    password[j + 1] = AllowedPasswordChars[hex2];

    j += 2;
  } else {
    return NULL;
  }

  password[j] = '\0';
  return password;
}

bool LoadState(const struct GameInfo *info, struct GameState *state, const char *password) {
  if (!state || state->startedGame || !password) {
    return false;
  }

  struct SaveData data;
  uint8_t *pData = (uint8_t *)&data;
  size_t dataSize = sizeof data;

  size_t requiredPasswordSize = ceil(8. / 6 * dataSize);
  size_t actualPasswordSize = strlen(password);
  if (requiredPasswordSize != actualPasswordSize) {
    return false;
  }

  // log_2(64) = 6 =/= 8 so using 6 * 4 = 8 * 3 = 24
  size_t i = 0;
  size_t j = 0;
  if (actualPasswordSize - i >= 4) {
    for (; actualPasswordSize - i >= 4; i += 4, j += 3) {
      // aaaaaa aabbbb bbbbcc cccccc -> aaaaaaaa bbbbbbbb cccccccc
      uint8_t hex1 = strchr(AllowedPasswordChars, password[i]) - AllowedPasswordChars;
      uint8_t hex2 = strchr(AllowedPasswordChars, password[i + 1]) - AllowedPasswordChars;
      uint8_t hex3 = strchr(AllowedPasswordChars, password[i + 2]) - AllowedPasswordChars;
      uint8_t hex4 = strchr(AllowedPasswordChars, password[i + 3]) - AllowedPasswordChars;

      pData[j] = hex1 | (hex2 & 0x3 /* 0b000011 */) << 6;
      pData[j + 1] = (hex2 & 0x3C /* 0b111100 */) >> 2 | (hex3 & 0xF /* 0b001111 */) << 4;
      pData[j + 2] = (hex3 & 0x30 /* 0b110000 */) >> 4 | hex4 << 2;
    }
  }
  if (actualPasswordSize - i == 3) {
    // aaaaaa aabbbb bbbb00 -> aaaaaaaa bbbbbbbb
    uint8_t hex1 = strchr(AllowedPasswordChars, password[i]) - AllowedPasswordChars;
    uint8_t hex2 = strchr(AllowedPasswordChars, password[i + 1]) - AllowedPasswordChars;
    uint8_t hex3 = strchr(AllowedPasswordChars, password[i + 2]) - AllowedPasswordChars;

    pData[j] = hex1 | (hex2 & 0x3 /* 0b000011 */) << 6;
    pData[j + 1] = (hex2 & 0x3C /* 0b111100 */) >> 2 | hex3 << 4;

    i += 3;
    j += 2;
  } else if (actualPasswordSize - i == 2) {
    uint8_t hex1 = strchr(AllowedPasswordChars, password[i]) - AllowedPasswordChars;
    uint8_t hex2 = strchr(AllowedPasswordChars, password[i + 1]) - AllowedPasswordChars;

    pData[j] = hex1 | hex2 << 6;

    i += 2;
    ++j;
  } else {
    return false;
  }

  pData[j] = '\0';
  ++j;

  if (PasswordVersion != data.version) {
    return false;
  }

  state->roomInfo = GetGameRoom(info, data.x, data.y);
  state->startedGame = true;

  return true;
}
