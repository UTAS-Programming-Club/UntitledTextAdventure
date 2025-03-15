package extensions.test;

import backend.Game;
import backend.GameInfo;
import backend.Screen;

final TestScreens: Map<GameScreen, Screen> = [
  Test => new ActionScreen("Test game screen", [
    new ScreenAction(GotoScreen(MainMenu), "Return to main menu", function (state: Game): GameOutcome {
      state.currentScreen = MainMenu;
      return GetNextOutput;
    })
  ]),
];
