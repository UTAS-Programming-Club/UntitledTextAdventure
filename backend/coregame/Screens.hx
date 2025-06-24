package backend.coregame;

import backend.Game;
import backend.Screen;

class MainMenu extends Screen {
  public function getBody(state: Game): UnicodeString return state.campaign.mainMenu;
}

class Load extends Screen {
  public function getBody(Game): UnicodeString return 'Loading is not currently supported';
}


// stat must be in [0, 100]
function CreateStatBar(stat: Int): UnicodeString {
  final boxCount: Int = Std.int(stat / 10);
  final bar: UnicodeString = [for (i in 0...boxCount) '█'].join('');
  final gap: UnicodeString = [for (i in 0...(10 - boxCount)) ' '].join('');
  final percentage: UnicodeString = StringTools.lpad(Std.string(stat), ' ', 3);
  return bar + gap + ' : ' + percentage + '%';
}

class PlayerEquipment extends Screen {
  public function getBody(state: Game): UnicodeString return
    'Player Equipment\n\n' +
    'Health:  ' + CreateStatBar(state.player.health) + '\n' +
    'Stamina: ' + CreateStatBar(state.player.stamina)/* + '\n\n' +
    'Head:             ' + state.player.head.name + '\n' +
    'Upper Body:       ' + state.player.upperBody.name + '\n' +
    'Hands:            ' + state.player.hands.name + '\n' +
    'Lower Body:       ' + state.player.lowerBody.name + '\n' +
    'Feet:             ' + state.player.feet.name + '\n' +
    'Primary Weapon:   ' + state.player.primaryWeapon.name + '\n' +
    'Secondary Weapon: ' + state.player.secondaryWeapon.name*/;
}