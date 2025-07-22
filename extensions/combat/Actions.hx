package extensions.combat;

import backend.Action;
import backend.Game;
import backend.GameInfo;

import backend.coregame.Outcomes;
import extensions.combat.Enemy;
import extensions.combat.Rooms;

class AttackEnemy extends Action {
  final enemyIdx: Int;

  public function new(enemyIdx: Int, title: UnicodeString) {
    super(title);
    this.enemyIdx = enemyIdx;
  }

  override function isVisible(state: Game): Bool {
    final room: CombatRoom = cast(state.getRoom(), CombatRoom);
    return enemyIdx < room.enemies.length && room.enemies[enemyIdx].health > 0;
  }

  function onTrigger(state: Game): GameOutcome {
    final roomState: CombatRoomState = state.getRoomState();
    roomState.phase = PlayerAttack;
    // TODO: Allow using secondary weapon
    roomState.chosenWeapon = state.player.primaryWeapon;
    roomState.chosenEnemyIdx = enemyIdx;
    return GetNextOutput;
  }
}

class FleeCombat extends Action {
  function onTrigger(state: Game): GameOutcome {
    final room: CombatRoom = cast(state.getRoom(), CombatRoom);
    for (enemy in room.enemies) {
      enemy.modifyHealth(100);
    }

    final previousX = Std.int(state.previousRoom % state.campaign.rooms.length);
    final previousY = Std.int(state.previousRoom / state.campaign.rooms.length);
    state.gotoRoom(previousX, previousY);
    return GetNextOutput;
  }
}
