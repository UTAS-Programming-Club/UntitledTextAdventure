package extensions.trap;

import backend.GameInfo;
import extensions.trap.Rooms;

@:nullSafety(Strict)
final TrapRooms: Map<GameRoom, Void -> backend.Room> = [
  Trap => TrapRoom.new
];
