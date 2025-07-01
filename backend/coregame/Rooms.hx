package backend.coregame;

import backend.GameInfo;
import backend.Room;

final UnusedRoom: GameRoom = new CoreGame_Unused();
class CoreGame_Unused extends Room {
}

final EmptyRoom: GameRoom = new CoreGame_Empty();
class CoreGame_Empty extends Room {
}
