#ifndef PCGAME_PARSER_H
#define PCGAME_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <types.h>

#include "game.h"

// TODO: Remove?
struct GameScreen {
  char *body;      // utf-8
  char *extraText; // utf-8
  enum CustomScreenCode customScreenCodeID;
  enum ScreenInputType inputType;
  enum Screen previousScreenID; // Only set if inputType == TextScreenInputType
  enum Screen nextScreenID;     // Only set if inputType == TextScreenInputType
};

// TODO: Remove?
struct GameScreenButton {
  char *title; // utf-8
  enum InputOutcome outcome;
  enum Screen newScreenID; // Only set if outcome == GotoScreenOutcome
  uint_fast8_t equipmentSlot;
};

bool LoadGameData(char *);
void UnloadGameData(void);

char *LoadGameName(void);
bool LoadDefaultPlayerInfo(struct PlayerInfo *);
// The RoomInfo array must be freed, even if this function returns false
bool LoadGameRooms(uint_fast8_t *, struct RoomInfo **);
bool LoadGameEquipment(struct EquipmentInfo **);

// This array must be freed by the end of the game
bool InitGameState(size_t *stateLen, unsigned char **state);
size_t GetGameStateOffset(enum Screen, uint_fast8_t);

// Returns UINT_FAST16_MAX on error
uint_fast16_t GetGameScreenCount(void);
bool GetGameScreen(enum Screen, struct GameScreen *);

bool GetGameScreenButton(enum Screen, uint_fast8_t, struct GameScreenButton *);
// Returns UINT_FAST8_MAX on error
uint_fast8_t GetGameScreenButtonCount(enum Screen);

#endif // PCGAME_PARSER_H
