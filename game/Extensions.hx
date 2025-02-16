package game;

import game.Equipment;

class Extension {
  private final actions: Enum<Any>;
  private final equipment: Array<EquipmentInfo>;
  private final rooms: Enum<Any>;

  public function new(actions: Enum<Any>, equipment: Array<EquipmentInfo>, rooms: Enum<Any>) {
    this.actions = actions;
    this.equipment = equipment;
    this.rooms = rooms;
  }
}
