package backend;

import backend.GameInfo;
import backend.Outcome;
import backend.Screen;

  // TODO: Use values or remove
typedef Extension = {
  // TODO: Check type when constructing actions
  final actions: Array<GameAction>;
  // final equipment: Array<?>;
  // TODO: Check type when constructing outcomes
  final outcomes: Array<{type: GameOutcome, constructor: Void -> Outcome}>;
  // final rooms: Array<?>;
  // Do not use [], get or exists when looping over multiple extensions
  // Haxe considers the various GameScreen enums to be abstract ints so the nth screen for each extension are all equal
  final screens: Map<GameScreen, Screen>;
}
