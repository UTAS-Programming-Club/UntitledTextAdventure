package extensions.healing;

import backend.Action;
import backend.coregame.Outcomes;
import backend.Game;
import backend.GameInfo;

class HealPlayer extends Action {
  function onTrigger(state: Game): GameOutcome {
    state.player.modifyHealth(100);
    return GetNextOutput;
  }
}
