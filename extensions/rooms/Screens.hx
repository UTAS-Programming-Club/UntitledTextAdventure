package extensions.rooms;

import backend.Game;
import backend.GameInfo;
import backend.macros.Helpers;
import backend.Room;
import backend.Screen;
import haxe.Constraints;

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

  @:generic
  public function getRoomState<T : Room & Constructible<Void -> Void>>(state: Game, x: UInt, y: UInt): T {
    final point: UInt = x * state.campaign.rooms.length + y;

    final roomData: Null<backend.Room> = roomState[point];
    final room: T = new T();
    if (roomData == null) {
      throw 'Room at $x, $y does not have any stored state.';
    }

    final roomDataType: String = Type.getClassName(Type.getClass(roomData));
    final roomType: String = Type.getClassName(Type.getClass(room));
    if (roomDataType != roomType) {
      throw 'Incorrect result type $roomType provided for room with type $roomDataType in getRoomState.';
    }

    return cast roomData;
  }
}
