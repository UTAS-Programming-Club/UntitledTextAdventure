package backend;

import backend.GameEnums;
import backend.Screen;

typedef Extension = {
  final actions: Array<Enum<Any>>;
  final screenObjs: Array<Map<GameScreen, Screen>>;
  // final actionHandler: Null<(state: Game, action: GameAction) -> GameOutcome>;
}
