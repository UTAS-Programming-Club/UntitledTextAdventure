#ifndef PCGAME_EQUIPMENT_H
#define PCGAME_EQUIPMENT_H

#include <stdint.h>
#include <types.h>

#include "game.h"

// Do not use outside of backend
// items equipable by player
// Never modify after creation
struct EquipmentInfo {
  // TODO: Remove?
  EquipmentID id;
  char *name;
  // TODO: Add enum for type (helmet, boots, etc). Is this actually needed?

  // stats
  PlayerStatDiff physAtkMod;
  PlayerStatDiff physDefMod;
  PlayerStatDiff magAtkMod;
  PlayerStatDiff magDefMod;

  // TODO: Add other stats such as dex, int, etc
};

// Only call for health or stamina, other uses are reserved for UpdateStats
bool UpdatePlayerStat(PlayerStat *, PlayerStatDiff);

bool UpdateStats(const struct GameInfo *, struct GameState *);

// void UnlockItem(state GameState *);

// Returns InvalidEquipmentID or a valid EquipmentID
EquipmentID GetEquippedItemID(const struct GameState *, uint_fast8_t);
// Returns NULL or a valid EquipmentInfo
struct EquipmentInfo *GetEquippedItem(const struct GameInfo *, const struct GameState *, uint_fast8_t);
bool SetEquippedItem(struct GameState *, uint_fast8_t, EquipmentID);

#endif // PCGAME_EQUIPMENT_H
