package extensions;

import game.Extensions;

enum TrapActions {
  DodgeTrap;
}

enum TrapRooms {
  Trap(lowerAgility: UInt, upperAgility: UInt, maxDamage: UInt);
}

private final TrapsExtension = new Extension(TrapActions, [], TrapRooms);
