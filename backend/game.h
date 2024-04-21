#ifndef PCGAME_GAME_H
#define PCGAME_GAME_H

#include <stdbool.h>
#include <stdint.h>
#include <uchar.h>

struct GameInput {
  char32_t *title;
};

struct GameOutput {
// public, safe to use outside of backend
  uint32_t screenID;
  char32_t *body;
  uint8_t inputCount;
  struct GameInput *inputs;
// implementation, do not use outside of backend
// Currently body and inputs[i].title MUST be allocated, this must be fixed if the encoding changes to utf-8 because then most button titles will also be direct copies of cJSON returned data
};

#define INVALID_SCREEN_ID UINT32_MAX

enum GameInputOutcome {
  InvalidInput       = 0, // Do not use in json or use in screens.c
  GetNextOutput      = 1, // Do not use in json or use in screens.c
  QuitGame           = 2,
  GotoScreen         = 3, // -> GetNextOutput, Needs newScreen field from json
};

bool SetupGame(void);
bool GetCurrentGameOutput(struct GameOutput *);
enum GameInputOutcome HandleGameInput(uint32_t stateID, uint32_t inputID);
void CleanupGame(void);

#endif // PCGAME_GAME_H
