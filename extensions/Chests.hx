package extensions;

import game.Extensions;

enum ChestActions {
  OpenChest;
}

enum ChestRooms {
  Chest;
}

private final ChestsExtension: Extension = {
  actions: ChestActions,
  equipment: [],
  rooms: ChestRooms
};
