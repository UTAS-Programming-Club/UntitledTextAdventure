package extensions.trap;

import backend.Extension;
import backend.Game;
import backend.GameInfo;

@:nullSafety(Strict)
final TrapExt: Extension = {
  // TODO: Make optional?
  actionHandler: function(state: Game, action: GameAction): GameOutcome {
    // This extension defines no actions currently
    return Invalid;
  }
};
