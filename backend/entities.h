#ifndef PCGAME_ENTITIES_H
#define PCGAME_ENTITIES_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <types.h>

// Game.h predefs
struct GameInfo;
struct GameState;

// Do not use outside of backend
struct PlayerInfo {
  // TODO: Add other stats that can be impacted by equipment
  EntityStat health;
  EntityStat stamina;
  EntityStat agility;
  EntityStat physAtk;
  EntityStat magAtk;
  EntityStat physDef;
  EntityStat magDef;

  // Do not access directly, use functions in equipment.h
  // Equipment types: helmets, chest pieces, gloves, pants, boots, primary weapon, secondary weapon
  bool unlockedItems[EquipmentCount];
  EquipmentID equippedItems[EquipmentTypeCount];
};

// Do not use outside of backend
struct EnemyInfo {
  // stats here
  // bool dead;
  EntityStat health;
  // 0 physical/1 magic
  // bool dmgType;
  // TODO: Use weapon?
  EntityStatDiff damage;
};

bool ApplyPlayerDamage(struct PlayerInfo *, EntityStatDiff);
// Only call for health (if diff ignores others stats) and stamina, other uses of UpdatePlayerStat are reserved for UpdateStats
bool ModifyPlayerStat(EntityStat *, EntityStatDiff);
bool RefreshStats(const struct GameInfo *, struct GameState *);

void EnemyPerformAttack(struct GameState *, const struct EnemyInfo *);
const char *CreateCombatString(struct GameState *, const struct EnemyInfo *);

// TODO: Individual attacks enemies can use with status' and stuff idk
// would have attack list with things like descriptor (blasted, stabbed etc.)

#endif // PCGAME_ENTITIES_H
