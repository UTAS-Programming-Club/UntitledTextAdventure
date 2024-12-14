package backend;

import backend.GlobalData;
import backend.Player;
import backend.Room;
import backend.Screen;
import haxe.ds.Vector;

@:nullSafety(Strict)
class GameState {
  // Python implementation of Vector.new(Int) passes null to python.Syntax.code
  // which is not allowed, so pass an alternative representation
  // "Null safety: Cannot pass nullable value to not-nullable argument "args" of function "code""
  // Only access roomState after calling GameState.SetupGame which is run by GlobalData.gameScreen
  // except via ActionScreen.GetActions which handles partially setup roomState
  public var roomState(default, null) = new Vector<Vector<Bool>>(GlobalData.floorSize, new Vector(0, false));
  public var currentScreen(default, null): Screen = GlobalData.mainMenuScreen;
  public var inGame: Bool = false;

  public var player(default, null): Player = new Player();

  public function new(): Void {
  }

  public function SetupGame(): Void {
    player = new Player();
    inGame = true;

    for (y in 0...roomState.length) {
      roomState[y] = new Vector<Bool>(GlobalData.floorSize, false);
    }
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
      case DodgeTrap:
        roomState[player.Y][player.X] = true;
        return ScreenActionOutcome.GetNextOutput;
      default:
        throw new haxe.Exception("Unknown screen action " + action + " recevied");
    }
  }
}
