package extensions.combat;

import backend.Action;
import backend.coregame.Outcomes;
import backend.Game;
import backend.GameInfo;
import extensions.combat.Enemy;

class AttackEnemy extends Action {
  final enemyNumber: Int;

  public function new(enemyNumber: Int, title: UnicodeString) {
    super(title);
    this.enemyNumber = enemyNumber;
  }

  override function isVisible(state: Game): Bool return enemyNumber < TestEnemies.length;

  function onTrigger(state: Game): GameOutcome return GetNextOutput;
}
