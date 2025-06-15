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

enum EquipmentSlot {
  Head;
  UpperBody;
  Hands;
  LowerBody;
  Feet;
  PrimaryWeapon;
  SecondaryWeapon;
}

class Equipment {
  public final type: EquipmentType;
  public final name: UnicodeString;

  public function new(type: EquipmentType, name: UnicodeString) {
    this.type = type;
    this.name = name;
  }

  public static function Get(type: EquipmentType, key: EnumValue): Equipment {
    final equipmentTypeMap: Null<Map<EnumValue, Equipment>> = GameInfo.Equipment[type];
    if (equipmentTypeMap == null) {
      throw 'Invalid equipment type: $type.';
    }

    final equipment: Null<Equipment> = equipmentTypeMap[key];
    if (equipment == null) {
      throw 'Invalid screen: $equipment.';
    }

    return equipment;
  }
}
