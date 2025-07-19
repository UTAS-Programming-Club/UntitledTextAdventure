package extensions.combat;

using StringTools;

import backend.Action;
import backend.coregame.Actions;
import backend.coregame.Screens;
import backend.Game;
import backend.GameInfo;
import backend.Screen;
import extensions.combat.Enemy;

final CombatScreen: GameScreen = new Combat_Combat();
class Combat_Combat extends ActionScreen {
  function getBody(state: Game): UnicodeString {
    var body: UnicodeString =
      'You find yourself surrounded.\n\n' +
      'Health:  ' + CreateStatBar(state.player.health) + '\n' +
      'Stamina: ' + CreateStatBar(state.player.stamina) + '\n\n';

    // TODO: Remove
    final enemies: Array<Enemy> = [TestEnemy, TestEnemy2];

    var longestTypeLength: Int = 0;
    for (enemy in enemies) {
      final newTypeLength: Int = Std.string(enemy.type).length;
      longestTypeLength = Std.int(Math.max(longestTypeLength, newTypeLength));
    }

    for (i in 0...enemies.length) {
      final enemy: Enemy = enemies[i];
      body += Std.string(enemy.type).lpad(' ', longestTypeLength) +
              ' enemy ${i + 1} health: ' +
              CreateStatBar(enemy.health);
      if (i != enemies.length - 1) {
        body += '\n';
      }
    }

    return body;
  };

  function getAllActions(): Array<Action> return [
    // TODO: Fix getting stuck in loop if two screens away from game screen
    new GotoScreen(PlayerEquipmentScreen, 'Check Inventory'),
    new GotoPreviousScreen('Flee'),
  ];
}
