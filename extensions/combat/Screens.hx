package extensions.combat;

using StringTools;

import backend.Action;
import backend.Game;
import backend.GameInfo;
import backend.Screen;

import backend.coregame.Actions;
import backend.coregame.Screens;
import extensions.combat.Actions;
import extensions.combat.Combat;
import extensions.combat.Enemy;
import extensions.combat.Rooms;

final CombatScreen: GameScreen = new Combat_Combat();
class Combat_Combat extends ActionScreen {
  function getBody(state: Game): UnicodeString {
    final room: CombatRoom = cast(state.getRoom(), CombatRoom);
    HandleCombat(state, room);

    var body: UnicodeString =
      'You find yourself surrounded.\n\n' +
      'Health:  ' + CreateStatBar(state.player.health) + '\n' +
      'Stamina: ' + CreateStatBar(state.player.stamina) + '\n';

    var longestTypeLength: Int = 0;
    for (enemy in room.enemies) {
      if (enemy.health > 0) {
        final newTypeLength: Int = Std.string(enemy.type).length;
        longestTypeLength = Std.int(Math.max(longestTypeLength, newTypeLength));
      }
    }

    for (i in 0...room.enemies.length) {
      final enemy: Enemy = room.enemies[i];
      if (enemy.health > 0) {
        body += '\n' + Std.string(enemy.type).lpad(' ', longestTypeLength) +
                ' enemy ${i + 1} health: ' +
                CreateStatBar(enemy.health);
      }
    }

    return body;
  };

  function getAllActions(): Array<Action> {
    var actions: Array<Action> = [];

    for (i in 0...MaxEnemyCount) {
      actions.push(new AttackEnemy(i, 'Attack Enemy ${i + 1}'));
    }
    // TODO: Detect changing equipment and skip turn
    actions.push(new GotoScreen(PlayerEquipmentScreen, 'Check Inventory'));
    actions.push(new FleeCombat('Flee'));

    return actions;
  }
}
