package backend.coregame;

import backend.Extension;
import backend.coregame.Actions;
import backend.coregame.Outcomes;
import backend.coregame.Rooms;
import backend.coregame.Screens;

final CoreGameExt: Extension = {
  module: 'backend.coregame',
  actions: [
    StartGame,
    GotoScreen,
    GotoPreviousScreen,
    Quit,
    GoNorth,
    GoEast,
    GoSouth,
    GoWest,
    CycleEquipment,
  ],
  equipment: [],
  outcomes: [
    Invalid,
    GetNextOutput,
    QuitGame,
  ],
  rooms: [
    UnusedRoom,
    EmptyRoom,
  ],
  screens: [
    MainMenuScreen,
    SaveScreen,
    LoadScreen,
    PlayerEquipmentScreen,
  ],
};
