package backend;

import backend.GameInfo;
import backend.Outcome;
import backend.Screen;

typedef Extension = {
  final module: UnicodeString;

  final actions: Array<GameAction>;
  // final equipment: Array<?>;
  final outcomes: Array<GameOutcome>;
  // final rooms: Array<?>;
  final screens: Array<GameScreen>;
}
