package backend;

import backend.GlobalData;
import backend.Screen;

class GameState {
  public var currentScreen(default, null): Screen = GlobalData.mainMenu;

  public function new(): Void {
  }

  public function HandleGameInput(action: ScreenActionType): Bool {
    switch (action) {
      case ScreenActionType.GotoScreen(screen):
        currentScreen = screen;
      default:
        return false;
    }

    return true;
  }
}
