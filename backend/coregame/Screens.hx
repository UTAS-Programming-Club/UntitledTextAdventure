package backend.coregame;

import backend.coregame.Actions;
import backend.Game;
import backend.GameInfo;
import backend.Room;
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

final LoadScreen: GameScreen = new CoreGame_Load();
class CoreGame_Load extends ActionScreen {
  function getBody(Game): UnicodeString return 'Loading is not currently supported';

  function getAllActions(): Array<Action> return [
    new GotoScreen(MainMenuScreen, 'Return to Main Menu'),
  ];
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
    new OpenMap('Open Map'),
    new GotoPreviousScreen('Return to Game'),
  ];
}

final MapScreen: GameScreen = new CoreGame_Map();
class CoreGame_Map extends ActionScreen {
  function getBody(state: Game): UnicodeString return RoomMap.createMap(state);

  function getAllActions(): Array<Action> return [
    new GotoPreviousScreen('Return to Game'),
  ];
}
