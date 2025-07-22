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

final CombatScreen: GameScreen = new Combat_Combat();
class Combat_Combat extends ActionScreen {
  function getBody(state: Game): UnicodeString {
    HandleCombat(state);

    var body: UnicodeString =
      'You find yourself surrounded.\n\n' +
      'Health:  ' + CreateStatBar(state.player.health) + '\n' +
      'Stamina: ' + CreateStatBar(state.player.stamina) + '\n';

    var longestTypeLength: Int = 0;
    for (enemy in TestEnemies) {
      final newTypeLength: Int = Std.string(enemy.type).length;
      longestTypeLength = Std.int(Math.max(longestTypeLength, newTypeLength));
    }

    for (i in 0...TestEnemies.length) {
      final enemy: Enemy = TestEnemies[i];
      body += '\n' + Std.string(enemy.type).lpad(' ', longestTypeLength) +
              ' enemy ${i + 1} health: ' +
              CreateStatBar(enemy.health);
    }

    return body;
  };

  function getAllActions(): Array<Action> {
    var actions: Array<Action> = [];

    for (i in 0...MaxEnemyCount) {
      actions.push(new AttackEnemy(i, 'Attack Enemy ${i + 1}'));
    }
    actions.push(new GotoScreen(PlayerEquipmentScreen, 'Check Inventory'));
    actions.push(new FleeCombat('Flee'));

    return actions;
  }
}
