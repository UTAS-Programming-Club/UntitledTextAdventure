package backend.coregame;

import backend.Extension;
import backend.Game;
import backend.GameInfo;

final CoreGameExt: Extension = {
  actionHandler: function(state: Game, action: GameAction): GameOutcome {
    switch (action) {
      case StartGame:
        state.currentScreen = state.campaign.gameScreen;
        return GetNextOutput;
      case GotoScreen(screen):
        state.currentScreen = screen;
        return GetNextOutput;
      case QuitGame:
        return QuitGame;
      default:
        return Invalid;
    }
  }
};
