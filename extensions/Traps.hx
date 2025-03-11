package extensions;

import backend.Extension;

enum TrapActions {
  DodgeTrap;
}

enum TrapRooms {
  Trap(lowerAgility: UInt, upperAgility: UInt, maxDamage: UInt);
}

private final TrapsExtension: Extension = {
  actions: TrapActions,
  equipment: [],
  rooms: TrapRooms,
  screens: []
};
