package backend;

import backend.GameInfo;

// TODO: Allow extensions to depend on other extensions
typedef Extension = {
  final module: UnicodeString;

  final actions: Array<GameAction>;
  final equipment: Array<GameEquipment>;
  final outcomes: Array<GameOutcome>;
  final screens: Array<GameScreen>;
}
