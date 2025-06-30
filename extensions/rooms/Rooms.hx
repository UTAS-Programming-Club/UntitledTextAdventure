package extensions.rooms;

import backend.GameInfo;
import backend.Room;

final UnusedRoom: GameRoom = new Rooms_Unused();
class Rooms_Unused extends Room {
}

final EmptyRoom: GameRoom = new Rooms_Empty();
class Rooms_Empty extends Room {
}
