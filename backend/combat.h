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
// TODO: Track events other than attacks
struct CombatEventInfo {
  enum CombatEventCause cause;
  EntityStatDiff damage;
  size_t enemyID;
  bool playerAbsorbed; // Only set if cause == EnemyCombatEventCause
};

// Do not use outside of backend
struct CombatInfo {
  bool inCombat;

  struct CombatEventInfo combatEventInfo[CombatEventInfoCount];
  size_t lastReadCombatEventInfoID;
  size_t lastWriteCombatEventInfoID;

  bool performingEnemyAttacks;
  size_t currentEnemyID; // Only set if performingEnemyAttacks == true
  size_t enemyCount; // enemyCount <= MaxEnemyCount
  struct EnemyInfo enemies[MaxEnemyCount];
};

bool StartCombat(const struct GameInfo *restrict, struct GameState *restrict);
bool PlayerPerformAttack(const struct GameInfo *restrict, struct GameState *restrict, size_t);
bool EnemyPerformAttack(const struct GameInfo *restrict, struct GameState *restrict);
const char *CreateCombatString(const struct GameInfo *restrict, struct GameState *restrict);

// TODO: Individual attacks enemies can use with status' and stuff idk
// would have attack list with things like descriptor (blasted, stabbed etc.)

#endif // PCGAME_COMBAT_H
