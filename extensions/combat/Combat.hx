package extensions.combat;

import backend.Equipment;
import backend.Game;
import extensions.combat.Enemy;

// TODO: Use return type or remove
function PerformPlayerAttack(weapon: Equipment, enemyNumber: Int): Bool {
  final enemy: Enemy = TestEnemies[enemyNumber];
  if (enemy.health == 0) {
    return false;
  }

  // TODO: Add physical and magic defense stats to enemies
  // TODO: Add crits/some randomness to damage done
  enemy.modifyHealth(-weapon.physical);
  enemy.modifyHealth(-weapon.magical);
  return true;
}

// TODO: Use return type or remove
function PerformEnemyAttack(state: Game, enemy: Enemy): Bool {
  if (enemy.health == 0) {
    return false;
  }

  // TODO: Add player agility
  // TODO: Use physical and magical defence stats
  state.player.modifyHealth(-enemy.damage);

  return true;
}

// TODO: Use return type or remove
function HandleCombat(state: Game, weapon: Equipment, enemyNumber: Int): Bool {
    if (enemyNumber >= TestEnemies.length) {
    throw ': Enemy to damage does not exist';
  }

  // TODO: Return after each to redraw
  PerformPlayerAttack(weapon, enemyNumber);
  for (enemy in TestEnemies) {
    PerformEnemyAttack(state, enemy);
  }

  return true;
}
