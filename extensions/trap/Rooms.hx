package extensions.trap;

import backend.GameInfo;
import backend.Room;
import backend.Screen;

final TrapRoom: GameRoom = new Trap_Trap();
class Trap_Trap extends StatefulRoom<TrapRoomState> {
}

class TrapRoomState extends ScreenState {
  public var activatedTrap: Bool = false;
}
