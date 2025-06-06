package extensions.rooms;

import backend.Game;
import backend.GameInfo;
import backend.Screen;
import extensions.rooms.Screens;
// TODO: Fix rooms extension depending on traps extension
import extensions.trap.Rooms.TrapRoom;

@:nullSafety(Strict)
function RoomTest(state: Game, screen: Screen): UnicodeString {
  final roomScreenState: GameRoomState = state.getScreenState();
  final x: Int = roomScreenState.x;
  final y: Int = roomScreenState.y;
  final room: GameRoom = state.campaign.rooms[x][y];

  var body: UnicodeString = 'This is the game, you are in Room [${x + 1}, ${y + 1}].';

  // TODO: Fix rooms extension depending on traps extension
  if (room == Trap) {
    final roomState: TrapRoom = roomScreenState.getRoomState(state);
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
  GameRooms => new StatefulActionScreen(GameRoomState.new, RoomTest, [
    new ScreenAction(GoNorth, "Go North", function (state: Game, screen: ActionScreen): Bool {
      final roomScreenState: GameRoomState = state.getScreenState();
      return roomScreenState.y < state.campaign.rooms.length - 1 &&
             state.campaign.rooms[roomScreenState.x][roomScreenState.y + 1] != Unused;
    }),
    new ScreenAction(GoEast, "Go East", function (state: Game, screen: ActionScreen): Bool {
      final roomScreenState: GameRoomState = state.getScreenState();
      return roomScreenState.x > 0 &&
             state.campaign.rooms[roomScreenState.x - 1][roomScreenState.y] != Unused;
    }),
    new ScreenAction(GoSouth, "Go South", function (state: Game, screen: ActionScreen): Bool {
      final roomScreenState: GameRoomState = state.getScreenState();
      return roomScreenState.y > 0 &&
             state.campaign.rooms[roomScreenState.x][roomScreenState.y - 1] != Unused;
    }),
    new ScreenAction(GoWest, "Go West", function (state: Game, screen: ActionScreen): Bool {
      final roomScreenState: GameRoomState = state.getScreenState();
      return roomScreenState.x < state.campaign.rooms.length - 1 &&
             state.campaign.rooms[roomScreenState.x + 1][roomScreenState.y] != Unused;
    }),
    // TODO: Fix rooms extension depending on traps extension
    new ScreenAction(DodgeTrap, "Dodge Trap", function (state: Game, screen: ActionScreen): Bool {
      final roomScreenState: GameRoomState = state.getScreenState();
      final room: GameRoom = state.campaign.rooms[roomScreenState.x][roomScreenState.y];

      if (room != Trap) {
        return false;
      }

      final roomState: TrapRoom = roomScreenState.getRoomState(state);

      return !roomState.activatedTrap;
    }),
    new ScreenAction(GotoScreen(PlayerEquipment), "Check Inventory"),
#if testrooms
    new ScreenAction(QuitGame, "Quit Game")
#else
    new ScreenAction(GotoScreen(MainMenu), "Return to main menu")
#end
  ])
];
