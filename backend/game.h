#ifndef PCGAME_GAME_H
#define PCGAME_GAME_H

#ifndef ARENA_IMPLEMENTATION
#include <arena.h>
#endif

#include <stdbool.h>
#include <stdint.h>

#include "types.h"

struct GameInput {
// public, safe to use outside of backend
  char *title; // utf-8
  bool visible;
// implementation, do not use outside of backend
  enum InputOutcome outcome;
};

// Do not use outside of backend
struct RoomInfo {
  // Always set
  bool exists;
  // Set only if the room exists
  enum RoomType type;
  RoomCoord x;
  RoomCoord y;
};
extern uint_fast8_t FloorSize;

struct GameOutput {
// public, safe to use outside of backend
  enum Screen screenID;
  char *body; // utf-8
  uint_fast8_t inputCount;
  struct GameInput *inputs;
  const struct RoomInfo *roomInfo;
  bool startedGame;
// implementation, do not use outside of backend
  Arena arena;
  unsigned char *stateData;
  enum CustomScreenCode customScreenCodeID;
// Currently body and inputs[i].title MUST be allocated, this must be fixed if the encoding changes to utf-8 because then most button titles will also be direct copies of cJSON returned data
};

bool SetupBackend(void);
bool GetCurrentGameOutput(struct GameOutput *);
enum InputOutcome HandleGameInput(struct GameOutput *, uint_fast8_t);
const struct RoomInfo *GetGameRoom(RoomCoord, RoomCoord);
void CleanupGame(struct GameOutput *);
void CleanupBackend(void);

#endif // PCGAME_GAME_H
