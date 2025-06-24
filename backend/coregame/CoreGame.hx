package backend.coregame;

import backend.Extension;
// import backend.Game;
// import backend.GameInfo;
import backend.coregame.Screens;

final CoreGameExt: Extension = {
  screens: [
    {type: MainMenu, constructor: MainMenu.new}
  ],
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
