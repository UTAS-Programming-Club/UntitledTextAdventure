package backend;

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

// TODO: Support two handed weapons
// TODO: Support only one slot at a time weapons?
class Equipment {
  public final type: EquipmentType;
  public final name: UnicodeString;

  public function new(type: EquipmentType, name: UnicodeString) {
    this.type = type;
    this.name = name;
  }

  public static function getType(slot: EquipmentSlot): EquipmentType return switch(slot) {
    case Head:
      Head;
    case UpperBody:
      UpperBody;
    case Hands:
      Hands;
    case LowerBody:
      LowerBody;
    case Feet:
      Feet;
    case PrimaryWeapon:
      Weapon;
    case SecondaryWeapon:
      Weapon;
  }
}
