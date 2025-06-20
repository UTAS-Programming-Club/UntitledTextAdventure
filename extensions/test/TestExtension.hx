package extensions.test;

import backend.Extension;
import backend.GameInfo;

@:nullSafety(Strict)
final TestExt: Extension = {
  // TODO: Make optional?
  actionHandler: function(state: Game, action: GameAction): GameOutcome {
    // This extension defines no actions currently
    return Invalid;
  }
};
