package extensions.equipment;

import backend.Extension;
import extensions.equipment.Equipment;
import extensions.equipment.Rooms;

// TODO: Move to coregame, player depends on Equipments.hx
@:nullSafety(Strict)
final EquipmentExt: Extension = {
  module: 'extensions.equipment',
  actions: [],
  equipment: [
    HeadNone,
    UpperBodyRags,
    HandsNone,
    LowerBodyRags,
    FeetNone,
    WeaponNone,
    WeaponFist,
    WeaponSword,
  ],
  outcomes: [],
  rooms: [
    ChestRoom,
  ],
  screens: [],
};
