package backend.coregame;

import backend.GameInfo;
import backend.Screen;

final CoreScreenInfo: Map<GameScreen, Screen> = [
  MainMenu => new ActionScreen("This is the main menu", [
    new ScreenAction(StartGame, "Start Game"),
    new ScreenAction(LoadGame, "Load Game"),
    new ScreenAction(QuitGame, "Quit Game")
  ])
];
