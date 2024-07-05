#ifndef PCGAME_GAME_H
#define PCGAME_GAME_H

#ifndef ARENA_IMPLEMENTATION
#include <arena.h>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <types.h>

// Do not use outside of backend
// items equipable by player
struct EquipmentInfo {
  uint_fast8_t id;
  char *name;
  // TODO: Add enum for type (helmet, boots, etc)

  // stats
  uint_fast8_t physAtkMod;
  uint_fast8_t physDefMod;
  uint_fast8_t magAtkMod;
  uint_fast8_t magDefMod;

  // TODO: Add other stats such as dex, int, etc
};

#define EquippedIDLen 7

// Do not use outside of backend
struct PlayerInfo {
  // TODO: Add custom types in types.in.h
  // helmet, shirts, gloves, pants, boots, 2x weapons
  uint_fast8_t equippedIDs[EquippedIDLen];
  uint_fast8_t health;
  uint_fast8_t stamina;
  uint_fast8_t physAtk;
  uint_fast8_t magAtk;
  uint_fast8_t physDef;
  uint_fast8_t magDef;

  // TODO: Add other stats such as agility that can be impacted by equipment
};

// Always make this const when possible to avoid accidental modification
struct GameInfo {
// public, safe to use outside of backend
  char *name; // utf-8
// implementation, do not use outside of backend
  bool initialised;

  // Only health, stamina, physAtk, magAtk, physDef, and magDef as available
  struct PlayerInfo defaultPlayerStats;

  uint_fast8_t floorSize;
  struct RoomInfo *rooms;

  uint_fast8_t equipmentDBLength;
  struct EquipmentInfo *equipmentDB;
};

struct GameInput {
// public, safe to use outside of backend
  char *title; // utf-8
  bool visible;
// implementation, do not use outside of backend
  enum InputOutcome outcome;
};

// Do not use outside of backend
struct RoomInfo {
  // Always set
  bool exists;
  // Set only if the room exists
  enum RoomType type;
  RoomCoord x;
  RoomCoord y;
};

// Always make this const when possible to avoid accidental modification
struct GameState {
// public, safe to use outside of backend
  enum Screen screenID;
  char *body; // utf-8
  uint_fast8_t inputCount;
  struct GameInput *inputs;
  struct PlayerInfo playerInfo;
  const struct RoomInfo *roomInfo;
  bool startedGame;
// implementation, do not use outside of backend
  Arena arena;
  unsigned char *stateData;
  enum CustomScreenCode customScreenCodeID;
// Currently body and inputs[i].title MUST be allocated, this must be fixed if the encoding changes to utf-8 because then most button titles will also be direct copies of cJSON returned data
};

// TODO: Move to another file that just handles equipment
// find item matching ID and equip it to the player slot swapping out existing equipment
void EquipItem(const struct GameInfo *, struct GameState *);

// GameInfo should be zero initialised before first call
bool SetupBackend(struct GameInfo *);
// GameState should be zero initialised before first call
bool UpdateGameState(const struct GameInfo *, struct GameState *);
enum InputOutcome HandleGameInput(const struct GameInfo *, struct GameState *, uint_fast8_t);
const struct RoomInfo *GetGameRoom(const struct GameInfo *, RoomCoord, RoomCoord);
void CleanupGame(struct GameState *);
void CleanupBackend(struct GameInfo *);

#endif // PCGAME_GAME_H
