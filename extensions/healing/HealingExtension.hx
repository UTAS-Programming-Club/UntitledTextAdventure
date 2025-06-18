package extensions.healing;

import backend.Extension;
import backend.Game;
import backend.GameInfo;

@:nullSafety(Strict)
final HealingExt: Extension = {
  actionHandler: function(state: Game, action: GameAction): GameOutcome {
    switch (action) {
      case Heal:
        state.player.ModifyHealth(100);
        return GetNextOutput;
      default:
        return Invalid;
    }
  }
};
