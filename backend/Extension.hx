package backend;

import backend.BaseGame;
import backend.GameEnums;
import backend.Screen;

typedef Extension = {
  final actions: Array<Enum<Any>>;
  final equipmentObjs: Array<Map<GameEquipment, Equipment>>;
  final roomObjs: Array<Map<GameRoom, Void -> Room>>;
  final screenObjs: Array<Map<GameScreen, Screen>>;
  final actionHandler: Null<(state: BaseGame, action: GameAction) -> GameOutcome>;
}
