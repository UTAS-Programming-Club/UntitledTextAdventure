package backend.coregame;

import backend.Action;
import backend.GameInfo;

class _StartGame extends Action {
}
final StartGame: _StartGame = new _StartGame();

class _GotoScreen extends Action {
  public final screen: GameScreen;

  public function new(screen: GameScreen) {
    super();
    this.screen = screen;
  }
}
final GotoScreen: GameScreen -> _GotoScreen = _GotoScreen.new;

class _GotoPreviousScreen extends Action {
}
final GotoPreviousScreen: _GotoPreviousScreen = new _GotoPreviousScreen();

class _Quit extends Action {
}
final Quit: _Quit = new _Quit();
