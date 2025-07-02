package backend;

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
}
