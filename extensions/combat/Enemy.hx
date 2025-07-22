package extensions.combat;

enum EnemyAttackType {
  Physical;
  Magical;
}

final MaxEnemyCount: Int = 3;
@:structInit
class Enemy {
  public var health(default, null): Int = 100;
  public final type: EnemyAttackType;
  public final damage: Int;
  public final minDodgeAgility: Int;
  public final maxDodgeAgility: Int;

  // change is clamped to [0, 100]
  public function modifyHealth(change: Int): Void {
    var newHealth: Int = health + change;
    if (newHealth < 0) {
      newHealth = 0;
    } else if (newHealth > 100) {
      newHealth = 100;
    }

    health = newHealth;
  }
}
