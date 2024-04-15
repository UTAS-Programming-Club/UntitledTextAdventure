#ifndef PCGAME_GAME_H
#define PCGAME_GAME_H

#include <stdint.h>
#include <uchar.h>

struct GameInput {
  uint8_t inputID;
  char32_t *title;
};

struct GameOutput {
  uint32_t stateID;
  char32_t *body;
  uint8_t inputCount;
  struct GameInput *inputs;
};

enum GameInputOutcome {
  InvalidInput,
  GetNextOutput,
  QuitGame
};

void GetCurrentGameOutput(struct GameOutput *);
enum GameInputOutcome HandleGameInput(uint32_t stateID, uint32_t inputID);
void CleanupGame(void);

#endif // PCGAME_GAME_H
