package backend.coregame;

import backend.Extension;
import backend.GameEnums;
import backend.Screen;
import backend.coregame.Actions;
import backend.coregame.ScreenInfo;

final CoreScreens2: Map<GameScreen, Screen> = [
  Test => new ActionScreen("This is a test", [
    new ScreenAction(QuitGame, "Quit Game")
  ]),
  Test2 => new ActionScreen("This is a test", [
    new ScreenAction(GotoScreen(Test), "Do a thing"),
    new ScreenAction(QuitGame, "Quit Game")
  ])
];


final CoreGameExt: Extension = {
  actions: [CoreGameAction],
  screenObjs: [CoreScreens, CoreScreens2]
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
