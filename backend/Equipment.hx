package backend;

import backend.GameEnums;

enum EquipmentType {
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

  public static function Get(key: GameEquipment): Equipment {
    throw 'Not Implemented';
    /*final equipment: Null<Equipment> = GameInfo.Equipment[key];
    if (equipment == null) {
      throw 'Invalid screen $equipment.';
    }

    return equipment;*/
  }
}
