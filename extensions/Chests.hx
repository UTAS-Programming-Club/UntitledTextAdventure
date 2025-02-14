package extensions;

enum ChestRooms {
  Chest1(test: Int);
  Chest2;
}

class ChestsExtension {
  static final rooms = ChestRooms;
}
