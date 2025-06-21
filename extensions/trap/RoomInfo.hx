package extensions.trap;

import backend.GameEnums;
import extensions.trap.Rooms;

@:nullSafety(Strict)
final TrapRooms: Map<GameRoom, Void -> backend.Room> = [
  Trap => TrapRoom.new
];
