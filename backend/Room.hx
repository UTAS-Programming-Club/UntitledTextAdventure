package backend;

@:nullSafety(Strict)
enum Room {
  Empty;
  Trap(lowerAgility: UInt, upperAgility: UInt, damage: UInt);
  // CustomChest(...);
  // Combat(enemies: Array<Enemy>);
}

@:nullSafety(Strict)
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
