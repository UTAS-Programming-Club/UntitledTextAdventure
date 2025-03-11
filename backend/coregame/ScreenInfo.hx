package backend.coregame;

import backend.GameInfo;
import backend.Screen;

final CoreScreenInfo: Map<GameScreens, Screen> = [
  MainMenu => new Screen("This is the main menu", [
    new ScreenAction(GotoScreen(GameRooms), "Start Game"),
    new ScreenAction(LoadGame, "Load Game"),
    new ScreenAction(QuitGame, "Quit Game")
  ])
];
