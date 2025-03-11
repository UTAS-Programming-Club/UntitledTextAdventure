package backend;

import backend.Equipment;

typedef Extension = {
  final actions: Enum<Any>;
  final equipment: Map<Any, EquipmentInfo>;
  final rooms: Enum<Any>;
  final screens: Map<Any, Screen>;
}
