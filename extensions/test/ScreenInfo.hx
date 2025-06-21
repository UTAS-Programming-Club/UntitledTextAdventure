package extensions.test;

import backend.GameEnums;
import backend.Screen;

@:nullSafety(Strict)
final TestScreens: Map<GameScreen, Screen> = [
  Test => new ActionScreen("Test game screen", [
    new ScreenAction(GotoScreen(MainMenu), "Return to main menu")
  ]),
];
