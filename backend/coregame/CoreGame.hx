package backend.coregame;

// import backend.BaseGame;
import backend.Extension;
import backend.GameEnums;
import backend.coregame.Actions;
import backend.coregame.Outcomes;
import backend.coregame.ScreenInfo;
import backend.coregame.Screens;

final CoreGameExt: Extension = {
  actions: [CoreGameAction],
  // equipment: [],
  outcomes: [CoreOutcome],
  rooms: [],
  screens: [CoreScreen],
  // equipmentObjs: [],
  // roomObjs: [],
  screenObjs: [CoreScreens],
  actionHandler: function(state: BaseGame, action: GameAction): GameOutcome {
    switch (action) {
      /*case GameAction.StartGame:
        state.startGame();
        return GetNextOutput;
      case GameAction.GotoScreen(screen):
        state.gotoScreen(screen);
        return GetNextOutput;
      case GameAction.GotoPreviousScreen:
        state.gotoScreen(state.previousScreen);
        return GetNextOutput;
      case GameAction.QuitGame:
        return QuitGame;*/
      default:
        return GameOutcome.Invalid;
    }
  }
};
