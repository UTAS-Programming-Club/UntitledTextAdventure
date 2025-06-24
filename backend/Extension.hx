package backend;

import backend.GameInfo;

  // TODO: Use values or remove
typedef Extension = {
  // TODO: Check type when constructing actions
  final actions: Array<GameAction>;
  // final equipment: Array<?>;
  // TODO: Check type when constructing outcomes
  final outcomes: Array<{type: GameOutcome, constructor: Void -> Outcome}>;
  // final rooms: Array<?>;
  final screens: Array<{type: GameScreen, constructor: Void -> Screen}>;
}
