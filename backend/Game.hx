package backend;

using StringTools;
import haxe.Constraints;

import backend.Campaign;
import backend.macros.Helpers;
import backend.GameInfo;
import backend.Player;
import backend.Room;
import backend.Screen;

class Game {
  public final campaign: Campaign;
  public final player: Player;

  // Only access via getScreen
  private var currentScreen: GameScreen;
  public var previousRoom(default, null): Int = -1;
  public var previousScreen(default, null): GameScreen;
  private var roomState: Map<Int, RoomState> = [];

  public function new() {
    campaign = getCampaign();
    player = new Player(campaign);

    currentScreen = campaign.initialScreen;
    previousScreen = campaign.initialScreen;

#if debuggame
    // Extension class instance checks
    for (ext in campaign.extensions) {
      var errors: Bool = checkGameTypeDeclarations(ext, 'Outcome', ext.outcomes);
      errors = checkGameTypeDeclarations(ext, 'Room', ext.rooms) || errors;
      errors = checkGameTypeDeclarations(ext, 'Screen', ext.screens) || errors;
      if (errors) {
        throw ': Please fix extension type declarations to continue';
      }

      // TODO: Find way to check equipment
      // They are all just instances of backend.Equipment so Std.string and Type.* aren't useful
    }

    // Campaign equipment instance checks
    var equipment: Array<GameEquipment> = [];
    for (ext in campaign.extensions) {
      equipment = equipment.concat(ext.equipment);
    }
    for (item in campaign.equipmentOrder) {
      if (!equipment.contains(item)) {
        throw ': Invalid item ${item.type}:${item.name} not declared in loaded extensions';
      }
    }
#end
    checkScreen(campaign.initialScreen);
#if testrooms
    startGame();
#end
  }

  public function startGame(): Void {
    player.reset(campaign);
    gotoRoom(campaign.initialRoomX, campaign.initialRoomY);
    roomState = [];
  }


// TODO: Figure out whis this fails with buildstatic.hxml
#if debuggame
  private function checkGameTypeDeclarations<T>(ext: Extension, type: UnicodeString, declaredItems: Array<T>): Bool {
    var errors = false;
    for (item in declaredItems) {
      final itemString: UnicodeString = Std.string(item).replace('Class<', '').replace('>', '');
      final itemName: Null<UnicodeString> = itemString.split(".").pop();
      if (itemName == null) {
        throw 'Internal error';
      }

      final itemModule: UnicodeString = itemString.substring(0, itemString.length - itemName.length - 1);
      if (ext.module != itemModule) {
        final itemDisplayName: UnicodeString = itemName.split('_').pop() ?? itemName;
        trace('$type $itemDisplayName declared in incorrect extension ${ext.module}');
        errors = true;
      }
    }

    return errors;
  }
#end

  private function checkScreen(screen: GameScreen): Void {
#if debuggame
    if (screen is GameRoom) {
      final type: Class<Room> = cast Type.getClass(screen);
      for (ext in campaign.extensions) {
        if (ext.rooms.contains(type)) {
          return;
        }
      }
    } else {
      for (ext in campaign.extensions) {
        if (ext.screens.contains(screen)) {
          return;
        }
      }
    }

    throw ': Invalid screen $screen';
#end
  }


  public function getScreen(): Screen {
    checkScreen(currentScreen);
    return currentScreen;
  }

  public function gotoScreen(newScreen: GameScreen): Void {
    checkScreen(newScreen);
    previousScreen = currentScreen;
    currentScreen = newScreen;
  }


  // x and y must be in [0, campaign.rooms.length)
  public function gotoRoom(x: Int, y: Int): Void {
    previousRoom = Room.getRoomID(this, player.x, player.y);
    player.changeRoom(campaign, x, y);

    final room: GameRoom = campaign.rooms[x][y];
    gotoScreen(room);

    final point: Int = Room.getRoomID(this, x, y);
    if (roomState.exists(point)) {
      return;
    }

#if debuggame
    final type: Class<Room> = Type.getClass(room);
    var roomExists: Bool = false;
    for (ext in campaign.extensions) {
      roomExists = roomExists || ext.rooms.contains(type);
      if (roomExists) {
        break;
      }
    }

    if (!roomExists) {
      throw ': Invalid room $room';
    }
#end

    if (!room.hasState()) {
      return;
    }

    roomState[point] = cast room.createState();
  }

  // x and y must be in [0, campaign.rooms.length)
  // TODO: Fix "[0] Instance constructor not found: T" when calling generic function from generic function
  // Constructible appears to be ignored at the second level
#if false // debuggame
  @:generic
#end
  public function tryGetRoomState<T : RoomState & Constructible<Void -> Void>>(?x: Int, ?y: Int): Null<T> {
    final xPos: Int = x ?? player.x;
    final yPos: Int = y ?? player.y;
    final point: Int = Room.getRoomID(this, xPos, yPos);

    final room: GameRoom = campaign.rooms[xPos][yPos];
    final roomState: Null<RoomState> = roomState[point];
    if (!room.hasState() || roomState == null) {
      return null;
    }

#if false // debuggame
    final stateType: String = Type.getClassName(Type.getClass(roomState));
    final expectedState: T = new T();
    final expectedType: String = Type.getClassName(Type.getClass(expectedState));
    if (stateType != expectedType) {
      throw ': Incorrect result type $expectedType provided for room with type $stateType';
    }
#end

    return cast roomState;
  }

  // x and y must be in [0, campaign.rooms.length)
  // TODO: Fix "[0] Instance constructor not found: T" when calling generic function from generic function
  // Constructible appears to be ignored at the second level
#if false // debuggame
  @:generic
#end
  public function getRoomState<T : RoomState & Constructible<Void -> Void>>(?x: Int, ?y: Int): T {
    final xPos: Int = x ?? player.x;
    final yPos: Int = y ?? player.y;
    final point: Int = Room.getRoomID(this, xPos, yPos);

    final room: GameRoom = campaign.rooms[xPos][yPos];
    final roomState: Null<RoomState> = roomState[point];
    if (!room.hasState() || roomState == null) {
      throw ': Room $room at $xPos, $yPos does not have any stored state';
    }

#if false // debuggame
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
