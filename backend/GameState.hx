package backend;

import backend.Campaign;
// import backend.CoreGame;
import backend.Helpers;
import backend.Player;
import backend.Room;
import backend.Screen;
import haxe.ds.Vector;



@:nullSafety(Strict)
class GameState {
  final campaign: Campaign;

  // TODO: Fix
  // Python implementation of Vector.new(Int) passes null to python.Syntax.code
  // which is not allowed, so pass an alternative representation
  // "Null safety: Cannot pass nullable value to not-nullable argument "args" of function "code""
  // Only access roomState after calling GameState.SetupGame which is run by GlobalData.gameScreen
  // except via ActionScreen.GetActions which handles partially setup roomState
  @:nullSafety(Off)
  public var roomState(default, null): Vector<Vector<Null<BasicRoomState>>> = null;
  public var currentScreen(default, null): Null<Screen> = null; //CoreScreens[CoreScreen.MainMenu];
  public var inGame: Bool = false;

  public var player(default, null): Player = new Player();

  public function new(campaign: Campaign) {
    this.campaign = campaign;
    // trace(this.campaign.extensions[0]);
    // Sys.stdin().readLine();

    player = new Player();
    inGame = true;

    // TODO: Fix
    // See comment above GameState.roomState for info about null issue
    @:nullSafety(Off)
    roomState = new Vector<Vector<Null<BasicRoomState>>>(campaign.floorSize, new Vector(0, null));
    for (y in 0...roomState.length) {
      // TODO: Fix
      // See comment above GameState.roomState for info about null issue
      @:nullSafety(Off)
      roomState[y] = new Vector<Null<BasicRoomState>>(campaign.floorSize, null);

      for (x in 0...roomState.length) {
        try {
      //     final room: Null<Room> = GlobalData.rooms[y][x];
      //     if (room == null) {
      //       continue;
      //     }
      // 
      //     final statefulRoom = cast(room, StatefulRoom<BasicRoomState>);
      //     roomState[y][x] = statefulRoom.CreateState();
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
      default:
        throw new haxe.Exception("Unknown screen action " + action + " recevied");
    }
  }
}
