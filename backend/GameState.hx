package backend;

import backend.GlobalData;
import backend.Player;
import backend.Room;
import backend.Screen;
import haxe.ds.Vector;

@:nullSafety(Strict)
class GameState {
  public var roomState(default, null) = new Vector<Vector<Bool>>(GlobalData.floorSize);
  public var currentScreen(default, null): Screen = GlobalData.mainMenuScreen;
  public var inGame: Bool = false;

  public var player(default, null): Player = new Player();

  public function new(): Void {
  }

  public function SetupGame(): Void {
    player = new Player();
    inGame = true;

    for (y in 0...roomState.length) {
      roomState[y] = new haxe.ds.Vector(GlobalData.floorSize);
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
      default:
        throw new haxe.Exception("Unknown screen action " + action + " recevied");
    }
  }
}
