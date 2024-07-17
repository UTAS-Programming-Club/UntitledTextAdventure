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


bool UnlockItem(struct PlayerInfo *playerInfo, EquipmentID ID) {
  if (!playerInfo || EquipmentCount <= ID || InvalidEquipmentID == ID) {
    return false;
  }

  playerInfo->unlockedItems[ID] = true;
  return true;
}

bool CheckItemUnlocked(const struct PlayerInfo *playerInfo, EquipmentID ID, bool *unlocked) {
  if (!playerInfo || EquipmentCount <= ID || InvalidEquipmentID == ID) {
    return false;
  }

  *unlocked = playerInfo->unlockedItems[ID];
  return true;
}


// TODO: Add type for equipmentSlot
EquipmentID GetEquippedItemID(const struct PlayerInfo *playerInfo, uint_fast8_t equipmentSlot) {
  if (!playerInfo || EquipmentTypeCount <= equipmentSlot) {
    return InvalidEquipmentID;
  }

  EquipmentID id = playerInfo->equippedItems[equipmentSlot];
  if (EquipmentCount <= id) {
    return InvalidEquipmentID;
  }

  return id;
}

struct EquipmentInfo *GetEquippedItem(const struct GameInfo *info, const struct PlayerInfo *playerInfo, uint_fast8_t equipmentSlot) {
  if (!info || !info->initialised || !playerInfo || EquipmentTypeCount <= equipmentSlot) {
    return NULL;
  }

  EquipmentID id = GetEquippedItemID(playerInfo, equipmentSlot);
  if (InvalidEquipmentID == id) {
    return NULL;
  }

  return info->equipment + id;
}

bool SetEquippedItem(struct PlayerInfo *playerInfo, uint_fast8_t equipmentSlot, EquipmentID newID) {
  if (!playerInfo || EquipmentTypeCount <= equipmentSlot || EquipmentCount <= newID
      || InvalidEquipmentID == newID) {
    return false;
  }

  if (!playerInfo->unlockedItems[newID]) {
    return false;
  }

  playerInfo->equippedItems[equipmentSlot] = newID;
  return true;
}
