package backend;

import backend.coregame.Outcomes;
import backend.Game;
import backend.GameInfo;
import backend.Screen;

abstract class Action {
  public final title: UnicodeString;

  public function new(title: UnicodeString) {
    this.title = title;
  }

  public function isVisible(Game, ActionScreen): Bool return true;
  public abstract function trigger(state: Game): GameOutcome;

  public function handleAction(state: Game): GameOutcome {
    final outcome: GameOutcome = trigger(state);
    if (outcome == Invalid) {
    // TODO: Cleanup now that there is no _ prefix
    // Should these be the other way?
    // Given Std.string(action) == backend.coregame._StartGame, top one prints StartGame, bottom just prints the whole thing
#if debug
      throw 'Unhandled action ${StringTools.replace(Std.string(this), '_', '').split('.').pop()} on ${state.getScreen()}.';
#else
      throw 'Unhandled action $this on ${state.getScreen()}.';
#end
    }

    return outcome;
  }
}
