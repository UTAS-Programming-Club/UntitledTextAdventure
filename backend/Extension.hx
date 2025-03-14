package backend;

import backend.Game;
import backend.GameInfo;
import backend.Screen;

typedef Extension = {
  final actionHandler: (state: Game, action: GameAction) -> GameOutcome;
}
