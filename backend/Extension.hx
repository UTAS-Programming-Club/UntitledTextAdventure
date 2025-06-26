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
  // Make sure the key is from the correct extension as exists, get and [] only check index and not type.
  final screens: Map<EnumValue, Screen>;
}
