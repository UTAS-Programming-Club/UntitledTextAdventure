package extensions.combat;

enum EnemyAttackType {
  Physical;
  Magical;
}

@:structInit
class Enemy {
  // final health: Int;
  final type: EnemyAttackType;
  // final damage: Int;
  // final minDodgeAgility: Int;
  // final maxDodgeAgility: Int;
}

final TestEnemy: Enemy = {
  type: Physical,
};
