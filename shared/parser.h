#ifndef PCGAME_PARSER_H
#define PCGAME_PARSER_H

#include <stdbool.h>
#include <stdint.h>

#include "../backend/game.h"

struct GameScreen {
  char32_t *body;
  char32_t *extraText;
  enum CustomScreenID customScreenID;
};

struct GameScreenButton {
  char32_t *title;
  enum InputOutcome outcome;
  enum ScreenID newScreen; // Only used(and set correctly) if outcome == GotoScreen
};

bool LoadGameData(char *);
void UnloadGameData(void);

unsigned char *InitGameState(void);
size_t GetGameStateOffset(enum ScreenID, uint8_t);

uint16_t GetGameScreenCount(void);
bool GetGameScreen(enum ScreenID, struct GameScreen *);

bool GetGameScreenButton(enum ScreenID, uint8_t, struct GameScreenButton *);
uint8_t GetGameScreenButtonCount(enum ScreenID);
void FreeGameScreenButton(struct GameScreenButton *);

#endif // PCGAME_PARSER_H
