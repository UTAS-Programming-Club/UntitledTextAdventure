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
  // TODO: Fix
  // See comment above GameState.roomState for info about null issue
  // Also causes "Null safety: Cannot unify Array<Null<backend.Room>> with haxe.extern.Rest<Dynamic>"
  // with Null<Room> but not Room if a default value is provided like in the roomState case
  // Frontends must call init before any using any functions that access rooms
  @:nullSafety(Off)
  public static final rooms = new Vector<Vector<Null<Room>>>(floorSize, new Vector(0));

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

      final room: Null<Room> = GetRoom(state.player.X, state.player.Y);
      final roomBody: String = room.GetBody(state);

      var body: UnicodeString =
        "This is the game, you are in room ["
        + (state.player.X + 1) + ", " + (state.player.Y + 1)
        + "].";
      if (roomBody != "") {
        body += "\n\n" + roomBody;
      }

      return body;
    },
    [
      new ScreenAction(
        "Go North",
        ScreenActionType.GoNorth,
        function (state: GameState, room: Room, _) {
          if (state.player.Y == floorSize - 1) {
            return false;
          }

          final nextRoom: Null<Room> = rooms[state.player.Y + 1][state.player.X];
          if (nextRoom == null) {
            return false;
          }

          final allowedDirections: Array<RoomDirection> = room.GetAllowedDirections(state);
          return allowedDirections.contains(RoomDirection.North);
        }
      ),
      new ScreenAction(
        "Go East",
        ScreenActionType.GoEast,
        function (state: GameState, room: Room, _) {
          if (state.player.X == floorSize - 1) {
            return false;
          }

          final nextRoom: Null<Room> = rooms[state.player.Y][state.player.X + 1];
          if (nextRoom == null) {
            return false;
          }

          final allowedDirections: Array<RoomDirection> = room.GetAllowedDirections(state);
          return allowedDirections.contains(RoomDirection.East);
        }
      ),
      new ScreenAction(
        "Go South",
        ScreenActionType.GoSouth,
        function (state: GameState, room: Room, _) {
          if (state.player.Y == 0) {
            return false;
          }

          final nextRoom: Null<Room> = rooms[state.player.Y - 1][state.player.X];
          if (nextRoom == null) {
            return false;
          }

          final allowedDirections: Array<RoomDirection> = room.GetAllowedDirections(state);
          return allowedDirections.contains(RoomDirection.South);
        }
      ),
      new ScreenAction(
        "Go West",
        ScreenActionType.GoWest,
        function (state: GameState, room: Room, _) {
          if (state.player.X == 0) {
            return false;
          }

          final nextRoom: Null<Room> = rooms[state.player.Y][state.player.X - 1];
          if (nextRoom == null) {
            return false;
          }

          final allowedDirections: Array<RoomDirection> = room.GetAllowedDirections(state);
          return allowedDirections.contains(RoomDirection.North);
        }
      ),
      new ScreenAction(
        "Attempt to dodge trap",
        ScreenActionType.DodgeTrap,
        function (state: GameState, room: Room, roomState: Bool) {
          return room is TrapRoom && !roomState;
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
      // TODO: Fix
      // See comment above GameState.roomState for info about null issue
      @:nullSafety(Off)
      rooms[i] = new Vector<Null<Room>>(floorSize);
    }

    rooms[0][0] = new EmptyRoom();
    rooms[1][0] = new EmptyRoom();
    rooms[0][1] = new EmptyRoom();
    rooms[1][1] = new EmptyRoom();
    rooms[2][1] = new TrapRoom(20, 40, 10, RoomDirection.North);
    rooms[3][1] = new EmptyRoom();
  }

  public static function GetRoom(x: Int, y: Int): Room {
    return Helpers.NullCheck(rooms[y][x], "Room (" + x + ", " + y + ") does not exist");
  }
}
