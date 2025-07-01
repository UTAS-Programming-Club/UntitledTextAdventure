package extensions.trap;

import backend.Game;
import backend.GameInfo;
import backend.Room;
import backend.Screen;

final TrapRoom: GameRoom = new Trap_Trap();
class Trap_Trap extends StatefulRoom<TrapRoomState> {
  override function getStatefulRoomBody(Game, state: TrapRoomState): UnicodeString {
    var body: UnicodeString = '\n\nThis is a trap room which has';

    if (!state.activatedTrap) {
      body += ' not';
    }

    body += ' been triggered.';

    return body;
  }
}

class TrapRoomState extends ScreenState {
  public var activatedTrap: Bool = false;
}
