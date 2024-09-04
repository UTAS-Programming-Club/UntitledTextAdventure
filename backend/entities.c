#include <stdbool.h> // bool, false, true
#include <types.h>   // defines: EquipmentTypeCount, MaximumEntityStat, MaximumEntityStatDiff, MinimumEntityStat, MinimumEntityStatDiff
                     // types: EntityStat, EntityStatDiff, EquipmentType

#include "../frontends/frontend.h"
#include "entities.h"       // structs: EnemyInfo, PlayerInfo
#include "equipment.h"      // struct EquipmentInfo
#include "game.h"           // struct GameInfo, struct GameState

bool ModifyEntityStat(EntityStat *restrict base, EntityStatDiff diff) {
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

static bool ModifyEntityStatDiff(EntityStatDiff *restrict base, EntityStatDiff diff) {
  if (!base || MinimumEntityStatDiff > *base || MaximumEntityStatDiff < *base
      || MinimumEntityStatDiff > diff || MaximumEntityStatDiff < diff) {
    return false;
  }

  if (MinimumEntityStatDiff > *base + diff) {
    *base = MinimumEntityStatDiff;
  } else if (MaximumEntityStat < *base + diff) {
    *base = MaximumEntityStatDiff;
  } else {
    *base += diff;
  }

  return true;
}

bool RefreshPlayerStats(const struct GameInfo *info, struct GameState *state) {
  if (!info || !info->initialised || !state) {
    return false;
  }

  state->playerInfo.priPhysAtk = MinimumEntityStat;
  state->playerInfo.priMagAtk = MinimumEntityStat;
  state->playerInfo.secPhysAtk = MinimumEntityStat;
  state->playerInfo.secMagAtk = MinimumEntityStat;
  state->playerInfo.physDef = MinimumEntityStat;
  state->playerInfo.magDef = MinimumEntityStat;

  for (enum EquipmentType i = 0; i < EquipmentTypeCount; ++i) {
    const struct EquipmentInfo *item = GetEquippedItem(info, &state->playerInfo, i);
    if (!item) {
      continue;
    }

    if (item->type != PriWeapEquipmentType) {
      ModifyEntityStatDiff(&state->playerInfo.secPhysAtk, item->physAtkMod);
      ModifyEntityStatDiff(&state->playerInfo.secMagAtk,  item->magAtkMod);
    }
    if (item->type != SecWeapEquipmentType) {
      ModifyEntityStatDiff(&state->playerInfo.priPhysAtk, item->physAtkMod);
      ModifyEntityStatDiff(&state->playerInfo.priMagAtk,  item->magAtkMod);
    }
    ModifyEntityStat(&state->playerInfo.physDef, item->physDefMod);
    ModifyEntityStat(&state->playerInfo.magDef,  item->magDefMod);
  }

  return true;
}
