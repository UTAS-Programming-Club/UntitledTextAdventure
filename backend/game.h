#ifndef PCGAME_GAME_H
#define PCGAME_GAME_H

#ifndef ARENA_IMPLEMENTATION
#include <arena.h>
#endif

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <types.h>

// Do not use outside of backend
struct PlayerInfo {
  // TODO: Add other stats such as agility that can be impacted by equipment
  EntityStat health;
  EntityStat stamina;
  EntityStat physAtk;
  EntityStat magAtk;
  EntityStat physDef;
  EntityStat magDef;

  // Do not access directly, use functions in equipment.h
  // Equipment types: helmets, chest pieces, gloves, pants, boots, primary weapon, secondary weapon
  bool unlockedItems[EquipmentCount];
  EquipmentID equippedItems[EquipmentTypeCount];
};

// Always make this const when possible to avoid accidental modification
struct GameInfo {
// public, safe to use outside of backend
  const char *name; // utf-8
// implementation, do not use outside of backend
  bool initialised;

  struct PlayerInfo defaultPlayerInfo;

  uint_fast8_t floorSize;
  struct RoomInfo *rooms;

  // TODO: Require struct to be on heap and then make this an actual array?
  struct EquipmentInfo *equipment; // Length is EquipmentCount
};

// Never modify after creation
struct GameInput {
// public, safe to use outside of backend
  const char *title; // utf-8
  bool visible;
// implementation, do not use outside of backend
  enum InputOutcome outcome;
};

// Do not use outside of backend
// Never modify after creation
struct RoomInfo {
  enum RoomType type;

  // Only set if type != InvalidRoomType
  RoomCoord x;
  RoomCoord y;

  // Only set if type == HealthChangeRoomType
  const char *eventDescription; // utf-8
  uint_fast8_t eventPercentageChance;
  EntityStatDiff eventStatChange;
};

// Always make this const when possible to avoid accidental modification
// TODO: Require struct to be on heap?
struct GameState {
// public, safe to use outside of backend
  enum Screen screenID;
  const char *body; // utf-8

  enum ScreenInputType inputType;
  uint_fast8_t inputCount;      // Only set if inputType == ButtonScreenInputType
  // TODO: Make const
  struct GameInput *inputs;     // Only set if inputType == ButtonScreenInputType
  enum Screen previousScreenID; // Only set if inputType == TextScreenInputType
  enum Screen nextScreenID;     // Only set if inputType == TextScreenInputType

  struct PlayerInfo playerInfo;
  const struct RoomInfo *roomInfo;
  bool startedGame;
// implementation, do not use outside of backend
  Arena arena;

  static_assert(sizeof(unsigned char) == sizeof(uint8_t), "Need 8 bit bytes.");
  size_t stateDataSize;
  unsigned char *stateData;

  enum CustomScreenCode customScreenCodeID;
};

// GameInfo should be zero initialised before first call
bool SetupBackend(struct GameInfo *);
// GameState should be zero initialised before first call
bool UpdateGameState(const struct GameInfo *, struct GameState *);
enum InputOutcome HandleGameInput(const struct GameInfo *, struct GameState *, uint_fast8_t, const char *);
const struct RoomInfo *GetGameRoom(const struct GameInfo *, RoomCoord, RoomCoord);
void CleanupGame(struct GameState *);
void CleanupBackend(struct GameInfo *);

// This header depends on structs in this header
#include "equipment.h"

#endif // PCGAME_GAME_H
