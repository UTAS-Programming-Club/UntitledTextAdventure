package extensions.rooms;

import backend.Extension;
import backend.Game;
import backend.GameInfo;

@:nullSafety(Strict)
final RoomsExt: Extension = {
  // TODO: Make optional?
  actionHandler: function(state: Game, action: GameAction): GameOutcome {
    // This extension defines no actions currently
    return Invalid;
  }
};
