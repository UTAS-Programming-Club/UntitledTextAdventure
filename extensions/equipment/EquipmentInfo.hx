package extensions.equipment;

import backend.Equipment;
import backend.GameInfo;

// TODO: Avoid EnumValue and casts
@:nullSafety(Strict)
final EquipmentEquipment: Map<EquipmentType, Map<EnumValue, Equipment>> = [
  Head      => cast EquipmentHead,
  UpperBody => cast EquipmentUpperBody,
  Hands     => cast EquipmentHands,
  LowerBody => cast EquipmentLowerBody,
  Feet      => cast EquipmentFeet,
  Weapon    => cast EquipmentWeapon
];

@:nullSafety(Strict)
final EquipmentHead: Map<GameEquipmentHead, Equipment> = [
  HeadNone => new Equipment(Head, "Nothing")
];

@:nullSafety(Strict)
final EquipmentUpperBody: Map<GameEquipmentUpperBody, Equipment> = [
  UpperBodyRags => new Equipment(UpperBody, "Rags")
];

@:nullSafety(Strict)
final EquipmentHands: Map<GameEquipmentHands, Equipment> = [
  HandsNone => new Equipment(Hands, "Nothing")
];

@:nullSafety(Strict)
final EquipmentLowerBody: Map<GameEquipmentLowerBody, Equipment> = [
  LowerBodyRags => new Equipment(LowerBody, "Rags")
];

@:nullSafety(Strict)
final EquipmentFeet: Map<GameEquipmentFeet, Equipment> = [
  FeetNone => new Equipment(Feet, "Nothing")
];

@:nullSafety(Strict)
final EquipmentWeapon: Map<GameEquipmentWeapon, Equipment> = [
  WeaponFist => new Equipment(Weapon, "Fist"),
  WeaponNone => new Equipment(Weapon, "Nothing")
];
