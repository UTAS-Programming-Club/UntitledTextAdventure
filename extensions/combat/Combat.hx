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

  var damage = -enemy.damage;

  // Attempt dodge via agility, damage is scaled by (max - actual) / (max - min)
  var agility = state.player.agility;
  agility = Std.int(Math.max(agility, enemy.minDodgeAgility));
  agility = Std.int(Math.min(agility, enemy.maxDodgeAgility));
  damage *= enemy.maxDodgeAgility - agility;
  damage = Std.int(damage / (enemy.maxDodgeAgility - enemy.minDodgeAgility));

  // Absorb damage via armor
  switch (enemy.type) {
    case Physical:
      damage += state.player.physicalDefence;
    case Magical:
      damage += state.player.magicalDefence;
  }

  state.player.modifyHealth(damage);

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
