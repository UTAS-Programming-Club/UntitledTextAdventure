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

@:generic
class GotoScreen<T : Screen & Constructible<Void -> Void>> extends Action {
  function trigger(state: Game): GameOutcome {
    state.gotoScreen(new T());
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
