package backend;

// import backend.Game;
// import backend.GameInfo;
import backend.Screen;
import haxe.Constraints;

typedef Extension = {
  // final actions: Array<?>;
  // final equipment: Array<?>;
  // final outcomes: Array<?>;
  // final rooms: Array<?>;
  final screens: Array<{type: Class<Screen>, constructor: Void -> Screen}>;
  
  // final actionHandler: Null<(state: Game, action: GameAction) -> GameOutcome>;
}
