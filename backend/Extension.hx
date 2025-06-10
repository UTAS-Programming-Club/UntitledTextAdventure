package backend;

import backend.Game;
import backend.GameInfo;
import backend.Screen;

typedef Extension = {
  final actionHandler: Null<(state: Game, action: GameAction) -> GameOutcome>;
  final textHandler: Null<(state: Game, str: UnicodeString) -> GameOutcome>;
}
