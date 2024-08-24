#include <stdbool.h>
#include <stdint.h>
#include <types.h>

#include "entities.h"
#include "equipment.h"
#include "game.h"

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
