package extensions;

import backend.Extension;

enum ChestActions {
  OpenChest;
}

enum ChestRooms {
  Chest;
}

private final ChestsExtension: Extension = {
  actions: ChestActions,
  equipment: [],
  rooms: ChestRooms,
  screens: []
};
