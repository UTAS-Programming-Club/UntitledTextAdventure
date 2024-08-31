#include <stdbool.h>
#include <stdint.h>
#include <types.h>

#include "entities.h"
#include "equipment.h"
#include "game.h"

bool UnlockItem(struct PlayerInfo *player, EquipmentID ID) {
  if (!player || EquipmentCount <= ID || InvalidEquipmentID == ID) {
    return false;
  }

  player->unlockedItems[ID] = true;
  return true;
}

bool CheckItemUnlocked(const struct PlayerInfo *player, EquipmentID ID, bool *unlocked) {
  if (!player || EquipmentCount <= ID || InvalidEquipmentID == ID) {
    return false;
  }

  *unlocked = player->unlockedItems[ID];
  return true;
}


EquipmentID GetEquippedItemID(const struct PlayerInfo *player, enum EquipmentType equipmentType) {
  if (!player || EquipmentTypeCount <= equipmentType) {
    return InvalidEquipmentID;
  }

  EquipmentID id = player->equippedItems[equipmentType];
  if (EquipmentCount <= id) {
    return InvalidEquipmentID;
  }

  return id;
}

const struct EquipmentInfo *GetEquippedItem(const struct GameInfo *info, const struct PlayerInfo *player, enum EquipmentType equipmentType) {
  if (!info || !info->initialised || !player || EquipmentTypeCount <= equipmentType) {
    return NULL;
  }

  EquipmentID id = GetEquippedItemID(player, equipmentType);
  if (InvalidEquipmentID == id) {
    return NULL;
  }

  return info->equipment + id;
}

bool SetEquippedItem(struct PlayerInfo *player, enum EquipmentType equipmentType, EquipmentID newID) {
  if (!player || EquipmentTypeCount <= equipmentType || EquipmentCount <= newID
      || InvalidEquipmentID == newID) {
    return false;
  }

  if (!player->unlockedItems[newID]) {
    return false;
  }

  player->equippedItems[equipmentType] = newID;
  return true;
}
