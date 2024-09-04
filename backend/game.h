#ifndef PCGAME_GAME_H
#define PCGAME_GAME_H

#ifndef ARENA_IMPLEMENTATION
#include <arena.h>
#endif

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <types.h>

#include "combat.h"
#include "entities.h"

// Always make this const when possible to avoid accidental modification
struct GameInfo {
// public, safe to use outside of backend
  const char *name; // utf-8
// implementation, do not use outside of backend
  bool initialised;

  struct PlayerInfo defaultPlayerInfo;

  // TODO: Change to size_t?
  uint_fast8_t floorSize;
  const struct RoomInfo *rooms;

  // TODO: Require struct to be on heap and then make this an actual array?
  const struct EquipmentInfo *equipment; // Length is EquipmentCount

  size_t enemyAttackCount;
  const struct EnemyAttackInfo *enemyAttacks;
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

  // Only set if type == CombatRoomType
  // TODO: Add actual enemy info with defence and other fields
  size_t enemyCount;
  size_t *enemies; // IDs into GameInfo.enemyAttacks
};

// Always make this const when possible to avoid accidental modification
// TODO: Require struct to be on heap?
struct GameState {
// public, safe to use outside of backend
  enum Screen screenID;
  const char *body; // utf-8

  enum ScreenInputType inputType;

  // Only set if inputType == ButtonScreenInputType
  uint_fast8_t inputCount;
  // TODO: Make const
  struct GameInput *inputs;

  // implementation, do not use outside of backend
  // Only set if inputType == TextScreenInputType
  enum Screen previousScreenID;
  enum Screen nextScreenID;

  // TODO: Rename to player and combat?
  struct PlayerInfo playerInfo;
  struct CombatInfo combatInfo;

  size_t previousRoomID;
  size_t roomID;

  bool startedGame;
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
// Returns SIZE_MAX if the room does not exist
size_t GetGameRoomID(const struct GameInfo *restrict, RoomCoord, RoomCoord);
// Room may not exist, always check result->type != InvalidRoomType
const struct RoomInfo *GetCurrentGameRoom(const struct GameInfo *restrict, const struct GameState *restrict);
void CleanupGame(struct GameState *);
void CleanupBackend(struct GameInfo *);

#endif // PCGAME_GAME_H
