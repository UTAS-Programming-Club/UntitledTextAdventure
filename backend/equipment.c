#include <stdbool.h>
#include <stdint.h>
#include <types.h>

#include "equipment.h"
#include "game.h"

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


EquipmentID GetEquippedItemID(const struct PlayerInfo *playerInfo, EquipmentType equipmentType) {
  if (!playerInfo || EquipmentTypeCount <= equipmentType) {
    return InvalidEquipmentID;
  }

  EquipmentID id = playerInfo->equippedItems[equipmentType];
  if (EquipmentCount <= id) {
    return InvalidEquipmentID;
  }

  return id;
}

struct EquipmentInfo *GetEquippedItem(const struct GameInfo *info, const struct PlayerInfo *playerInfo, EquipmentType equipmentType) {
  if (!info || !info->initialised || !playerInfo || EquipmentTypeCount <= equipmentType) {
    return NULL;
  }

  EquipmentID id = GetEquippedItemID(playerInfo, equipmentType);
  if (InvalidEquipmentID == id) {
    return NULL;
  }

  return info->equipment + id;
}

bool SetEquippedItem(struct PlayerInfo *playerInfo, EquipmentType equipmentType, EquipmentID newID) {
  if (!playerInfo || EquipmentTypeCount <= equipmentType || EquipmentCount <= newID
      || InvalidEquipmentID == newID) {
    return false;
  }

  if (!playerInfo->unlockedItems[newID]) {
    return false;
  }

  playerInfo->equippedItems[equipmentType] = newID;
  return true;
}
