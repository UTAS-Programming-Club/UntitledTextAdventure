package backend;

import backend.Equipment;
import backend.GameInfo;

class Player {
  public var health(default, null): Int = 0;
  public var stamina(default, null): Int = 0;
  public var chest(default, null): Equipment;

  public function new(initialChest: GameEquipment) {
    chest = Equipment.Get(initialChest);
  }

  public function Reset(initialChest: GameEquipment): Void {
    health = 100;
    stamina = 100;
    chest = Equipment.Get(initialChest);
  }

  public function ModifyHealth(change: Int): Void {
     var newHealth: Int = health + change;
     if (newHealth < 0) {
      newHealth = 0;
     } else if (newHealth > 100) {
      newHealth = 100;
     }

     health = newHealth;
  }
}
