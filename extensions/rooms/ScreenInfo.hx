package extensions.rooms;

import backend.Game;
import backend.GameInfo;
import backend.Screen;
import extensions.rooms.Screens;
// TODO: Fix rooms extension depending on traps extension
import extensions.trap.Rooms.TrapRoom;

@:nullSafety(Strict)
function RoomTest(state: Game, screen: Screen): UnicodeString {
  final roomScreen = cast(screen, GameRoomScreen);
  final x: UInt = roomScreen.x;
  final y: UInt = roomScreen.y;
  final room: GameRoom = state.campaign.rooms[x][y];

  var body: UnicodeString = 'This is the game, you are in Room [${x + 1}, ${y + 1}].';

  // TODO: Fix rooms extension depending on traps extension
  if (room == Trap) {
    final gameScreen: GameRoomScreen = cast screen;
    final roomState: TrapRoom = gameScreen.getRoomState(state, x, y, TrapRoom.new);
    body += '\n\nThis is a trap room which has';

    if (!roomState.activatedTrap) {
      body += ' not';
    }

    body += ' been triggered.';
  }

  return body;

}

@:nullSafety(Strict)
final RoomScreens: Map<GameScreen, Screen> = [
  GameRooms => new GameRoomScreen(RoomTest, [
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
    new ScreenAction(GoWest, "Go West", function (state: Game, screen: Screen): Bool {
      final roomScreen = cast(screen, GameRoomScreen);
      return roomScreen.x < state.campaign.rooms.length - 1 &&
             state.campaign.rooms[roomScreen.x + 1][roomScreen.y] != Unused;
    }),
    // TODO: Fix rooms extension depending on traps extension
    new ScreenAction(DodgeTrap, "Dodge Trap", function (state: Game, screen: Screen): Bool {
      final roomScreen = cast(screen, GameRoomScreen);
      final x: UInt = roomScreen.x;
      final y: UInt = roomScreen.y;
      final room: GameRoom = state.campaign.rooms[x][y];

      if (room != Trap) {
        return false;
      }

      final gameScreen: GameRoomScreen = cast screen;
      final roomState: TrapRoom = gameScreen.getRoomState(state, x, y, TrapRoom.new);

      return !roomState.activatedTrap;
    }),
#if testrooms
    new ScreenAction(QuitGame, "Quit Game")
#else
    new ScreenAction(GotoScreen(MainMenu), "Return to main menu")
#end
  ], 0, 0),
];
