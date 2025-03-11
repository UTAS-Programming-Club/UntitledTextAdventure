package extensions;

import backend.Campaign;
import backend.Equipment;
import backend.Extension;
import backend.Screen;

enum Actions {
  GoNorth;
  GoEast;
  GoSouth;
  GoWest;
}

enum EquipmentNames {
  ChestRags;
  LegRags;
}

private final Equipment: Map<EquipmentNames, EquipmentInfo> = [
  ChestRags => {name: "Rags", type: Chest},
  LegRags => {name: "Rags", type: Legs}
];

enum Rooms {
  Empty;
}


enum ScreenNames {
  
}

private final Screens: Map<ScreenNames, Screen> = [
  
];

private final UntitledTextAdventureExtension: Extension = {
  actions: Actions,
  equipment: Equipment,
  rooms: Rooms,
  screens: Screens
};

final UntitledTextAdventure: Campaign = {
  floorSize: 5,
  extensions: [UntitledTextAdventureExtension]
};
