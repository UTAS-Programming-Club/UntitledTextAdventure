package extensions;

import game.Extensions;

enum TrapRooms {
  Trap;
  Trap2(val1: Int, val2: String);
  Test3;
}

// TODO: Use or remove
// final TrapsExtension = new Extension(TrapRooms);

class TrapsExtension implements Extension {
  final rooms = TrapRooms;
}
