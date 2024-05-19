#ifndef PCGAME_PARSER_H
#define PCGAME_PARSER_H

#include <stdbool.h>
#include <stdint.h>

#include "../backend/game.h"

struct GameScreen {
  char32_t *body;
  char32_t *extraText;
  enum CustomScreenCode customScreenCodeID;
};

struct GameScreenButton {
  char32_t *title;
  enum InputOutcome outcome;
  enum Screen newScreenID; // Only set if outcome == GotoScreenOutcome
};

bool LoadGameData(char *);
void UnloadGameData(void);

unsigned char *InitGameState(void);
size_t GetGameStateOffset(enum Screen, uint8_t);

uint16_t GetGameScreenCount(void);
bool GetGameScreen(enum Screen, struct GameScreen *);

bool GetGameScreenButton(enum Screen, uint8_t, struct GameScreenButton *);
uint8_t GetGameScreenButtonCount(enum Screen);
void FreeGameScreenButton(struct GameScreenButton *);

#endif // PCGAME_PARSER_H
