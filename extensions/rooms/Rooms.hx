package extensions.rooms;

import backend.Room;

final UnusedRoom: Room = new Rooms_Unused();
class Rooms_Unused extends Room {
}

final EmptyRoom: Room = new Rooms_Empty();
class Rooms_Empty extends Room {
}
