package backend;

import backend.GlobalData;
import backend.Player;
import backend.Screen;

class GameState {
  public var currentScreen(default, null): Screen = GlobalData.mainMenuScreen;
  public final player = new Player();

  public function new(): Void {
  }

  public function HandleGameInput(action: ScreenActionType): ScreenActionOutcome {
    switch (action) {
      case ScreenActionType.GotoScreen(screen):
        currentScreen = screen;
        return ScreenActionOutcome.GetNextOutput;
      case ScreenActionType.QuitGame:
        return ScreenActionOutcome.QuitGame;
      case ScreenActionType.GoNorth:
        player.Y = Std.int(Math.min(player.Y + 1, GlobalData.floorSize));
        return ScreenActionOutcome.GetNextOutput;
      case ScreenActionType.GoEast:
        player.X = Std.int(Math.min(player.X + 1, GlobalData.floorSize));
        return ScreenActionOutcome.GetNextOutput;
      case ScreenActionType.GoSouth:
        player.Y = Std.int(Math.max(player.Y - 1, 0));
        return ScreenActionOutcome.GetNextOutput;
      case ScreenActionType.GoWest:
        player.X = Std.int(Math.max(player.X - 1, 0));
        return ScreenActionOutcome.GetNextOutput;
      default:
        throw new haxe.Exception("Unknown screen action " + action + " recevied");
    }
  }
}
