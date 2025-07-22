package extensions.combat;

import backend.Action;
import backend.Game;
import backend.GameInfo;

import backend.coregame.Outcomes;
import extensions.combat.Enemy;
import extensions.combat.Rooms;

class AttackEnemy extends Action {
  final enemyNumber: Int;

  public function new(enemyNumber: Int, title: UnicodeString) {
    super(title);
    this.enemyNumber = enemyNumber;
  }

  override function isVisible(state: Game): Bool return enemyNumber < TestEnemies.length;

  function onTrigger(state: Game): GameOutcome {
    final roomState: CombatRoomState = state.getRoomState();
    roomState.phase = PlayerAttack;
    // TODO: Allow using secondary weapon
    roomState.currentWeapon = state.player.primaryWeapon;
    roomState.currentEnemyNumber = enemyNumber;
    return GetNextOutput;
  }
}

class FleeCombat extends Action {
  // TODO: Reset enemies to 100% health
  function onTrigger(state: Game): GameOutcome {
    final previousX = Std.int(state.previousRoom % state.campaign.rooms.length);
    final previousY = Std.int(state.previousRoom / state.campaign.rooms.length);
    state.gotoRoom(previousX, previousY);
    return GetNextOutput;
  }
}
