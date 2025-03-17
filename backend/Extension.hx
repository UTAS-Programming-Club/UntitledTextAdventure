package backend;

import backend.Game;
import backend.GameInfo;
import backend.Screen;

typedef Extension = {
  // TODO: Make optional?
  final actionHandler: (state: Game, action: GameAction) -> GameOutcome;
}
