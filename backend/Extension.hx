package backend;

import backend.GameInfo;
import backend.Outcome;
import backend.Screen;

typedef Extension = {
  final actions: Array<GameAction>;
  // final equipment: Array<?>;
  final outcomes: Array<{type: GameOutcome, constructor: Void -> Outcome}>;
  // final rooms: Array<?>;
  final screens: Array<GameScreen>;
}
