package game;

import game.Equipment;
import game.Extensions;

enum Actions {
  GoNorth;
  GoEast;
  GoSouth;
  GoWest;
}

enum Equipment {
  ChestRags;
  LegRags;
}

private final Equipment: Map<Equipment, EquipmentInfo> = [
  ChestRags => {name: "Rags", type: Chest},
  LegRags => {name: "Rags", type: Legs}
];

enum Rooms {
  Empty;
}

private final Game: Extension = {
  actions: Actions,
  equipment: Equipment,
  rooms: Rooms
};
