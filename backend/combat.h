#ifndef PCGAME_COMBAT_H
#define PCGAME_COMBAT_H

#include <stdbool.h> // bool
#include <stddef.h>  // size_t
#include <types.h>   // enums: CombatEventCause, EnemyAttackType
                     // types: EntityStat, EntityStatDiff

#include "entities.h"

// Game.h predefs
struct GameInfo;
struct GameState;

// Do not use outside of backend
struct EnemyAttackInfo {
  enum EnemyAttackType type;
  EntityStatDiff damage;
  EntityStat minDodgeAgility;
  EntityStat maxDodgeAgility;
};

// Do not use outside of backend
struct CombatEventInfo {
  enum CombatEventCause cause;
  enum CombatEventAction action;
  EntityStatDiff damage; // Only set if action == AttackCombatEventAction
  size_t enemyID;        // Only set if action == AttackCombatEventAction
  bool playerAbsorbed;   // Only set if cause == EnemyCombatEventCause and action == AttackCombatEventAction
};

// Do not use outside of backend
struct CombatInfo {
  bool inCombat;
  bool changedEquipment;
  bool playerWon;

  struct CombatEventInfo combatEventInfo[CombatEventInfoCount];
  size_t lastReadCombatEventInfoID;
  size_t lastWriteCombatEventInfoID;

  bool performingEnemyAttacks;
  size_t currentEnemyID; // Only set if performingEnemyAttacks == true
  size_t enemyCount; // enemyCount <= MaxEnemyCount
  struct EnemyInfo enemies[MaxEnemyCount];
};

bool StartCombat(const struct GameInfo *restrict, struct GameState *restrict);
enum InputOutcome HandleGameCombat(const struct GameInfo *restrict, struct GameState *restrict, size_t);
bool UpdateCombat(struct GameState *restrict);
const char *CreateCombatString(const struct GameInfo *restrict, struct GameState *restrict);

// TODO: Individual attacks enemies can use with status' and stuff idk
// would have attack list with things like descriptor (blasted, stabbed etc.)

#endif // PCGAME_COMBAT_H
