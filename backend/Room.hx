package backend;

import backend.Player;

enum Room {
  Empty;
  Trap(lowerAgility: UInt, upperAgility: UInt, damage: UInt);
  // HealthChange(diff: Int);
  // CustomChest(...);
  // Combat(enemies: Array<Enemy>);
}
