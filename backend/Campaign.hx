package backend;

// import backend.Equipment;
import backend.Extension;
// import backend.GameInfo;
import backend.Screen;
import haxe.Constraints;

typedef Campaign = {
  final mainMenu: UnicodeString;

  final extensions: Array<Extension>;

  final initialScreen: Void -> Screen;
  // final gameScreen: GameScreen;

  // final rooms: Array<Array<GameRoom>>; // Must be square
  final initialRoomX: Int; // Must be in [0, rooms.length)
  final initialRoomY: Int; // Must be in [0, rooms.length)

  // final initialHead: GameEquipment;
  // final initialUpperBody: GameEquipment;
  // final initialHands: GameEquipment;
  // final initialLowerBody: GameEquipment;
  // final initialFeet: GameEquipment;
  // final initialPrimaryWeapon: GameEquipment;
  // final initialSecondaryWeapon: GameEquipment;
}
