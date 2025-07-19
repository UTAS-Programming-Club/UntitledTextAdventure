package extensions.combat;

enum EnemyAttackType {
  Physical;
  Magical;
}

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
