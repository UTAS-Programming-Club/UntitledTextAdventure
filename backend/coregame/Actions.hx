package backend.coregame;

import backend.Action;
import backend.GameInfo;

class StartGame extends Action {
}

class GotoScreen extends Action {
  final screen: GameScreen;

  public function new(screen: GameScreen) {
    super();
    this.screen = screen;
  }
}

class GotoPreviousScreen extends Action {
}

class Quit extends Action {
}
