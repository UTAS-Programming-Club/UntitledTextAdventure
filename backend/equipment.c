#include <stdbool.h>
#include <stdint.h>
#include <types.h>

#include "equipment.h"
#include "game.h"

bool UpdatePlayerStat(PlayerStat *base, PlayerStatDiff diff) {
  if (!base || MaximumPlayerStat < *base
      || MinimumPlayerStatDiff > diff || MaximumPlayerStatDiff < diff) {
    return false;
  }

  if (0 > diff && MinimumPlayerStat > *base + diff) {
    *base = MinimumPlayerStat;
  } else if (0 < diff && MaximumPlayerStat < *base + diff) {
    *base = MaximumPlayerStat;
  } else {
    *base += diff;
  }
  return true;
}

bool UpdateStats(const struct GameInfo *info, struct GameState *state) {
  if (!info || !state) {
    return false;
  }

  state->playerInfo.physAtk = MinimumPlayerStat;
  state->playerInfo.magAtk = MinimumPlayerStat;
  state->playerInfo.physDef = MinimumPlayerStat;
  state->playerInfo.magDef = MinimumPlayerStat;

  // TODO: Add type for equipmentType
  for (uint_fast8_t i = 0; i < EquipmentTypeCount; ++i) {
    const struct EquipmentInfo *item = info->equipment + state->playerInfo.equippedItems[i];
    if (!item) {
      continue;
    }

    UpdatePlayerStat(&state->playerInfo.physAtk, item->physAtkMod);
    UpdatePlayerStat(&state->playerInfo.magAtk,  item->magAtkMod);
    UpdatePlayerStat(&state->playerInfo.physDef, item->physDefMod);
    UpdatePlayerStat(&state->playerInfo.magDef,  item->magDefMod);
  }

  return true;
}


// TODO: Add type for equipmentSlot
EquipmentID GetEquippedItemID(const struct GameState *state, uint_fast8_t equipmentSlot) {
  if (!state || EquipmentTypeCount <= equipmentSlot) {
    return InvalidEquipmentID;
  }

  EquipmentID id = state->playerInfo.equippedItems[equipmentSlot];
  if (EquipmentCount <= id) {
    return InvalidEquipmentID;
  }

  return id;
}

struct EquipmentInfo *GetEquippedItem(const struct GameInfo *info, const struct GameState *state, uint_fast8_t equipmentSlot) {
  if (!info || !info->initialised || !state || EquipmentTypeCount <= equipmentSlot) {
    return NULL;
  }

  EquipmentID id = GetEquippedItemID(state, equipmentSlot);
  if (InvalidEquipmentID == id) {
    return NULL;
  }

  return info->equipment + id;
}

bool SetEquippedItem(struct GameState *state, uint_fast8_t equipmentSlot, EquipmentID newID) {
  if (!state || EquipmentTypeCount <= equipmentSlot || EquipmentCount <= newID
      || InvalidEquipmentID == newID) {
    return false;
  }

  state->playerInfo.equippedItems[equipmentSlot] = newID;
  return true;
}
