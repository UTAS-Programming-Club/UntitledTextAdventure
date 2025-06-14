package backend;

import backend.GameInfo;
using Type;

enum EquipmentType {
  Head;
  UpperBody;
  Hands;
  LowerBody;
  Feet;
  Weapon;
}

class Equipment {
  public final type: EquipmentType;
  public final name: UnicodeString;

  public function new(type: EquipmentType, name: UnicodeString) {
    this.type = type;
    this.name = name;
  }

  public static function Get(type: EquipmentType, key: EnumValue): Equipment {
    var equipment: Null<Equipment> = null;

    final keyType: Enum<Dynamic> = key.getEnum();
    switch (keyType) {
      case GameEquipmentHead:
        final idx: Int = key.enumIndex();
        final equipmentKey: GameEquipmentHead = GameEquipmentHead.createEnumIndex(idx, null);
        equipment = GameInfo.EquipmentHead[equipmentKey];
      case GameEquipmentUpperBody:
        final idx: Int = key.enumIndex();
        final equipmentKey: GameEquipmentUpperBody = GameEquipmentUpperBody.createEnumIndex(idx, null);
        equipment = GameInfo.EquipmentUpperBody[equipmentKey];
      case GameEquipmentHands:
        final idx: Int = key.enumIndex();
        final equipmentKey: GameEquipmentHands = GameEquipmentHands.createEnumIndex(idx, null);
        equipment = GameInfo.EquipmentHands[equipmentKey];
      case GameEquipmentLowerBody:
        final idx: Int = key.enumIndex();
        final equipmentKey: GameEquipmentLowerBody = GameEquipmentLowerBody.createEnumIndex(idx, null);
        equipment = GameInfo.EquipmentLowerBody[equipmentKey];
      case GameEquipmentFeet:
        final idx: Int = key.enumIndex();
        final equipmentKey: GameEquipmentFeet = GameEquipmentFeet.createEnumIndex(idx, null);
        equipment = GameInfo.EquipmentFeet[equipmentKey];
      case GameEquipmentWeapon:
        final idx: Int = key.enumIndex();
        final equipmentKey: GameEquipmentWeapon = GameEquipmentWeapon.createEnumIndex(idx, null);
        equipment = GameInfo.EquipmentWeapon[equipmentKey];
      default:
        throw 'Unexpected equipment type "e"';
    }

    if (equipment == null) {
      throw 'Invalid screen $equipment.';
    }

    return equipment;
  }
}
