package backend;

import backend.Extension;
import backend.GameInfo;

typedef Campaign = {
  final mainMenu: UnicodeString;

  final extensions: Array<Extension>;

  final initialScreen: GameScreen;

  final rooms: Array<Array<GameRoom>>; // Must be square
  final initialRoomX: Int; // Must be in [0, rooms.length)
  final initialRoomY: Int; // Must be in [0, rooms.length)

  // Only appending new equipment will retain save compatibility
  final equipmentOrder: Array<GameEquipment>;
  final initialHead: GameEquipment;
  final initialUpperBody: GameEquipment;
  final initialHands: GameEquipment;
  final initialLowerBody: GameEquipment;
  final initialFeet: GameEquipment;
  final initialPrimaryWeapon: GameEquipment;
  final initialSecondaryWeapon: GameEquipment;
}
