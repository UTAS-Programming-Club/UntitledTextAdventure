#ifndef PCGAME_ENTITIES_H
#define PCGAME_ENTITIES_H

#include <stdbool.h> // bool
#include <stddef.h>  // size_t
#include <types.h>   // defines: EquipmentCount, EquipmentTypeCount
                     // types: EntityStat, EntityStatDiff, EquipmentID

// Game.h predefs
struct GameInfo;
struct GameState;

// Do not use outside of backend
struct PlayerInfo {
  // TODO: Add other stats that can be impacted by equipment
  EntityStat health;
  EntityStat stamina;
  // TODO: Decide how agility is increased. Perhaps inverse to armour weight and/or levels
  EntityStat agility;
  EntityStatDiff priPhysAtk;
  EntityStatDiff priMagAtk;
  EntityStatDiff secPhysAtk;
  EntityStatDiff secMagAtk;
  EntityStat physDef;
  EntityStat magDef;

  // Do not access directly, use functions in equipment.h
  // Equipment types: helmets, chest pieces, gloves, pants, boots, primary weapon, secondary weapon
  bool unlockedItems[EquipmentCount];
  EquipmentID equippedItems[EquipmentTypeCount];
};

// Do not use outside of backend
struct EnemyInfo {
  EntityStat health;
  size_t attackID;
};

// Only call for health (if diff ignores others stats) and stamina, other uses of UpdatePlayerStat are reserved for UpdateStats
bool ModifyEntityStat(EntityStat *restrict, EntityStatDiff);
bool RefreshPlayerStats(const struct GameInfo *, struct GameState *);

#endif // PCGAME_ENTITIES_H
