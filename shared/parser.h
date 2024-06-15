#ifndef PCGAME_PARSER_H
#define PCGAME_PARSER_H

#include <stdbool.h>
#include <stdint.h>

#include "../backend/game.h"

struct GameScreen {
  char *body;      // utf-8
  char *extraText; // utf-8
  enum CustomScreenCode customScreenCodeID;
};

struct GameScreenButton {
  char *title; // utf-8
  enum InputOutcome outcome;
  enum Screen newScreenID; // Only set if outcome == GotoScreenOutcome
};

bool LoadGameData(char *);
void UnloadGameData(void);

bool LoadGameRooms(uint_fast8_t *floorSize, struct RoomInfo **);

unsigned char *InitGameState(void);
size_t GetGameStateOffset(enum Screen, uint_fast8_t);

// Returns UINT_FAST16_MAX on error
uint_fast16_t GetGameScreenCount(void);
bool GetGameScreen(enum Screen, struct GameScreen *);

bool GetGameScreenButton(enum Screen, uint_fast8_t, struct GameScreenButton *);
// Returns UINT_FAST8_MAX on error
uint_fast8_t GetGameScreenButtonCount(enum Screen);

#endif // PCGAME_PARSER_H
