package backend;

import backend.GlobalData;

class GameState {
  public var currentScreen(default, null): Screen = GlobalData.mainMenu;

  public function new(): Void {
  }
}
