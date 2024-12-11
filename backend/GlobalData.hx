package backend;

import backend.Entity;
import backend.Room;
import backend.Screen;
import haxe.ds.Vector;

class GlobalData {
  public static final floorSize: Int = 5;
  public static final rooms = new Vector<Vector<Null<Room>>>(floorSize);

  // TODO: Remove, visit count is only used for testing
  private static var mainMenuVisitCount: Int = 0;
  public static final mainMenuScreen = new ActionScreen(
    function (state: GameState) {
      var body: UnicodeString =
          "Untitled text adventure game\n"
        + "----------------------------\n"
        + "By the UTAS Programming Club\n\n"
        + "Currently unimplemented :(";

      if (mainMenuVisitCount > 0) {
        body += "\n\nReload count: " + mainMenuVisitCount;
      }
      mainMenuVisitCount++;

      return body;
    }
  );

  static final gameScreen = new ActionScreen(
    function (state: GameState) {
      return "This is the game, you are in room ["
           + (state.player.X + 1) + ", " + (state.player.Y + 1)
           + "].";
    },
    [
      new ScreenAction(
        "Go North",
        ScreenActionType.GoNorth,
        function (state: GameState) {
          if (state.player.Y == floorSize - 1) {
            return false;
          }
          final room: Room = rooms[state.player.Y + 1][state.player.X];
          return room != null;
        }
      ),
      new ScreenAction(
        "Go East",
        ScreenActionType.GoEast,
        function (state: GameState) {
          if (state.player.X == floorSize - 1) {
            return false;
          }
          final room: Room = rooms[state.player.Y][state.player.X + 1];
          return room != null;
        }
      ),
      new ScreenAction(
        "Go South",
        ScreenActionType.GoSouth,
        function (state: GameState) {
          if (state.player.Y == 0) {
            return false;
          }
          final room: Room = rooms[state.player.Y - 1][state.player.X];
          return room != null;
        }
      ),
      new ScreenAction(
        "Go West",
        ScreenActionType.GoWest,
        function (state: GameState) {
          if (state.player.X == 0) {
            return false;
          }
          final room: Room = rooms[state.player.Y][state.player.X - 1];
          return room != null;
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
  }
}
