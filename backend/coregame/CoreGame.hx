package backend.coregame;

import backend.Extension;
import backend.Game;
import backend.GameInfo;

final CoreGameExt: Extension = {
  actionHandler: function(state: Game, action: GameAction): GameOutcome {
    switch (action) {
      case StartGame:
        state.gotoScreen(state.campaign.gameScreen);
        state.player.Reset(state.campaign);
        // TODO: Move room x, y to player class?
        // TODO: Reset state.screenState
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
  }
};
