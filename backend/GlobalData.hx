package backend;

import backend.Entity;
import backend.Room;
import backend.Screen;
import haxe.ds.Vector;

@:nullSafety(Strict)
class GlobalData {
  // TODO: Support multiple floors
  // TODO: Support removing doors between adjacent rooms
  public static final floorSize: UInt = 5;
  public static final rooms = new Vector<Vector<Null<Room>>>(floorSize);

  public static final mainMenuScreen = new ActionScreen(
    function (state: GameState) {
      state.inGame = false;

      return "Untitled text adventure game\n"
           + "----------------------------\n"
           + "By the UTAS Programming Club\n\n"
           + "Currently unimplemented :(";
      }
  );

  static final gameScreen = new ActionScreen(
    function (state: GameState) {
      if (!state.inGame) {
        state.SetupGame();
      }

      var body: UnicodeString =
        "This is the game, you are in room ["
        + (state.player.X + 1) + ", " + (state.player.Y + 1)
        + "].";

      final room = rooms[state.player.Y][state.player.X];
      switch (room) {
        case Empty:
          // Ignore
        case Trap(_, _):
          if (state.roomState[state.player.Y][state.player.X]) {
            body += "\n\nTriggered";
          } else {
            body += "\n\nNot triggered";
          }
        case null:
          throw new haxe.Exception("Unknown room " + room + " recevied");
        default:
          throw new haxe.Exception("Unknown room " + room + " recevied");
      }

      return body;
    },
    [
      new ScreenAction(
        "Go North",
        ScreenActionType.GoNorth,
        function (state: GameState, _, _) {
          if (state.player.Y == floorSize - 1) {
            return false;
          }
          final room: Null<Room> = rooms[state.player.Y + 1][state.player.X];
          return room != null;
        }
      ),
      new ScreenAction(
        "Go East",
        ScreenActionType.GoEast,
        function (state: GameState, _, _) {
          if (state.player.X == floorSize - 1) {
            return false;
          }
          final room: Null<Room> = rooms[state.player.Y][state.player.X + 1];
          return room != null;
        }
      ),
      new ScreenAction(
        "Go South",
        ScreenActionType.GoSouth,
        function (state: GameState, _, _) {
          if (state.player.Y == 0) {
            return false;
          }
          final room: Null<Room> = rooms[state.player.Y - 1][state.player.X];
          return room != null;
        }
      ),
      new ScreenAction(
        "Go West",
        ScreenActionType.GoWest,
        function (state: GameState, _, _) {
          if (state.player.X == 0) {
            return false;
          }
          final room: Null<Room> = rooms[state.player.Y][state.player.X - 1];
          return room != null;
        }
      ),
      new ScreenAction(
        "Attempt to dodge trap",
        ScreenActionType.DodgeTrap,
        function (state: GameState, room: Room, roomState: Bool) {
          return room.match(Trap(_, _)) && !roomState;
        }
      ),
      new ScreenAction(
        "Quit",
        ScreenActionType.GotoScreen(mainMenuScreen)
      )
    ]
  );

  static final loadScreen = new ActionScreen(
    "Game loading is not currently supported",
    [
      new ScreenAction(
        "Quit",
        ScreenActionType.GotoScreen(mainMenuScreen)
      )
    ]
  );

  public static final enemyStats: Map<UnicodeString, EntityStats> = [
    "Demon" => new EntityStats(100, 100, 10, 10, 10, 10, 10)
  ];

  public static function Init(): Void {
    mainMenuScreen.Init([
      new ScreenAction(
        "Start Game",
        ScreenActionType.GotoScreen(gameScreen)
      ),
      new ScreenAction(
        "Load Game",
        ScreenActionType.GotoScreen(loadScreen)
      ),
      new ScreenAction(
        "Quit Game",
        ScreenActionType.QuitGame
      )
    ]);

    for (i in 0...rooms.length) {
      rooms[i] = new haxe.ds.Vector(floorSize);
    }

    rooms[0][0] = Room.Empty;
    rooms[1][0] = Room.Empty;
    rooms[0][1] = Room.Empty;
    rooms[1][1] = Room.Empty;
    rooms[2][1] = Room.Trap(20, 40, 10);
  }
}
