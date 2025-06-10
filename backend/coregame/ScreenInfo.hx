package backend.coregame;

using StringTools;

import backend.Game;
import backend.GameInfo;
import backend.Save;
import backend.Screen;

// stat must be in [0, 100]
function CreateStatBar(stat: Int): UnicodeString {
  final boxCount: Int = Std.int(stat / 10);
  final bar: UnicodeString = [for (i in 0...boxCount) '█'].join('');
  final gap: UnicodeString = [for (i in 0...(10 - boxCount)) ' '].join('');
  final percentage: UnicodeString = Std.string(stat).lpad(' ', 3);
  return bar + gap + ' : ' + percentage + '%';
}

function GenerateLoadBody(state: Game, Screen) : UnicodeString {
  // TODO: Figure out why Save.Load gives "backend.GameScreen has no field Load"
  return backend.Save.Load(state, "YYO01OJFAJ89#");
}

function GenerateEquipmentBody(state: Game, Screen): UnicodeString {
  return
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
}

final CoreScreens: Map<GameScreen, Screen> = [
  MainMenu => new ActionScreen(
    function(state: Game, Screen): UnicodeString {
      return state.campaign.mainMenu;
    }, [
    new ScreenAction(StartGame, "Start Game"),
    new ScreenAction(GotoScreen(Load), "Load Game"),
    new ScreenAction(QuitGame, "Quit Game")
  ]),
  Load => new ActionScreen(GenerateLoadBody, [
    new ScreenAction(GotoScreen(MainMenu), "Return to Main Menu")
  ]),
  PlayerEquipment => new ActionScreen(GenerateEquipmentBody, [
    new ScreenAction(GotoPreviousScreen, "Return to Game")
  ])
];
