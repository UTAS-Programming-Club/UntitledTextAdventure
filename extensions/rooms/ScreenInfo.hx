package extensions.rooms;

import backend.Game;
import backend.GameInfo;
import backend.Screen;
import extensions.rooms.Screens;

@:nullSafety(Strict)
function RoomTest(state: Game, screen: Screen): UnicodeString {
  final roomScreen = cast(screen, GameRoomScreen);
  final x: UInt = roomScreen.x;
  final y: UInt = roomScreen.y;
  final room: GameRoom = state.campaign.rooms[x][y];
  return 'This is the game, you are in Room [${x + 1}, ${y + 1}]: $room.';
}

@:nullSafety(Strict)
final RoomScreens: Map<GameScreen, Screen> = [
  GameRooms => new GameRoomScreen(RoomTest, [
    // TODO: Prevent visiting Unused rooms
    new ScreenAction(GoNorth, "Go North", function (state: Game, screen: Screen): Bool {
      final roomScreen = cast(screen, GameRoomScreen);
      return roomScreen.y < state.campaign.rooms.length - 1 &&
             state.campaign.rooms[roomScreen.x][roomScreen.y + 1] != Unused;
    }),
    new ScreenAction(GoEast, "Go East", function (state: Game, screen: Screen): Bool {
      final roomScreen = cast(screen, GameRoomScreen);
      return roomScreen.x > 0 && state.campaign.rooms[roomScreen.x - 1][roomScreen.y] != Unused;
    }),
    new ScreenAction(GoSouth, "Go South", function (state: Game, screen: Screen): Bool {
      final roomScreen = cast(screen, GameRoomScreen);
      return roomScreen.y > 0 && state.campaign.rooms[roomScreen.x][roomScreen.y - 1] != Unused;
    }),
    // TODO: Prevent visiting Unused rooms
    new ScreenAction(GoWest, "Go West", function (state: Game, screen: Screen): Bool {
      final roomScreen = cast(screen, GameRoomScreen);
      return roomScreen.x < state.campaign.rooms.length - 1 &&
             state.campaign.rooms[roomScreen.x + 1][roomScreen.y] != Unused;
    }),
#if testrooms
    new ScreenAction(QuitGame, "Quit Game")
#else
    new ScreenAction(GotoScreen(MainMenu), "Return to main menu")
#end
  ], 0, 0),
];
