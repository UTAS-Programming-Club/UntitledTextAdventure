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

static struct CombatEventInfo *GetNextEventInfo(struct GameState *restrict state) {
  state->combatInfo.lastWriteCombatEventInfoID =
    DecMod(state->combatInfo.lastWriteCombatEventInfoID, CombatEventInfoCount);

  return &state->combatInfo.combatEventInfo[state->combatInfo.lastWriteCombatEventInfoID];
}

static bool PlayerPerformAttack(const struct GameInfo *restrict info, struct GameState *restrict state, size_t enemyID) {
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

  struct CombatEventInfo *eventInfo = GetNextEventInfo(state);
  eventInfo->cause = PlayerCombatEventCause;
  eventInfo->action = AttackCombatEventAction;
  eventInfo->damage = enemy->health - originalHealth;
  eventInfo->enemyID = enemyID;

  return true;
}

static bool EnemyPerformAttack(const struct GameInfo *restrict info, struct GameState *restrict state) {
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

  struct CombatEventInfo *eventInfo = GetNextEventInfo(state);
  eventInfo->cause = EnemyCombatEventCause;
  eventInfo->action = AttackCombatEventAction;
  eventInfo->damage = absorbedDamage;
  eventInfo->enemyID = enemyID;
  eventInfo->playerAbsorbed = dodgedDamage < absorbedDamage;

  return true;
}

enum InputOutcome HandleGameCombat(const struct GameInfo *restrict info, struct GameState *restrict state, size_t attackedEnemyID) {
  if (!state->combatInfo.performingEnemyAttacks) {
    if (!PlayerPerformAttack(info, state, attackedEnemyID)) {
      return InvalidInputOutcome;
    }

    state->combatInfo.performingEnemyAttacks = true;
    state->combatInfo.currentEnemyID = 0;
    return GetNextOutputOutcome;
  }

  if (state->combatInfo.performingEnemyAttacks) {
    size_t *curEnemyID = &state->combatInfo.currentEnemyID;
    while (0 == state->combatInfo.enemies[*curEnemyID].health && *curEnemyID < state->combatInfo.enemyCount) {
      ++*curEnemyID;
    }
    if (*curEnemyID < state->combatInfo.enemyCount && !EnemyPerformAttack(info, state)) {
      return InvalidInputOutcome;
    }
    ++*curEnemyID;

    if (*curEnemyID >= state->combatInfo.enemyCount) {
      state->combatInfo.performingEnemyAttacks = false;
    }

    return GetNextOutputOutcome;
  }

  return InvalidInputOutcome;
}

#define LINE_ENDING ".\n"
#define LOG_LINE_START "⬤ " // Black Large Circle

bool UpdateCombat(struct GameState *restrict state) {
  if (!state) {
    return false;
  }

  if (state->combatInfo.changedEquipment) {
    state->combatInfo.performingEnemyAttacks = true;
    state->combatInfo.currentEnemyID = 0;

    state->combatInfo.changedEquipment = false;

    struct CombatEventInfo *eventInfo = GetNextEventInfo(state);
    eventInfo->cause = PlayerCombatEventCause;
    eventInfo->action = EquipmentSwapCombatEventAction;
  }

  state->combatInfo.playerWon = true;
  for (size_t i = 0; i < state->combatInfo.enemyCount; ++i) {
    if (0 != state->combatInfo.enemies[i].health) {
      state->combatInfo.playerWon = false;
      break;
    }
  }

  return true;
}

// TODO: Move to json
const char *const AttackNames[] = {"Physical", "Magical"};

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
    if (event->cause == PlayerCombatEventCause && event->action == AttackCombatEventAction) {
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
    } else if (event->cause == PlayerCombatEventCause && event->action == EquipmentSwapCombatEventAction) {
      DStrAppend(str, "You changed your equipment" LINE_ENDING);
      eventID = DecMod(eventID, CombatEventInfoCount);
    } else if (event->cause == PlayerCombatEventCause) {
      PrintError("Invalid player event action detected");
      return NULL;
    }

    for (size_t i = 0; i < CombatEventInfoCount; ++i) {
      event = &state->combatInfo.combatEventInfo[eventID];
      if (eventID < state->combatInfo.lastWriteCombatEventInfoID
          || EnemyCombatEventCause != event->cause) {
        break;
      }

      if (event->action != AttackCombatEventAction) {
        PrintError("Invalid enemy event action detected");
        return NULL;
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

  size_t longestUsedAttackName = 0;
  for (size_t i = 0; i < state->combatInfo.enemyCount; ++i) {
    const struct EnemyInfo *enemy = &state->combatInfo.enemies[i];
    if (info->enemyAttackCount <= enemy->attackID) {
      return NULL;
    }

    // TODO: Check if this is worth checking, does parser.c already do it?
    enum EnemyAttackType attackType = info->enemyAttacks[enemy->attackID].type;
    if (InvalidEnemyAttackType == attackType || MaxEnemyAttackType < attackType) {
      return NULL;
    }

    size_t attackNameLen = strlen(AttackNames[attackType - 1]);
    longestUsedAttackName = attackNameLen > longestUsedAttackName ? attackNameLen : longestUsedAttackName;
  }

  // TODO: Add enemy stamina
  for (size_t i = 0; i < state->combatInfo.enemyCount; ++i) {
    const struct EnemyInfo *enemy = &state->combatInfo.enemies[i];
    enum EnemyAttackType attackType = info->enemyAttacks[enemy->attackID].type;
    const char *attackName = AttackNames[attackType - 1];
    size_t attackNameLen = strlen(attackName);

    DStrPrintf(str, "%*s%s enemy %zu ", longestUsedAttackName - attackNameLen, "", attackName, i + 1);
    if (0 == enemy->health) {
      DStrAppend(str, "is dead");
    } else {
      int enemyHealthBarCount = (enemy->health + 9) / 10;
      // int enemyStaminaBarCount = (enemy->stamina + 9) / 10;
      DStrPrintf(str, "health: %.*s%*s : %3i%%", enemyHealthBarCount * blockSize,
        bar, 10 - enemyHealthBarCount, "", enemy->health
      );
      // DStrPrintf(str, "%*sstamina: %.*s%*s : %3i%%", longestUsedAttackName + 9, "",
      //   enemyStaminaBarCount * blockSize, bar, 10 - enemyStaminaBarCount, "", 59
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
          switch (event->action) {
            case AttackCombatEventAction:
              DStrPrintf(str, LOG_LINE_START "You did %" PRIEntityStatDiff
                                             " physical damage to enemy %zu\n",
                         -event->damage, event->enemyID + 1);
              break;
            case EquipmentSwapCombatEventAction:
              DStrAppend(str, LOG_LINE_START "You changed your equipment\n");
              break;
            case InvalidCombatEventAction:
              PrintError("Invalid player event action detected");
              return NULL;
          }

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
