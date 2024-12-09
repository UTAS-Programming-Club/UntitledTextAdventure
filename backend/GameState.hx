package backend;

import backend.GlobalData;
import backend.Screen;

class GameState {
  public var currentScreen(default, null): Screen = GlobalData.mainMenuScreen;

  public function new(): Void {
  }

  public function HandleGameInput(action: ScreenActionType): ScreenActionOutcome {
    switch (action) {
      case ScreenActionType.GotoScreen(screen):
        currentScreen = screen;
        return ScreenActionOutcome.GetNextOutput;
      case ScreenActionType.QuitGame:
        return ScreenActionOutcome.QuitGame;
      default:
        throw new haxe.Exception("Unknown screen action " + action + " recevied");
    }
  }
}
