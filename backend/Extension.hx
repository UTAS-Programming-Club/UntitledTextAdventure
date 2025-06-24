package backend;

import backend.BaseGame;
import backend.GameEnums;
// import backend.Equipment;
// import backend.Room;
import backend.Screen;

typedef Extension = {
  final actions: Array<Enum<Any>>;
  // final equipment: Array<Enum<Any>>;
  final outcomes: Array<Enum<Any>>;
  final rooms: Array<Enum<Any>>;
  final screens: Array<Enum<Any>>;
  
  // final equipmentObjs: Array<Map<Enum<Any>, Equipment>>;
  // final roomObjs: Array<Map<GameRoom, Void -> Room>>;
  final screenObjs: Array<Map<GameScreen, Screen>>;
  final actionHandler: Null<(state: BaseGame, action: GameAction) -> GameOutcome>;
}
