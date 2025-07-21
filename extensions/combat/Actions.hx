package extensions.combat;

import backend.Action;
import backend.coregame.Outcomes;
import backend.Game;
import backend.GameInfo;
import extensions.combat.Enemy;
import extensions.combat.Combat;

class AttackEnemy extends Action {
  final enemyNumber: Int;

  public function new(enemyNumber: Int, title: UnicodeString) {
    super(title);
    this.enemyNumber = enemyNumber;
  }

  override function isVisible(state: Game): Bool return enemyNumber < TestEnemies.length;

  function onTrigger(state: Game): GameOutcome {
    // TODO: Allow using secondary weapon
    HandleCombat(state, state.player.primaryWeapon, enemyNumber);
    return GetNextOutput;
  }
}

class FleeCombat extends Action {
  function onTrigger(state: Game): GameOutcome {
    final previousX = Std.int(state.previousRoom % state.campaign.rooms.length);
    final previousY = Std.int(state.previousRoom / state.campaign.rooms.length);
    state.gotoRoom(previousX, previousY);
    return GetNextOutput;
  }
}
