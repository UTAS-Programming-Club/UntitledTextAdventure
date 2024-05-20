#ifndef PCGAME_GAME_H
#define PCGAME_GAME_H

#ifndef ARENA_IMPLEMENTATION
#include <arena.h>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <uchar.h>

#include "types.h"

struct GameInput {
// public, safe to use outside of backend
  char32_t *title;
  bool visible;
// implementation, do not use outside of backend
  bool titleArena;
  enum InputOutcome outcome;
};

struct RoomInfo {
  enum RoomType type;
  RoomID roomID;
  RoomID northRoomID;
  RoomID eastRoomID;
  RoomID southRoomID;
  RoomID westRoomID;
};

struct GameOutput {
// public, safe to use outside of backend
  enum Screen screenID;
  char32_t *body;
  uint8_t inputCount;
  struct GameInput *inputs;
  struct RoomInfo roomInfo;
// implementation, do not use outside of backend
  Arena arena;
  bool bodyArena;
  bool inputsArrayArena;
  unsigned char *stateData;
  enum CustomScreenCode customScreenCodeID;
// Currently body and inputs[i].title MUST be allocated, this must be fixed if the encoding changes to utf-8 because then most button titles will also be direct copies of cJSON returned data
};

bool SetupGame(void);
bool GetCurrentGameOutput(struct GameOutput *);
enum InputOutcome HandleGameInput(struct GameOutput *, uint8_t);
void CleanupGame(struct GameOutput *);

#endif // PCGAME_GAME_H
