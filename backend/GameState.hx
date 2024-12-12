package backend;

import backend.GlobalData;
import backend.Player;
import backend.Screen;
import haxe.ds.Vector;

class GameState {
  public var roomState = new Vector<Vector<Null<Bool>>>(GlobalData.floorSize);

  public var currentScreen(default, null): Screen = GlobalData.mainMenuScreen;
  public final player = new Player();

  public function new(): Void {
    // TODO: Move to a function that runs on first visiting the game screen after initial load or quit
    // Currently state is kept when returning to the main menu and starting a new game
    for (i in 0...roomState.length) {
      roomState[i] = new haxe.ds.Vector(GlobalData.floorSize);
    }

    // TODO: Add init for each room to couple this to room creation per state
    roomState[2][1] = false;
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
