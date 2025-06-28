package backend;

using StringTools;

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
      throw   ': Unhandled action ${Std.string(this).split('.').pop()}'
            + ' on ${Std.string(state.getScreen()).replace('_', '.').split('.').pop()}.';
    }

    return outcome;
  }
}
