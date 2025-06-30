package extensions.rooms;

import haxe.Constraints;

import backend.Action;
import backend.Campaign;
import backend.coregame.Actions;
import backend.coregame.Screens;
import backend.Game;
import backend.GameInfo;
import backend.Room;
import backend.Screen;
import extensions.rooms.Actions;


final GameRoomsScreen: GameScreen = new Rooms_GameRooms();
@:nullSafety(Strict)
class Rooms_GameRooms extends StatefulActionScreen<GameRoomState> {
  function getStatefulBody(Game, state: GameRoomState): UnicodeString {
    return 'This is the game, you are in Room [${state.x + 1}, ${state.y + 1}].';
  }

  function getAllActions(): Array<Action> return [
    new GoNorth("Go North"),
    new GoEast("Go East"),
    new GoSouth("Go South"),
    new GoWest("Go West"),
    new GotoScreen(PlayerEquipmentScreen, "Check Inventory"),
#if testrooms
    new Quit("Quit Game")
#else
    new GotoScreen(MainMenuScreen, "Return to main menu")
#end
  ];
}

@:nullSafety(Strict)
class GameRoomState extends ScreenState {
  // Only modify these using changeRoom to ensure state is setup
  // For some reason ++ and possibly -- works despite disabling public assignment
  public var x(default, null): Int; // Must be in [0, campaign.rooms.length)
  public var y(default, null): Int; // Must be in [0, campaign.rooms.length)
  private var roomState: Map<Int, RoomState> = [];

  public function new(campaign: Campaign) {
    super();
    x = campaign.initialRoomX;
    y = campaign.initialRoomY;
  }

  // TODO: Call on first room appearing
  // x and y must be in [0, campaign.rooms.length)
  public function changeRoom(state: Game, x: Int, y: Int): Void {
    this.x = x;
    this.y = y;

    final point: Int = x * state.campaign.rooms.length + y;
    if (roomState.exists(point)) {
      return;
    }

    final room: GameRoom = state.campaign.rooms[x][y];
#if debuggame
    var roomExists: Bool = false;
    for (ext in state.campaign.extensions) {
      roomExists = roomExists || ext.rooms.contains(room);
      if (roomExists) {
        break;
      }
    }

    if (!roomExists) {
      throw ': Invalid room $room';
    }
#end

    if (!room.hasRoomState()) {
      return;
    }

    roomState[point] = room.createRoomState();
  }

  // x and y must be in [0, campaign.rooms.length)
  @:generic
  public function getRoomState<T : RoomState & Constructible<Void -> Void>>(state: Game, ?x: Int, ?y: Int): T {
    final xPos: Int = x ?? this.x;
    final yPos: Int = y ?? this.y;
    final point: Int = xPos * state.campaign.rooms.length + yPos;

    final room: GameRoom = state.campaign.rooms[xPos][yPos];
    final roomState: Null<RoomState> = roomState[point];
    if (!room.hasRoomState() || roomState == null) {
      throw ': Room $room at $xPos, $yPos does not have any stored state';
    }

#if debuggame
    final stateType: String = Type.getClassName(Type.getClass(roomState));
    final expectedState: T = new T();
    final expectedType: String = Type.getClassName(Type.getClass(expectedState));
    if (stateType != expectedType) {
      throw ': Incorrect result type $expectedType provided for room with type $stateType';
    }
#end

    return cast roomState;
  }
}
