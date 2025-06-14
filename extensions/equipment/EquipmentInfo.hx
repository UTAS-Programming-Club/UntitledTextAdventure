package extensions.equipment;

import backend.Equipment;
import backend.GameInfo;

@:nullSafety(Strict)
final EquipmentEquipmentHead: Map<GameEquipmentHead, Equipment> = [
  HeadNone => new Equipment(Head, "Nothing")
];

@:nullSafety(Strict)
final EquipmentEquipmentUpperBody: Map<GameEquipmentUpperBody, Equipment> = [
  UpperBodyRags => new Equipment(UpperBody, "Rags")
];

@:nullSafety(Strict)
final EquipmentEquipmentHands: Map<GameEquipmentHands, Equipment> = [
  HandsNone => new Equipment(Hands, "Nothing")
];

@:nullSafety(Strict)
final EquipmentEquipmentLowerBody: Map<GameEquipmentLowerBody, Equipment> = [
  LowerBodyRags => new Equipment(LowerBody, "Rags")
];

@:nullSafety(Strict)
final EquipmentEquipmentFeet: Map<GameEquipmentFeet, Equipment> = [
  FeetNone => new Equipment(Feet, "Nothing")
];

@:nullSafety(Strict)
final EquipmentEquipmentWeapon: Map<GameEquipmentWeapon, Equipment> = [
  WeaponFist => new Equipment(Weapon, "Fist"),
  WeaponNone => new Equipment(Weapon, "Nothing")
];
