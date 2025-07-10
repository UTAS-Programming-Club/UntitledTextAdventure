package extensions.trap;

import backend.Action;
import backend.Game;
import backend.Room;
import backend.Screen;
import extensions.trap.Actions;

class TrapRoom extends StatefulRoom<TrapRoomState> {
  function getStatefulRoomBody(Game, state: TrapRoomState): UnicodeString {
    var body: UnicodeString = '\n\nThis is a trap room which has';

    if (!state.activated) {
      body += ' not';
    }

    body += ' been triggered.';

    return body;
  }

  override function getRoomActions(): Array<Action> return [
    new DodgeTrap("Dodge Trap"),
  ];

  function getMapSymbol(): UnicodeString return '⚠';
}

class TrapRoomState extends RoomState {
  public var activated: Bool = false;

  function isCompleted(): Bool return activated;
  function requireCompleted(): Bool return true;
}
