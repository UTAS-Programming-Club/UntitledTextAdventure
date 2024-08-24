#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "entities.h"
#include "equipment.h"
#include "game.h"
#include "stringhelpers.h"

bool ApplyPlayerDamage(struct PlayerInfo *playerInfo, EntityStatDiff diff) {
  // calculate actual diff based on defences
  diff = diff + playerInfo->physDef;
  if(diff >= 0) {
    diff = 0;
  }

  // TODO: Dodge chance based on agility or something return false
  ModifyPlayerStat(&playerInfo->health, diff);
  return true;
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


// TODO: Record game state
void EnemyPerformAttack(struct GameState *state, const struct EnemyInfo *enemy) {
  // TODO: loops throuhg enemy array and takes their actions
  // TODO: choses random attack
  // TODO: if attack is physical/magical pass in as a bool

  // EntityStatDiff damage = enemy->damage;

  if(ApplyPlayerDamage(&state->playerInfo, enemy->damage)) {
    // TODO: make enemy attacks and events in combat modify string
  } else {
    // changes outcome if dodged
  }
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
    DStrPrintf(str, "Enemy %zu Health: %.*s%*s : %3i%%", i + 1,
      enemyHealthBarCount * blockSize, bar, 10 - enemyHealthBarCount, "", enemy->health
    );
    // DStrPrintf(str, "Enemy %zu Stamina: %.*s%*s : %3i%%\n\n",
    //   enemyStaminaBarCount * blockSize, bar, 10 - enemyStaminaBarCount, "", enemyIStamina
    // );
  // }

  return str->str;
}
