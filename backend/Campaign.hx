package backend;

import backend.Equipment;
import backend.Extension;
import backend.GameInfo;

typedef Campaign = {
  final mainMenu: UnicodeString;

  final extensions: Array<Extension>;

  final initialScreen: GameScreen;
  final gameScreen: GameScreen;

  final rooms: Array<Array<GameRoom>>;
  final initialRoomX: UInt;
  final initialRoomY: UInt;

  final initialChest: GameEquipment;
}
