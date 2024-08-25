#include <stdbool.h> // bool, false, true
#include <string.h>  // size_t, strlen
#include <types.h>   // defines: EquipmentTypeCount, MaximumEntityStat, MaximumEntityStatDiff, MinimumEntityStat, MinimumEntityStatDiff, PRIEntityStatDiff
                     // enums: CombatEventCause, EnemyAttackType
                     // types: EntityStat, EntityStatDiff, EquipmentType

#include "../frontends/frontend.h"
#include "entities.h"       // structs: CombatEventInfo, EnemyAttackInfo, EnemyInfo, PlayerInfo
#include "equipment.h"      // struct EquipmentInfo
#include "game.h"           // struct GameInfo, struct GameState
#include "stringhelpers.h"  // struct DStr, DStrAppend, DStrNew, DStrPrintf

static EntityStatDiff ApplyPlayerAgility(const struct PlayerInfo *restrict playerInfo, const struct EnemyAttackInfo *restrict attackInfo) {
  // Performs (agility_max - min(agility_max, max(agility_max, agility))) / (agility_max - agility_min)
  // Turns x in [0, 100] to [0, 1] for the percentage of agility in [min, max]
  // e.g. x <= min -> 0, x half way between min and max -> 0.5, max <= x -> 1
  EntityStat playerAgility = playerInfo->agility;
  if (playerAgility <= attackInfo->minDodgeAgility) {
      playerAgility = attackInfo->minDodgeAgility;
  } else if (playerAgility >= attackInfo->maxDodgeAgility) {
      playerAgility = attackInfo->maxDodgeAgility;
  }

  return ((attackInfo->maxDodgeAgility - playerAgility) * attackInfo->damage) /
         (attackInfo->maxDodgeAgility - attackInfo->minDodgeAgility);
}

bool ModifyPlayerStat(EntityStat *base, EntityStatDiff diff) {
  if (!base || MaximumEntityStat < *base
      || MinimumEntityStatDiff > diff || MaximumEntityStatDiff < diff) {
    return false;
  }

  if (0 > diff && MinimumEntityStat > *base + diff) {
    *base = MinimumEntityStat;
  } else if (0 < diff && MaximumEntityStat < *base + diff) {
    *base = MaximumEntityStat;
  } else {
    *base += diff;
  }
  return true;
}

bool RefreshStats(const struct GameInfo *info, struct GameState *state) {
  if (!info || !state) {
    return false;
  }

  state->playerInfo.physAtk = MinimumEntityStat;
  state->playerInfo.magAtk = MinimumEntityStat;
  state->playerInfo.physDef = MinimumEntityStat;
  state->playerInfo.magDef = MinimumEntityStat;

  for (EquipmentType i = 0; i < EquipmentTypeCount; ++i) {
    const struct EquipmentInfo *item = info->equipment + state->playerInfo.equippedItems[i];
    if (!item) {
      continue;
    }

    ModifyPlayerStat(&state->playerInfo.physAtk, item->physAtkMod);
    ModifyPlayerStat(&state->playerInfo.magAtk,  item->magAtkMod);
    ModifyPlayerStat(&state->playerInfo.physDef, item->physDefMod);
    ModifyPlayerStat(&state->playerInfo.magDef,  item->magDefMod);
  }

  return true;
}


// TODO: Remove
extern size_t TestEnemyCount;
extern const struct EnemyInfo TestEnemies[];

bool EnemyPerformAttack(struct GameState *state, size_t enemyID) {
  // TODO: Loop through enemy array and takes their actions, put elsewhere?
  // TODO: Allow enemies to have multiple attacks?
  // TODO: Record outcomes for CreateCombatString
  // TODO: Add crits/some randomness to damage done
  if (!state || enemyID >= TestEnemyCount) {
    return false;
  }

  const struct EnemyInfo *enemyInfo = &TestEnemies[enemyID];
  EntityStatDiff dodgedDamage = ApplyPlayerAgility(&state->playerInfo, &enemyInfo->attackInfo);
  EntityStatDiff absorbedDamage = dodgedDamage;

  switch (enemyInfo->attackInfo.type) {
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

  ModifyPlayerStat(&state->playerInfo.health, absorbedDamage);

  // TODO: Use ssize_t to avoid this?
  if (state->combatInfo.lastCombatEventInfoID == 0) {
    state->combatInfo.lastCombatEventInfoID = CombatEventInfoCount;
  }
  --state->combatInfo.lastCombatEventInfoID;
  struct CombatEventInfo *eventInfo = &state->combatInfo.combatEventInfo[
    state->combatInfo.lastCombatEventInfoID
  ];
  // TODO: Mention dodging and armor absorption
  eventInfo->cause = EnemyCombatEventCause;
  eventInfo->damage = absorbedDamage;
  eventInfo->enemyID = enemyID;
  eventInfo->playerAbsorbed = dodgedDamage < absorbedDamage;

  return true;
}

#define LINE_ENDING ".\n"
#define LOG_LINE_START "⬤ " // Black Large Circle

const char *CreateCombatString(struct GameState *state, size_t enemyCount, const struct EnemyInfo *enemies) {
  if (!state || !enemies) {
    return NULL;
  }

  struct DStr *str = DStrNew(&state->arena);
  if (!str) {
    return NULL;
  }

  // TODO: Mention magic attacks and other items, splash items?
  // TODO: Mention rest turns for stamina recovery
  DStrAppend(str, "You find yourself surrounded.\n\n");
  // if (PlayerDidAttackWithSword) {
  //   DStrPrintf(str, "You swung at enemy %i with your %s", enemyID, swordName);
  //   if (EnemyDodged) {
  //     DStrAppend(str, "but they dodged" LINE_ENDING);
  //   } else if (EnemyFailedToDodge) {
  //     DStrAppend(str, ", they tried to dodge but you still managed to hit them" LINE_ENDING);
  //   } else {
  //     DStrAppend(str, "and managed to hit them" LINE_ENDING);
  //   }
  // }

  size_t eventID = state->combatInfo.lastCombatEventInfoID;
  struct CombatEventInfo *event;

  // combatEventInfo order:
  // 0: Enemy n - 1 event
  // ...
  // n - 1: Enemy 1 event
  // n: Enemy 0 event
  // n + 1: Player event(s)
  // ...
  // If an enemy is dead, no event will exist for them
  // Listed IDs are the i in (lastCombatEventInfoID + i) % CombatEventInfoCount
  // TODO: Add rest turns for stamina recovery
  // TODO: Reorder as 0 to n - 1?
  // NOTE: This will show all events in the log until player events are actually a thing, not worth fixing
  for (size_t i = 0; i < CombatEventInfoCount; ++i) {
    event = &state->combatInfo.combatEventInfo[
      (eventID + i) % CombatEventInfoCount
    ];
    if (event->cause != EnemyCombatEventCause) {
      break;
    }
    const struct EnemyInfo *enemy = &enemies[event->enemyID];
    DStrPrintf(str, "Enemy %zu ", event->enemyID + 1);
    switch (enemy->attackInfo.type) {
      case PhysEnemyAttackType:
        // TODO: Allow enemies to have different kinds of physical attacks
        bool playerPartiallyDodged = state->playerInfo.agility > enemy->attackInfo.minDodgeAgility;
        bool playerFullyDodged = state->playerInfo.agility >= enemy->attackInfo.maxDodgeAgility;
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
          DStrAppend(str, " but your armor absorbed the impact");
        // partial dodge, no absorb
        } else if (playerPartiallyDodged && !event->playerAbsorbed) {
          DStrAppend(str, ", you tried to dodge but were still hit");
        // partial dodge, partial absorb. playerPartiallyDodged would be enough but this is better for clarity
        } else if (playerPartiallyDodged && event->playerAbsorbed) {
          DStrAppend(str, ", you tried to dodge but were still hit with your armor softening the blow");
        // no dodge, partial absorb. event->playerAbsorbed would again be enough
        } else if (!playerPartiallyDodged && event->playerAbsorbed) {
          DStrAppend(str, " but your armor softened the blow");
        }
        break;
      case MagEnemyAttackType:
        // TODO: Repeat the above attack reporting for magic attacks
        // TODO: Support multiple types of magic
        DStrAppend(str, "launched a fireball at you");
        break;
      case InvalidEnemyAttackType:
        PrintError("Recorded combat event involved an invalid attack type");
        return NULL;
    }
    DStrAppend(str, LINE_ENDING);
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
  for (size_t i = 0; i < enemyCount; ++i) {
    int enemyHealthBarCount = (enemies[i].health + 9) / 10;
    // int enemyStaminaBarCount = (enemyIStamina + 9) / 10;
    DStrPrintf(str, "Enemy %zu Health: %.*s%*s : %3i%%\n", i + 1,
      enemyHealthBarCount * blockSize, bar, 10 - enemyHealthBarCount, "", enemies[i].health
    );
    // DStrPrintf(str, "Enemy %zu Stamina: %.*s%*s : %3i%%\n\n",
    //   enemyStaminaBarCount * blockSize, bar, 10 - enemyStaminaBarCount, "", enemyIStamina
    // );
  }

  event = &state->combatInfo.combatEventInfo[eventID];

  if (event->cause != UnusedCombatEventCause) {
    DStrAppend(str, "\nCombat log:\n");

    // TODO: Don't show 0 damage events?
    // TODO: Switch to a for loop like the one for last round reporting
    do {
      switch (event->cause) {
        case PlayerCombatEventCause:
          // TODO: Record attack type for player
          DStrPrintf(str, LOG_LINE_START "You did %" PRIEntityStatDiff " damage to enemy %zu\n",
                     -event->damage, event->enemyID + 1);
          break;
        case EnemyCombatEventCause: ;
          DStrPrintf(str, LOG_LINE_START "Enemy %zu did %" PRIEntityStatDiff " ",
                     event->enemyID + 1, -event->damage);
          switch (enemies[event->enemyID].attackInfo.type) {
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
        // TODO: Replace default with Invalid and Unused cases
        default:
          PrintError("Recorded combat event was caused by an invalid entity");
          return NULL;
      }

      eventID = (eventID + 1) % CombatEventInfoCount;
      event = &state->combatInfo.combatEventInfo[eventID];
      if (eventID == state->combatInfo.lastCombatEventInfoID) {
        break;
      }
    } while (event->cause != UnusedCombatEventCause);
  }

  if (state->combatInfo.performingEnemyAttacks) {
    DStrPrintf(str, "\nWaiting for enemy %zu to attack" LINE_ENDING, state->combatInfo.currentEnemyNumber + 1);
  }

  return str->str;
}
