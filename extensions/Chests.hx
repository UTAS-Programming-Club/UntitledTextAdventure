package extensions;

import game.Extensions;

enum ChestRooms {
  Chest1(test: Int);
  Chest2;
}

enum ChestActions {
  OpenChest(test: String);
}

private final ChestsExtension = new Extension(ChestRooms, ChestActions);
