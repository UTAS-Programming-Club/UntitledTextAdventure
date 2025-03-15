package extensions.rooms;

import backend.Game;
import backend.GameInfo;
import backend.Screen;

function RoomTest(state: Game): UnicodeString {
  return "This is the game";
}

final RoomScreens: Map<GameScreen, Screen> = [
  GameRooms => new ActionScreen(RoomTest, [
    new ScreenAction(GotoScreen(MainMenu), "Return to main menu")
  ]),
];
