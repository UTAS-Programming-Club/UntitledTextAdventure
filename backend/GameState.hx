package backend;

import backend.GlobalData;
import backend.Helpers;
import backend.Player;
import backend.Room;
import backend.Screen;
import haxe.ds.Vector;

@:nullSafety(Strict)
class GameState {
  // TODO: Fix
  // Python implementation of Vector.new(Int) passes null to python.Syntax.code
  // which is not allowed, so pass an alternative representation
  // "Null safety: Cannot pass nullable value to not-nullable argument "args" of function "code""
  // Only access roomState after calling GameState.SetupGame which is run by GlobalData.gameScreen
  // except via ActionScreen.GetActions which handles partially setup roomState
  @:nullSafety(Off)
  public var roomState(default, null) = new Vector<Vector<Null<BasicRoomState>>>(GlobalData.floorSize, new Vector(0, null));
  public var currentScreen(default, null): Screen = GlobalData.mainMenuScreen;
  public var inGame: Bool = false;

  public var player(default, null): Player = new Player();

  public function new(): Void {
  }

  public function SetupGame(): Void {
    player = new Player();
    inGame = true;

    for (y in 0...roomState.length) {
      // TODO: Fix
      // See comment above GameState.roomState for info about null issue
      @:nullSafety(Off)
      roomState[y] = new Vector<Null<BasicRoomState>>(GlobalData.floorSize, null);

      for (x in 0...roomState.length) {
        try {
          final room: Null<Room> = GlobalData.rooms[y][x];
          if (room == null) {
            continue;
          }

          final statefulRoom = cast(room, StatefulRoom<BasicRoomState>);
          roomState[y][x] = statefulRoom.CreateState();
        } catch(e) {
        }
      }
    }
  }

  @:generic()
  public function GetCurrentRoomState<T>(): T {
    final state: BasicRoomState = Helpers.NullCheck(
      roomState[player.Y][player.X],
      "Room (" + player.X + ", " + player.Y + ")'s state is missing"
    );

    // TODO: Fix error
    // Only @:const type parameters on @:generic classes can be used as value, @:const is not supported for generic functions
    // if (!Std.isOfType(state, T)) {
    //   Helpers.ThrowStr("Room (" + player.X + ", " + player.Y + ")'s state is broken");
    // }

    return cast state;
  }

  public function HandleGameInput(action: ScreenActionType): ScreenActionOutcome {
    switch (action) {
      case GotoScreen(screen):
        currentScreen = screen;
        return ScreenActionOutcome.GetNextOutput;
      case QuitGame:
        return ScreenActionOutcome.QuitGame;
      case GoNorth:
        player.Y = Std.int(Math.min(player.Y + 1, GlobalData.floorSize));
        return ScreenActionOutcome.GetNextOutput;
      case GoEast:
        player.X = Std.int(Math.min(player.X + 1, GlobalData.floorSize));
        return ScreenActionOutcome.GetNextOutput;
      case GoSouth:
        player.Y = Std.int(Math.max(player.Y - 1, 0));
        return ScreenActionOutcome.GetNextOutput;
      case GoWest:
        player.X = Std.int(Math.max(player.X - 1, 0));
        return ScreenActionOutcome.GetNextOutput;
      // TODO: Move to TrapRoom class
      case DodgeTrap:
        final state: Null<BasicRoomState> = roomState[player.Y][player.X];
        if (state != null) {
          state.completed = true;
        }
        return ScreenActionOutcome.GetNextOutput;
      default:
        throw new haxe.Exception("Unknown screen action " + action + " recevied");
    }
  }
}
