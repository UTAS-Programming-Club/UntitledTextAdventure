package extensions.equipment;

import backend.Extension;
import extensions.equipment.Equipment;

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
  screens: [],
};
