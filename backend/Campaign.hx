package backend;

import backend.Extension;
import backend.GameEnums;

typedef Campaign = {
  final mainMenu: UnicodeString;

  final extensions: Array<Extension>;

  final initialScreen: GameScreen;
  final gameScreen: GameScreen;

  final rooms: Array<Array<GameRoom>>; // Must be square
  final initialRoomX: Int; // Must be in [0, rooms.length)
  final initialRoomY: Int; // Must be in [0, rooms.length)

  final initialHead: GameEquipment;
  final initialUpperBody: GameEquipment;
  final initialHands: GameEquipment;
  final initialLowerBody: GameEquipment;
  final initialFeet: GameEquipment;
  final initialPrimaryWeapon: GameEquipment;
  final initialSecondaryWeapon: GameEquipment;
}
