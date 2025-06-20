package extensions.equipment;

import backend.Extension;
import backend.GameInfo;

@:nullSafety(Strict)
final EquipmentExt: Extension = {
  // TODO: Make optional?
  actionHandler: function(state: Game, action: GameAction): GameOutcome {
    // This extension defines no actions currently
    return Invalid;
  }
};
