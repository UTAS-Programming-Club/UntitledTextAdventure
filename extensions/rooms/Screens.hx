package extensions.rooms;

import backend.Game;
import backend.macros.Helpers;
import backend.Screen;

enum RoomsScreen {
  GameRooms;
}

@:nullSafety(Strict)
class GameRoomScreen extends ActionScreen {
  public var x(default, null): UInt;
  public var y(default, null): UInt;

  public function new(updateState: OneOf<UnicodeString, (Game, Screen) -> UnicodeString>,
                      ?actions: Array<ScreenAction>,
                      x: UInt, y: UInt) {
    super(updateState, actions);
    this.x = x;
    this.y = y;
  }
}
