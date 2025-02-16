package game;

import game.Equipment;
import game.Extensions;

enum Actions {
  GoNorth;
  GoEast;
  GoSouth;
  GoWest;
}

final ChestRags: EquipmentInfo = {name: "Rags", type: Chest};
final LegRags: EquipmentInfo = {name: "Rags", type: Legs};
final Equipment: Array<EquipmentInfo> = [
  ChestRags,
  LegRags
];

enum Rooms {
  Empty;
}

private final Game = new Extension(Actions, Equipment, Rooms);
