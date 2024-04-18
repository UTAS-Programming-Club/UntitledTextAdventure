#ifndef PCGAME_GAME_H
#define PCGAME_GAME_H

#include <stdbool.h>
#include <stdint.h>
#include <uchar.h>

struct GameInput {
  char32_t *title;
};

struct GameOutput {
  uint32_t screenID;
  char32_t *body;
  uint8_t inputCount;
  struct GameInput *inputs;
};

enum GameInputOutcome {
  InvalidInput       = 0, // Do not use in json or use in screens.c
  GetNextOutput      = 1, // Do not use in json or use in screens.c
  QuitGame           = 2,
  GotoTestScreen     = 3, // -> GetNextOutput
  GotoMainMenuScreen = 4, // -> GetNextOutput
};

bool SetupGame(void);
bool GetCurrentGameOutput(struct GameOutput *);
enum GameInputOutcome HandleGameInput(uint32_t stateID, uint32_t inputID);
void CleanupGame(void);

#endif // PCGAME_GAME_H
