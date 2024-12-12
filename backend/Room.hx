package backend;

import backend.Player;

enum Room {
  Empty;
  Trap(lowerAgility: UInt, upperAgility: UInt, damage: UInt);
  // HealthChange(diff: Int);
  // CustomChest(...);
  // Combat(enemies: Array<Enemy>);
}

function IsRoomStateful(room: Room): Bool {
  return switch(room) {
    case Empty:
      false;
    case Trap(_, _):
      true;
    default:
      throw new haxe.Exception("Invalid room " + room + " detected");
  }
}
