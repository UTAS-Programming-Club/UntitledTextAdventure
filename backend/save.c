#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

char *GetSavePassword(struct GameState *state) {
  if (!state || !state->startedGame) {
    return NULL;
  }

  struct SaveData data = {
    .version = PasswordVersion,
    .x = state->roomInfo->x,
    .y = state->roomInfo->y
  };

  uint8_t *pData = (uint8_t *)&data;
  size_t dataSize = sizeof data;
  char *password = arena_alloc(&state->arena, (dataSize + 1) * sizeof *password);

  for (size_t i = 0; i < dataSize; ++i) {
     password[i] = AllowedPasswordChars[pData[i]];
  }
  password[dataSize] = '\0';

  return password;
}
