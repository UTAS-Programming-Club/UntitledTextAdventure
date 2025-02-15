package extensions;

import game.Extensions;

enum TrapRooms {
  Trap;
  Trap2(val1: Int, val2: String);
  Test3;
}

enum TrapActions {
}

private final TrapsExtension = new Extension(TrapRooms, TrapActions);
