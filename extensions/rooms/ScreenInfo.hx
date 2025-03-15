package extensions.rooms;

import backend.Game;
import backend.GameInfo;
import backend.Screen;
import extensions.rooms.Screens;

@:nullSafety(Strict)
function RoomTest(state: Game, screen: Screen): UnicodeString {
  final roomScreen = cast(screen, GameRoomScreen);
  final x: UInt = roomScreen.x + 1;
  final y: UInt = roomScreen.y + 1;
  return 'This is the game, you are in Room [$x, $y].';
}

@:nullSafety(Strict)
final RoomScreens: Map<GameScreen, Screen> = [
  GameRooms => new GameRoomScreen(RoomTest, [
    new ScreenAction(GotoScreen(MainMenu), "Return to main menu")
  ], 0, 0),
];
