package extensions.combat;

import backend.Equipment;
import backend.Game;
import extensions.combat.Enemy;

function DamageEnemy(enemy: Enemy, damage: Int): Void {
  final cappedDamage: Int = Std.int(Math.min(enemy.health, damage));
  enemy.health -= cappedDamage;
}

// TODO: Use return type or remove
function PerformPlayerAttack(weapon: Equipment, enemyNumber: Int): Bool {
  if (enemyNumber >= TestEnemies.length) {
    throw ': Enemy to damage does not exist';
  }

  final enemy: Enemy = TestEnemies[enemyNumber];
  if (enemy.health == 0) {
    return false;
  }

  // TODO: Add physical and magic defense stats to enemies
  // TODO: Add crits/some randomness to damage done
  DamageEnemy(enemy, weapon.physical);
  DamageEnemy(enemy, weapon.magical);
  return true;
}
