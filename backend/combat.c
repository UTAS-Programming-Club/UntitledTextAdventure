#include <stdbool.h> // bool, false, true
#include <string.h>  // size_t, strlen
#include <types.h>   // defines: MaximumEntityStat, PRIEntityStatDiff
                     // enums: CombatEventCause, EnemyAttackType
                     // types: EntityStat, EntityStatDiff

#include "../frontends/frontend.h"
#include "combat.h"        // structs: CombatEventInfo, EnemyAttackInfo
#include "entities.h"      // structs: EnemyInfo, PlayerInfo
#include "game.h"          // struct GameInfo, struct GameState
#include "stringhelpers.h" // struct DStr, DStrAppend, DStrNew, DStrPrintf

static EntityStatDiff ApplyPlayerAgility(const struct PlayerInfo *restrict player, const struct EnemyAttackInfo *restrict attackInfo) {
  // Performs (agility_max - min(agility_max, max(agility_max, agility))) / (agility_max - agility_min)
  // Turns x in [0, 100] to [0, 1] for the percentage of agility in [min, max]
  // e.g. x <= min -> 0, x half way between min and max -> 0.5, max <= x -> 1
  EntityStat playerAgility = player->agility;
  if (playerAgility <= attackInfo->minDodgeAgility) {
      playerAgility = attackInfo->minDodgeAgility;
  } else if (playerAgility >= attackInfo->maxDodgeAgility) {
      playerAgility = attackInfo->maxDodgeAgility;
  }

  return ((attackInfo->maxDodgeAgility - playerAgility) * attackInfo->damage) /
         (attackInfo->maxDodgeAgility - attackInfo->minDodgeAgility);
}

static size_t DecMod(size_t val, size_t mod) {
  if (val == 0) {
    val = mod;
  }
  return val - 1;
}

bool StartCombat(const struct GameInfo *restrict info, struct GameState *restrict state) {
  if (!info || !info->initialised || !state) {
    return false;
  }

  state->combatInfo.inCombat = true;

  for (size_t i = 0; i < CombatEventInfoCount; ++i) {
    state->combatInfo.combatEventInfo[i].cause = UnusedCombatEventCause;
  }
  state->combatInfo.lastReadCombatEventInfoID = 0;
  state->combatInfo.lastWriteCombatEventInfoID = 0;
  state->combatInfo.performingEnemyAttacks = false;

  const struct RoomInfo *room = GetCurrentGameRoom(info, state);
  if (room->type == InvalidRoomType) {
    return false;
  }

  state->combatInfo.enemyCount = room->enemyCount;
  for (size_t i = 0; i < state->combatInfo.enemyCount; ++i) {
    struct EnemyInfo *enemy = &state->combatInfo.enemies[i];
    enemy->health = MaximumEntityStat;
    enemy->attackID = room->enemies[i];
  }

  return true;
}

bool PlayerPerformAttack(const struct GameInfo *restrict info, struct GameState *restrict state, size_t enemyID) {
  if (!info || !info->initialised || !state || enemyID >= state->combatInfo.enemyCount) {
    return false;
  }

  struct EnemyInfo *enemy = &state->combatInfo.enemies[enemyID];
  if (0 == enemy->health) {
    return false;
  }

  // TODO: Add physical and magic defense stats to enemies
  // TODO: Allow using secondary weapon
  // TODO: Add crits/some randomness to damage done
  EntityStat originalHealth = enemy->health;
  ModifyEntityStat(&enemy->health, state->playerInfo.priPhysAtk);
  ModifyEntityStat(&enemy->health, state->playerInfo.priMagAtk);

  state->combatInfo.lastWriteCombatEventInfoID =
    DecMod(state->combatInfo.lastWriteCombatEventInfoID, CombatEventInfoCount);

  struct CombatEventInfo *eventInfo = &state->combatInfo.combatEventInfo[
    state->combatInfo.lastWriteCombatEventInfoID
  ];
  eventInfo->cause = PlayerCombatEventCause;
  eventInfo->damage = enemy->health - originalHealth;
  eventInfo->enemyID = enemyID;

  return true;
}

bool EnemyPerformAttack(const struct GameInfo *restrict info, struct GameState *restrict state) {
  // TODO: Allow enemies to have multiple attacks?
  // TODO: Add crits/some randomness to damage done
  if (!info || !info->initialised || !state) {
    return false;
  }

  size_t enemyID = state->combatInfo.currentEnemyID;
  const struct EnemyInfo *enemy = &state->combatInfo.enemies[enemyID];
  const struct EnemyAttackInfo *enemyAttack = &info->enemyAttacks[enemy->attackID];
  if (0 == enemy->health) {
    return false;
  }

  EntityStatDiff dodgedDamage = ApplyPlayerAgility(&state->playerInfo, enemyAttack);
  EntityStatDiff absorbedDamage = dodgedDamage;

  switch (enemyAttack->type) {
    case PhysEnemyAttackType:
      absorbedDamage += state->playerInfo.physDef;
      break;
    case MagEnemyAttackType:
      absorbedDamage += state->playerInfo.magDef;
      break;
    default:
      PrintError("Attacking enemy has an invalid attack type");
      return false;
  }

  if (0 < absorbedDamage) {
    absorbedDamage = 0;
  }

  ModifyEntityStat(&state->playerInfo.health, absorbedDamage);

  state->combatInfo.lastWriteCombatEventInfoID =
    DecMod(state->combatInfo.lastWriteCombatEventInfoID, CombatEventInfoCount);

  struct CombatEventInfo *eventInfo = &state->combatInfo.combatEventInfo[
    state->combatInfo.lastWriteCombatEventInfoID
  ];
  eventInfo->cause = EnemyCombatEventCause;
  eventInfo->damage = absorbedDamage;
  eventInfo->enemyID = enemyID;
  eventInfo->playerAbsorbed = dodgedDamage < absorbedDamage;

  return true;
}

#define LINE_ENDING ".\n"
#define LOG_LINE_START "⬤ " // Black Large Circle

const char *CreateCombatString(const struct GameInfo *restrict info, struct GameState *restrict state) {
  if (!info || !info->initialised || !state) {
    return NULL;
  }

  struct DStr *str = DStrNew(&state->arena);
  if (!str) {
    return NULL;
  }

  DStrAppend(str, "You find yourself surrounded.\n\n");

  // combatEventInfo order:
  // 0: Enemy n - 1 event
  // ...
  // n - 1: Enemy 1 event
  // n: Enemy 0 event
  // n + 1: Player event(s)
  // ...
  // If an enemy is dead, no event will exist for them
  // Listed IDs are the i in (lastReadCombatEventInfoID - i) % CombatEventInfoCount
  size_t eventID = DecMod(state->combatInfo.lastReadCombatEventInfoID, CombatEventInfoCount);
  // bool foundPlayerEventID = false;
  const struct CombatEventInfo *event = &state->combatInfo.combatEventInfo[eventID];

  // TODO: Fix this disappearing during rounds
  if (!state->combatInfo.performingEnemyAttacks) {
    // TODO: Add rest turns for stamina recovery
    // TODO: Mention magic attacks and other items, splash items?
    if (event->cause == PlayerCombatEventCause) {
      DStrPrintf(str, "You attacked enemy %i with your sword", event->enemyID + 1);
      if (0 == state->combatInfo.enemies[event->enemyID].health) {
        DStrAppend(str, " and it died");
      }
      DStrAppend(str, LINE_ENDING);
      // if (EnemyDodged) {
      //    DStrAppend(str, "but they dodged" LINE_ENDING);
      //  } else if (EnemyFailedToDodge) {
      //    DStrAppend(str, ", they tried to dodge but you still managed to hit them" LINE_ENDING);
      //  } else {
      //    DStrAppend(str, "and managed to hit them" LINE_ENDING);
      //  }
      eventID = DecMod(eventID, CombatEventInfoCount);
    }

    for (size_t i = 0; i < CombatEventInfoCount; ++i) {
      event = &state->combatInfo.combatEventInfo[eventID];
      if (eventID < state->combatInfo.lastWriteCombatEventInfoID
          || EnemyCombatEventCause != event->cause) {
        break;
      }

      const struct EnemyInfo *enemy = &state->combatInfo.enemies[event->enemyID];
      const struct EnemyAttackInfo *enemyAttack = &info->enemyAttacks[enemy->attackID];
      bool playerPartiallyDodged = state->playerInfo.agility > enemyAttack->minDodgeAgility;
      bool playerFullyDodged = state->playerInfo.agility >= enemyAttack->maxDodgeAgility;

      DStrPrintf(str, "Enemy %zu ", event->enemyID + 1);
      switch (enemyAttack->type) {
        case PhysEnemyAttackType:
          // TODO: Allow enemies to have different kinds of physical attacks
          // full dodge, no/partial/full absorb
          if (playerFullyDodged) {
            DStrAppend(str, "tried to attack");
          } else {
            DStrAppend(str, "attacked");
          }
          DStrAppend(str, " you with their sword");
          // full dodge, no/partial/full absorb
          if (playerFullyDodged) {
            DStrAppend(str, " but missed");
          // no/partial dodge, full absorb
          } else if (event->playerAbsorbed && event->damage == 0) {
            DStrAppend(str, " but your armour absorbed the impact");
          // partial dodge, no absorb
          } else if (playerPartiallyDodged && !event->playerAbsorbed) {
            DStrAppend(str, ", you tried to dodge but were still hit");
          // partial dodge, partial absorb. playerPartiallyDodged would be enough but this is better for clarity
          } else if (playerPartiallyDodged && event->playerAbsorbed) {
            DStrAppend(str, ", you tried to dodge but were still hit with your armour softening the blow");
          // no dodge, partial absorb. event->playerAbsorbed would again be enough
          } else if (!playerPartiallyDodged && event->playerAbsorbed) {
            DStrAppend(str, " but your armour softened the blow");
          }
          break;
        case MagEnemyAttackType:
          // TODO: Support multiple types of magic attacks
          // full dodge, no/partial/full absorb
          if (playerFullyDodged) {
            DStrAppend(str, "tried to launch");
          } else {
            DStrAppend(str, "launched");
          }
          DStrAppend(str, " a fireball at you");
          // full dodge, no/partial/full absorb
          if (playerFullyDodged) {
            DStrAppend(str, " but it missed");
          // no/partial dodge, full absorb
          } else if (event->playerAbsorbed && event->damage == 0) {
            DStrAppend(str, " but your armour absorbed the impact");
          // partial dodge, no absorb
          } else if (playerPartiallyDodged && !event->playerAbsorbed) {
            DStrAppend(str, ", you tried to dodge it but were still hit");
          // partial dodge, partial absorb. playerPartiallyDodged would be enough but this is better for clarity
          } else if (playerPartiallyDodged && event->playerAbsorbed) {
            DStrAppend(str, ", you tried to dodge it but were still hit with your armour softening the blow");
          // no dodge, partial absorb. event->playerAbsorbed would again be enough
          } else if (!playerPartiallyDodged && event->playerAbsorbed) {
            DStrAppend(str, " but your armour softened the blow");
          }
          break;
        case InvalidEnemyAttackType:
          PrintError("Recorded combat event involved an invalid attack type");
          return NULL;
      }
      DStrAppend(str, LINE_ENDING);
      eventID = DecMod(eventID, CombatEventInfoCount);
    }

    state->combatInfo.lastReadCombatEventInfoID = (eventID + 1) % CombatEventInfoCount;
  }

  char bar[] = "██████████";
  size_t blockSize = strlen("█");

  uint_fast8_t playerHealthBarCount = (state->playerInfo.health + 9) / 10;
  uint_fast8_t playerStaminaBarCount = (state->playerInfo.stamina + 9) / 10;
  DStrPrintf(str, "\nYour Health:  %.*s%*s : %3i%%\n",
    playerHealthBarCount * blockSize, bar, 10 - playerHealthBarCount, "", state->playerInfo.health
  );
  DStrPrintf(str, "Your Stamina: %.*s%*s : %3i%%\n\n",
    playerStaminaBarCount * blockSize, bar, 10 - playerStaminaBarCount, "", state->playerInfo.stamina
  );

  // TODO: Add enemy stamina
  for (size_t i = 0; i < state->combatInfo.enemyCount; ++i) {
    DStrPrintf(str, "Enemy %zu ", i + 1);
    if (0 == state->combatInfo.enemies[i].health) {
      DStrAppend(str, "is dead");
    } else {
      int enemyHealthBarCount = (state->combatInfo.enemies[i].health + 9) / 10;
      // int enemyStaminaBarCount = (state->combatInfo.enemies[i].stamina + 9) / 10;
      DStrPrintf(str, "Health: %.*s%*s : %3i%%", enemyHealthBarCount * blockSize,
        bar, 10 - enemyHealthBarCount, "", state->combatInfo.enemies[i].health
      );
      // DStrPrintf(str, "Enemy %zu Stamina: %.*s%*s : %3i%%", i + 1,
      //   bar, 10 - enemyStaminaBarCount, "", state->combatInfo.enemies[i].stamina
      // );
    }
    DStrAppend(str, "\n");
  }

  eventID = state->combatInfo.lastWriteCombatEventInfoID;
  event = &state->combatInfo.combatEventInfo[eventID];

  if (UnusedCombatEventCause != event->cause) {
    DStrAppend(str, "\nCombat log:\n");

    for (size_t i = 0; UnusedCombatEventCause != event->cause; ++i) {
      switch (event->cause) {
        case PlayerCombatEventCause:
          // TODO: Record attack type for player
          DStrPrintf(str, LOG_LINE_START "You did %" PRIEntityStatDiff
                                         " physical damage to enemy %zu\n",
                     -event->damage, event->enemyID + 1);
          break;
        case EnemyCombatEventCause: ;
          DStrPrintf(str, LOG_LINE_START "Enemy %zu did %" PRIEntityStatDiff " ",
                     event->enemyID + 1, -event->damage);
          const struct EnemyInfo *enemy = &state->combatInfo.enemies[event->enemyID];
          switch (info->enemyAttacks[enemy->attackID].type) {
            case PhysEnemyAttackType:
              DStrAppend(str, "physical");
              break;
            case MagEnemyAttackType:
              DStrAppend(str, "magic");
              break;
            case InvalidEnemyAttackType:
              PrintError("Recorded combat event involved an invalid attack type");
              return NULL;
          }
          DStrAppend(str, " damage to you\n");
          break;
        case UnusedCombatEventCause:
          break;
        case InvalidCombatEventCause:
          PrintError("Recorded combat event was caused by an invalid entity");
          return NULL;
      }

      eventID = (eventID + 1) % CombatEventInfoCount;
      if (eventID == state->combatInfo.lastWriteCombatEventInfoID) {
        break;
      }
      event = &state->combatInfo.combatEventInfo[eventID];
    }
  }

  if (state->combatInfo.performingEnemyAttacks) {
    DStrPrintf(str, "\nWaiting for enemy %zu to attack" LINE_ENDING, state->combatInfo.currentEnemyID + 1);
  }

  return str->str;
}
