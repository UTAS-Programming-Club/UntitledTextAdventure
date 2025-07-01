package extensions.trap;

import backend.Action;
import backend.Game;
import backend.GameInfo;
import backend.Room;
import backend.Screen;
import extensions.trap.Actions;

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

  override function getRoomActions(): Array<Action> return [
    new DodgeTrap("Dodge Trap"),
  ];
}

class TrapRoomState extends ScreenState {
  public var activatedTrap: Bool = false;
}
