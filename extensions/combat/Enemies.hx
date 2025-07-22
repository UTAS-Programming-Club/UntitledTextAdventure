package extensions.combat;

final TestPhysicalEnemy: Enemy = {
  type: Physical,
  damage: 10,
  minDodgeAgility: 15,
  maxDodgeAgility: 25,
};

final TestMagicalEnemy: Enemy = {
  type: Magical,
  damage: 3,
  minDodgeAgility: 30,
  maxDodgeAgility: 50,
};
