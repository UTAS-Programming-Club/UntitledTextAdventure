package backend.coregame;

import backend.Extension;
import backend.Game;
import backend.GameInfo;

final CoreGameExt: Extension = {
  actionHandler: function(state: Game, action: GameAction): GameOutcome {
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
  },
  textHandler: function(state: Game, str: UnicodeString): GameOutcome {
    switch (state.currentScreen) {
      case Load:
        state.loadGame(str);
        return GetNextOutput;
      default:
        return Invalid;
    }
  }
};
