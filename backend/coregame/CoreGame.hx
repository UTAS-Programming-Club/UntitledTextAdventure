package backend.coregame;

import backend.Extension;
import backend.coregame.Actions;
import backend.coregame.Outcomes;
import backend.coregame.Screens;

final CoreGameExt: Extension = {
  module: 'backend.coregame',
  actions: [
    StartGame,
    GotoScreen,
    GotoPreviousScreen,
    Quit,
  ],
  outcomes: [
    Invalid,
    GetNextOutput,
    QuitGame,
  ],
  screens: [
    MainMenuScreen,
    LoadScreen,
    PlayerEquipmentScreen,
  ],
};
