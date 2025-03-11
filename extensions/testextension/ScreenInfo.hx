package extensions.testextension;

import backend.GameInfo;
import backend.Screen;

final TestExtensionScreenInfo: Map<GameScreens, Screen> = [
  GameRooms => new Screen("This is the game", [
    new ScreenAction(GotoScreen(MainMenu), "Return to main menu")
  ]),
  LoadGame => new Screen("This is the load game screen", [
    new ScreenAction(GotoScreen(GameRooms), "Return to main menu")
  ])
];
