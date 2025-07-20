package extensions.combat;

enum EnemyAttackType {
  Physical;
  Magical;
}

final MaxEnemyCount: Int = 3;
@:structInit
class Enemy {
  public var health: Int = 100;
  public final type: EnemyAttackType;
  // final damage: Int;
  // final minDodgeAgility: Int;
  // final maxDodgeAgility: Int;
}

final TestEnemy: Enemy = {
  type: Physical,
};

final TestEnemy2: Enemy = {
  type: Magical,
};

// TODO: Remove
final TestEnemies: Array<Enemy> = [TestEnemy, TestEnemy2];
