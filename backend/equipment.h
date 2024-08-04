#ifndef PCGAME_EQUIPMENT_H
#define PCGAME_EQUIPMENT_H

#include <stdint.h>
#include <types.h>

#include "game.h"

// Do not use outside of backend
// items equipable by player
// Never modify after creation
struct EquipmentInfo {
  char *name;
  // TODO: Add enum for type (helmet, boots, etc). Is this actually needed?

  // stats
  EntityStatDiff physAtkMod;
  EntityStatDiff physDefMod;
  EntityStatDiff magAtkMod;
  EntityStatDiff magDefMod;

  // TODO: Add other stats such as dex, int, etc
};

bool ApplyPlayerDamage(struct PlayerInfo *, EntityStatDiff);
// Only call for health (if diff ignores others stats) and stamina, other uses of UpdatePlayerStat are reserved for UpdateStats
bool ModifyPlayerStat(EntityStat *, EntityStatDiff);
bool RefreshStats(const struct GameInfo *, struct GameState *);

bool UnlockItem(struct PlayerInfo *, EquipmentID);
bool CheckItemUnlocked(const struct PlayerInfo *, EquipmentID, bool *);

// Returns InvalidEquipmentID or a valid EquipmentID
EquipmentID GetEquippedItemID(const struct PlayerInfo *, EquipmentType);
// Returns NULL or a valid EquipmentInfo
struct EquipmentInfo *GetEquippedItem(const struct GameInfo *, const struct PlayerInfo *, EquipmentType);
bool SetEquippedItem(struct PlayerInfo *, EquipmentType, EquipmentID);

#endif // PCGAME_EQUIPMENT_H
