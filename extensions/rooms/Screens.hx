package extensions.rooms;

import backend.Game;
import backend.GameInfo;
import backend.macros.Helpers;
import backend.Room;
import backend.Screen;

enum RoomsScreen {
  GameRooms;
}

@:nullSafety(Strict)
class GameRoomScreen extends ActionScreen {
  public var x(default, null): UInt;
  public var y(default, null): UInt;
  var roomState: Map<UInt, Room> = [];

  public function new(updateState: OneOf<UnicodeString, (Game, Screen) -> UnicodeString>,
                      ?actions: Array<ScreenAction>,
                      x: UInt, y: UInt) {
    super(updateState, actions);
    this.x = x;
    this.y = y;
  }

  public function changeRoom(state: Game, x: UInt, y: UInt): Void {
    this.x = x;
    this.y = y;

    final point: UInt = x * state.campaign.rooms.length + y;
    if (roomState.exists(point)) {
      return;
    }

    final room: GameRoom = state.campaign.rooms[x][y];
    if (!GameInfo.Rooms.exists(room)) {
      return;
    }

    final roomClass: Void -> Room = cast GameInfo.Rooms[room];
    roomState[point] = roomClass();
  }

  // TODO: Find a way to avoid roomConstructor parameter
  public function getRoomState<T : Room>(state: Game, x: UInt, y: UInt, roomConstructor: Void -> T): T {
    final point: UInt = x * state.campaign.rooms.length + y;

    final roomData: Null<backend.Room> = roomState[point];
    final room: backend.Room = roomConstructor();

    final roomDataType: String = Std.string(roomData);
    final roomType: String = Std.string(room);
    if (roomDataType != roomType) {
      throw 'Incorrect result type $roomType provided for room with type $roomDataType in getRoomState.';
    }

    return cast roomData;
  }
}
