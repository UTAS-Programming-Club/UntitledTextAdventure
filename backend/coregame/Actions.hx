package backend.coregame;

import backend.Action;
import backend.coregame.Outcomes;
import backend.GameInfo;
import backend.Screen;
import haxe.Constraints;

class StartGame extends Action {
  function trigger(state: Game): GameOutcome {
    state.startGame();
    return GetNextOutput;
  }
}

class GotoScreen extends Action {
  private final screen: GameScreen;

  public function new(screen: GameScreen, title: UnicodeString) {
    super(title);
    this.screen = screen;
  }

  function trigger(state: Game): GameOutcome {
    state.gotoScreen(screen);
    return GetNextOutput;
  }
}

class GotoPreviousScreen extends Action {
  function trigger(state: Game): GameOutcome {
    state.gotoScreen(state.previousScreen);
    return GetNextOutput;
  }
}

class Quit extends Action {
  function trigger(state: Game): GameOutcome {
    return QuitGame;
  }
}
