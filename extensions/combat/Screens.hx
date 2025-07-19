package extensions.combat;

import backend.Action;
import backend.coregame.Actions;
import backend.Game;
import backend.GameInfo;
import backend.Screen;

final CombatScreen: GameScreen = new Combat_Combat();
class Combat_Combat extends ActionScreen {
  function getBody(state: Game): UnicodeString return
    'You find yourself surrounded.';

  function getAllActions(): Array<Action> return [
    new GotoPreviousScreen('Return to Game'),
  ];
}
