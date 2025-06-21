package extensions.equipment;

import backend.Equipment;
import backend.GameEnums;

@:nullSafety(Strict)
final EquipmentEquipment: Map<GameEquipment, Equipment> = [
  HeadNone      => new Equipment(Head, "Nothing"),
  UpperBodyRags => new Equipment(UpperBody, "Rags"),
  HandsNone     => new Equipment(Hands, "Nothing"),
  LowerBodyRags => new Equipment(LowerBody, "Rags"),
  FeetNone      => new Equipment(Feet, "Nothing"),
  PrimaryWeaponFist   => new Equipment(PrimaryWeapon, "Fist"),
  SecondaryWeaponNone => new Equipment(SecondaryWeapon, "Nothing")
];
