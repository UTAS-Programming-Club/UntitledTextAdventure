package extensions.combat;

import backend.Equipment;
import backend.Game;
import extensions.combat.Enemy;
import extensions.combat.Rooms;

enum CombatPhase {
  WaitingForInput;
  PlayerAttack;
  EnemyAttacks;
}

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

function HandleCombat(state: Game): Void {
  final roomState: CombatRoomState = state.getRoomState();
#if debuggame
  if (roomState.currentEnemyNumber >= TestEnemies.length) {
    throw ': Enemy to damage does not exist';
  }
#end

  switch (roomState.phase) {
    case PlayerAttack:
      PerformPlayerAttack(roomState.currentWeapon, roomState.currentEnemyNumber);
      roomState.phase = EnemyAttacks;
      state.repeatLastOutput = true;
    case EnemyAttacks:
      for (enemy in TestEnemies) {
        PerformEnemyAttack(state, enemy);
      }
      roomState.phase = WaitingForInput;
      state.repeatLastOutput = false;
    case WaitingForInput:
  }
}
