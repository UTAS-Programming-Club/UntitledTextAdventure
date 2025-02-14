package extensions;

import game.Extensions;

enum ChestRooms {
  Chest1(test: Int);
  Chest2;
}

class ChestsExtension implements Extension {
  final rooms = ChestRooms;
}
