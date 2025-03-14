package extensions.rooms;

import backend.Game;
import backend.GameInfo;
import backend.Screen;

final RoomScreenInfo: Map<GameScreen, Screen> = [
  GameRooms => new ActionScreen("This is the game", [
    new ScreenAction(GotoScreen(MainMenu), "Return to main menu")
  ]),
];
