package backend.coregame;

import backend.Extension;
import backend.coregame.Actions;
import backend.coregame.Outcomes;
import backend.coregame.Screens;

final CoreGameExt: Extension = {
  actions: [
    StartGame,
    GotoScreen,
    GotoPreviousScreen,
    Quit,
  ],
  outcomes: [
    {type: Invalid, constructor: Invalid.new},
    {type: GetNextOutput, constructor: GetNextOutput.new},
    {type: QuitGame, constructor: QuitGame.new},
  ],
  screens: [
    {type: MainMenu, constructor: MainMenu.new},
    {type: Load, constructor: Load.new},
    {type: PlayerEquipment, constructor: PlayerEquipment.new},
  ],
};
