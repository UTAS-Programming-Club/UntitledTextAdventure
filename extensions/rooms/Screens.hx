package extensions.rooms;

import backend.Campaign;
import backend.GameInfo;
import backend.macros.Helpers;
import backend.Room;
import backend.Screen;
import haxe.Constraints;

enum RoomsScreen {
  GameRooms;
}

@:nullSafety(Strict)
class GameRoomState extends ScreenState {
  // Only modify these using changeRoom to ensure state is setup
  // For some reason ++ and possibly -- works despite disabling public assignment
  public var x(default, null): Int; // Must be in [0, campaign.rooms.length)
  public var y(default, null): Int; // Must be in [0, campaign.rooms.length)
  var roomState: Map<Int, Room> = [];

  public function new(campaign: Campaign) {
    x = campaign.initialRoomX;
    y = campaign.initialRoomY;
  }

  // TODO: Call on first room appearing
  // x and y must be in [0, campaign.rooms.length)
  /*public function changeRoom(state: Game, x: Int, y: Int): Void {
    this.x = x;
    this.y = y;

    final point: Int = x * state.campaign.rooms.length + y;
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

  // x and y must be in [0, campaign.rooms.length)
  @:generic
  public function getRoomState<T : Room & Constructible<Void -> Void>>(state: Game, ?x: Int, ?y: Int): T {
    final xPos: Int = x ?? this.x;
    final yPos: Int = y ?? this.y;

    final point: Int = xPos * state.campaign.rooms.length + yPos;

    final roomData: Null<backend.Room> = roomState[point];
    final room: T = new T();
    if (roomData == null) {
      throw 'Room at $xPos, $yPos does not have any stored state.';
    }

    final roomDataType: String = Type.getClassName(Type.getClass(roomData));
    final roomType: String = Type.getClassName(Type.getClass(room));
    if (roomDataType != roomType) {
      throw 'Incorrect result type $roomType provided for room with type $roomDataType in getRoomState.';
    }

    return cast roomData;
  }*/
}
