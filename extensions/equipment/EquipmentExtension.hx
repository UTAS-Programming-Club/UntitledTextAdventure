package extensions.equipment;

import backend.Extension;
import extensions.equipment.Equipment;
import extensions.equipment.Rooms;

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
    PrimaryWeaponFist,
    SecondaryWeaponNone,
  ],
  outcomes: [],
  rooms: [
    ChestRoom,
  ],
  screens: [],
};
