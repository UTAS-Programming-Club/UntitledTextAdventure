package backend;

enum EquipmentType {
  Head;
  Chest; // Covers upper arms as well
  Hands; // Covers Lower arms as well
  Legs;
  Feet;
  SingleHandedWeapon;
  DoubleHandedWeapon;
}

typedef EquipmentInfo = {name: String, type: EquipmentType};
