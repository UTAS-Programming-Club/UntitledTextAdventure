package backend;

import backend.GameInfo;

@:nullSafety(Strict)
class Game {
  public var currentScreen(default, null): Null<Screen>;

  public function new() {
    currentScreen = GameInfo.ScreenInfo[MainMenu];
  }
}
