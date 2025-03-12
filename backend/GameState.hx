package backend;

import backend.GameInfo;

@:nullSafety(Strict)
class GameState {
  public var currentScreen(default, null): Null<Screen>;

  public function new() {
    currentScreen = GameInfo.ScreenInfo[MainMenu];
  }
}
