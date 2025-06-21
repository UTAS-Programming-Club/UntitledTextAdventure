package backend.coregame;

import backend.Extension;
import backend.GameEnums;
import backend.coregame.Actions;
import backend.coregame.ScreenInfo;

final CoreGameExt: Extension = {
  actions: [CoreGameAction],
  equipmentObjs: [],
  roomObjs: [],
  screenObjs: [CoreScreens]
  /*actionHandler: function(state: Game, action: GameAction): GameOutcome {
    switch (action) {
      case StartGame:
        state.startGame();
        return GetNextOutput;
      case GotoScreen(screen):
        state.gotoScreen(screen);
        return GetNextOutput;
      case GotoPreviousScreen:
        state.gotoScreen(state.previousScreen);
        return GetNextOutput;
      case QuitGame:
        return QuitGame;
      default:
        return Invalid;
    }
  }*/
};
