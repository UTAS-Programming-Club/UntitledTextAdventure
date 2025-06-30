package extensions.trap;

import backend.GameInfo;
import backend.Room;


final TrapRoom: GameRoom = new Trap_Trap();
class Trap_Trap extends StatefulRoom<TrapRoomState> {
}

class TrapRoomState extends RoomState {
  public var activatedTrap: Bool = false;
}
