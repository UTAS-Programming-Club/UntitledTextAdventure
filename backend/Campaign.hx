package backend;

import backend.Equipment;
import backend.Extension;
import backend.GameInfo;

typedef Campaign = {
  final mainMenu: UnicodeString;

  final extensions: Array<Extension>;

  final initialScreen: GameScreen;
  final gameScreen: GameScreen;

  final rooms: Array<Array<GameRoom>>; // Must be square
  final initialRoomX: Int; // Must be in [0, rooms.length)
  final initialRoomY: Int; // Must be in [0, rooms.length)

  final initialHead: GameEquipmentHead;
  final initialUpperBody: GameEquipmentUpperBody;
  final initialHands: GameEquipmentHands;
  final initialLowerBody: GameEquipmentLowerBody;
  final initialFeet: GameEquipmentFeet;
  final initialPrimaryWeapon: GameEquipmentWeapon;
  final initialSecondaryWeapon: GameEquipmentWeapon;
}
