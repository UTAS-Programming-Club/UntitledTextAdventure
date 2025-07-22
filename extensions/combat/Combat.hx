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
function PerformPlayerAttack(weapon: Equipment, enemy: Enemy): Bool {
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

function HandleCombat(state: Game, room: CombatRoom): Void {
  final roomState: CombatRoomState = state.getRoomState();
#if debuggame
  if (roomState.chosenEnemyIdx >= room.enemies.length) {
    throw ': Enemy to damage does not exist';
  }
#end

  switch (roomState.phase) {
    case PlayerAttack:
      final enemy: Enemy = room.enemies[roomState.chosenEnemyIdx];
      PerformPlayerAttack(roomState.chosenWeapon, enemy);
      roomState.phase = EnemyAttacks;
      roomState.currentEnemyIdx = 0;
      state.repeatLastOutput = true;
    case EnemyAttacks:
      final enemy: Enemy = room.enemies[roomState.currentEnemyIdx];
      PerformEnemyAttack(state, enemy);
      roomState.currentEnemyIdx++;
      if (roomState.currentEnemyIdx == room.enemies.length) {
        roomState.phase = WaitingForInput;
        state.repeatLastOutput = false;
      }
    case WaitingForInput:
  }
}
