package backend.coregame;

import backend.coregame.Actions;
import backend.coregame.Outcomes;
import backend.Game;
import backend.GameInfo;
import backend.Save;
import backend.Screen;

final MainMenuScreen: GameScreen = new CoreGame_MainMenu();
class CoreGame_MainMenu extends ActionScreen {
  function getBody(state: Game): UnicodeString return state.campaign.mainMenu;

  function getAllActions(): Array<Action> return [
    new StartGame('Start Game'),
    new GotoScreen(LoadScreen, 'Load Game'),
    new Quit('Quit Game'),
  ];
}


final SaveScreen: GameScreen = new CoreGame_Save();
class CoreGame_Save extends ActionScreen {
  function getBody(state: Game): UnicodeString return 'This is a test: ' + Save(state);

  function getAllActions(): Array<Action> return [
    new Quit('Quit'),
  ];
}

final LoadScreen: GameScreen = new CoreGame_Load();
class CoreGame_Load extends TextScreen {
  function getBody(state: Game): UnicodeString return 'Password';

  function onTextEntry(state:backend.Game, str:UnicodeString): GameOutcome {
    state.loadGame(str);
    return GetNextOutput;
  }
}


// stat must be in [0, 100]
function CreateStatBar(stat: Int): UnicodeString {
  final boxCount: Int = Std.int(stat / 10);
  final bar: UnicodeString = [for (i in 0...boxCount) '█'].join('');
  final gap: UnicodeString = [for (i in 0...(10 - boxCount)) ' '].join('');
  final percentage: UnicodeString = StringTools.lpad(Std.string(stat), ' ', 3);
  return bar + gap + ' : ' + percentage + '%';
}

final PlayerEquipmentScreen: GameScreen = new CoreGame_PlayerEquipment();
class CoreGame_PlayerEquipment extends ActionScreen {
  function getBody(state: Game): UnicodeString return
    'Player Equipment\n\n' +
    'Health:  ' + CreateStatBar(state.player.health) + '\n' +
    'Stamina: ' + CreateStatBar(state.player.stamina) + '\n\n' +
    'Head:             ' + state.player.head.name + '\n' +
    'Upper Body:       ' + state.player.upperBody.name + '\n' +
    'Hands:            ' + state.player.hands.name + '\n' +
    'Lower Body:       ' + state.player.lowerBody.name + '\n' +
    'Feet:             ' + state.player.feet.name + '\n' +
    'Primary Weapon:   ' + state.player.primaryWeapon.name + '\n' +
    'Secondary Weapon: ' + state.player.secondaryWeapon.name;

  function getAllActions(): Array<Action> return [
    new CycleEquipment(Head,            'Cycle Head Item'),
    new CycleEquipment(UpperBody,       'Cycle Upper Body Item'),
    new CycleEquipment(Hands,           'Cycle Hands Item'),
    new CycleEquipment(LowerBody,       'Cycle Lower Body Item'),
    new CycleEquipment(Feet,            'Cycle Feet Item'),
    new CycleEquipment(PrimaryWeapon,   'Cycle Primary Weapon'),
    new CycleEquipment(SecondaryWeapon, 'Cycle Secondary Weapon'),
    new GotoPreviousScreen('Return to Game'),
  ];
}
