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
// items equipable by player
// Never modify after creation
struct EquipmentInfo {
  EquipmentID id;
  char *name;
  // TODO: Add enum for type (helmet, boots, etc)

  // stats
  PlayerStatDiff physAtkMod;
  PlayerStatDiff physDefMod;
  PlayerStatDiff magAtkMod;
  PlayerStatDiff magDefMod;

  // TODO: Add other stats such as dex, int, etc
};

#define EquippedItemsSlots 7

// Do not use outside of backend
struct PlayerInfo {
  // TODO: Add custom types in types.in.h
  // helmet, shirts, gloves, pants, boots, 2x weapons
  const struct EquipmentInfo *equippedItems[EquippedItemsSlots];
  PlayerStat health;
  PlayerStat stamina;
  PlayerStat physAtk;
  PlayerStat magAtk;
  PlayerStat physDef;
  PlayerStat magDef;
  // TODO: turn into 2D array
  const struct EquipmentInfo *helmetSlot[EquipmentSlotLength];

  // TODO: Add other stats such as agility that can be impacted by equipment
};

// Always make this const when possible to avoid accidental modification
struct GameInfo {
// public, safe to use outside of backend
  const char *name; // utf-8
// implementation, do not use outside of backend
  bool initialised;

  // Only health, stamina, physAtk, magAtk, physDef, and magDef as available
  struct PlayerInfo defaultPlayerStats;

  uint_fast8_t floorSize;
  struct RoomInfo *rooms;

  uint_fast8_t equipmentCount;
  struct EquipmentInfo *equipment;
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
  PlayerStatDiff eventStatChange;
};

// Always make this const when possible to avoid accidental modification
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

// TODO: Move to another file that just handles equipment
void UpdateStats(struct GameState *);

// GameInfo should be zero initialised before first call
bool SetupBackend(struct GameInfo *);
// GameState should be zero initialised before first call
bool UpdateGameState(const struct GameInfo *, struct GameState *);
enum InputOutcome HandleGameInput(const struct GameInfo *, struct GameState *, uint_fast8_t, const char *);
const struct RoomInfo *GetGameRoom(const struct GameInfo *, RoomCoord, RoomCoord);
void CleanupGame(struct GameState *);
void CleanupBackend(struct GameInfo *);

#endif // PCGAME_GAME_H
