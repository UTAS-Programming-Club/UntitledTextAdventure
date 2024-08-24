#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../frontends/frontend.h"
#include "entities.h"
#include "equipment.h"
#include "game.h"
#include "stringhelpers.h"

static EntityStat ApplyPlayerAgility(const struct PlayerInfo *playerInfo, const struct EnemyAttackInfo *attackInfo) {
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


bool EnemyPerformAttack(struct GameState *state, const struct EnemyInfo *enemyInfo) {
  // TODO: Loop through enemy array and takes their actions, put elsewhere?
  // TODO: Allow enemies to have multiple attacks?
  // TODO: Record outcomes for CreateCombatString
  // TODO: Add crits/some randomness to damage done

  EntityStatDiff damage = ApplyPlayerAgility(&state->playerInfo, &enemyInfo->attackInfo);

  switch (enemyInfo->attackInfo.type) {
    case PhysEnemyAttackType:
      damage += state->playerInfo.physDef;
      break;
    case MagEnemyAttackType:
      damage += state->playerInfo.magDef;
      break;
    default:
      PrintError("Attacking enemy has an invalid attack type");
      return false;
  }

  if(damage >= 0) {
    return true;
  }

  ModifyPlayerStat(&state->playerInfo.health, damage);

  // TODO: Use ssize_t to avoid this?
  if (state->lastCombatEventInfoIdx == 0) {
    state->lastCombatEventInfoIdx = 10;
  }
  --state->lastCombatEventInfoIdx;
  struct CombatEventInfo *eventInfo = &state->combatEventInfo[state->lastCombatEventInfoIdx];
  eventInfo->cause = EnemyCombatEventCause;
  eventInfo->damage = damage;
  eventInfo->enemyInfo = enemyInfo;

  return true;
}

#define LINE_ENDING ".\n"

const char *CreateCombatString(struct GameState *state, const struct EnemyInfo *enemy) {
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

  // TODO: Mention magic attacks
  // TODO: Mention rest turns for stamina recovery
  // for (size_t i = 0; i < enemyCount; ++i) {
  // size_t i = 0;
  //   if (EnemyIAttackedPlayer) {
  //     DStrPrintf(str, "Enemy %zu swung at you with their sword", i);
  //     if (PlayerDodgedEnemyIAttack) {
  //       DStrAppend(str, "but you dodged" LINE_ENDING);
  //     } else if (PlayerFailedToDodgeEnemyIAttack) {
  //       DStrAppend(str, ", you tried to dodge but they still managed to hit you" LINE_ENDING);
  //     } else {
  //       DStrAppend(str, "and managed to hit you" LINE_ENDING);
  //     }
  //     if (!PlayerDodgedEnemyIAttack) {
  //     }
  //   }
  // }

  // DStrAppend(str, "\n");

  char bar[] = "██████████";
  size_t blockSize = strlen("█");

  uint_fast8_t playerHealthBarCount = (state->playerInfo.health + 9) / 10;
  uint_fast8_t playerStaminaBarCount = (state->playerInfo.stamina + 9) / 10;
  DStrPrintf(str, "Your Health:  %.*s%*s : %3i%%\n",
    playerHealthBarCount * blockSize, bar, 10 - playerHealthBarCount, "", state->playerInfo.health
  );
  DStrPrintf(str, "Your Stamina: %.*s%*s : %3i%%\n\n",
    playerStaminaBarCount * blockSize, bar, 10 - playerStaminaBarCount, "", state->playerInfo.stamina
  );

  // TODO: Support multiple enemies
  // TODO: Add enemy stamina
  // for (size_t i = 0; i < enemyCount; ++i) {
  size_t i = 0;
    int enemyHealthBarCount = (enemy->health + 9) / 10;
    // int enemyStaminaBarCount = (enemyIStamina + 9) / 10;
    DStrPrintf(str, "Enemy %zu Health: %.*s%*s : %3i%%\n", i + 1,
      enemyHealthBarCount * blockSize, bar, 10 - enemyHealthBarCount, "", enemy->health
    );
    // DStrPrintf(str, "Enemy %zu Stamina: %.*s%*s : %3i%%\n\n",
    //   enemyStaminaBarCount * blockSize, bar, 10 - enemyStaminaBarCount, "", enemyIStamina
    // );
  // }

  size_t eventInfoIdx = state->lastCombatEventInfoIdx;
  struct CombatEventInfo *eventInfo = &state->combatEventInfo[eventInfoIdx];
  if (eventInfo->cause != UnusedCombatEventCause) {
    DStrAppend(str, "\nCombat log:\n");
  }
  while (eventInfo->cause != UnusedCombatEventCause) {
    switch (state->combatEventInfo[eventInfoIdx].cause) {
      case PlayerCombatEventCause:
        DStrPrintf(str, "You attacked the enemy and did %" PRIEntityStatDiff " damage.\n", -eventInfo->damage);
        break;
      case EnemyCombatEventCause:
        DStrPrintf(str, "The enemy attacked you and did %" PRIEntityStatDiff " damage.\n", -eventInfo->damage);
        break;
      default:
        PrintError("Recorded combat event was caused by an invalid entity");
        return NULL;
    }

    eventInfoIdx = (eventInfoIdx + 1) % CombatEventInfoCount;
    eventInfo = &state->combatEventInfo[eventInfoIdx];
    if (eventInfoIdx == state->lastCombatEventInfoIdx) {
      break;
    }
  }

  return str->str;
}
