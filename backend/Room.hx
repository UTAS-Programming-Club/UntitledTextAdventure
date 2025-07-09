package backend;

import haxe.Constraints;
using StringTools;
import sys.io.File;
import sys.io.FileOutput;

import backend.Action;
// TODO: Fix backend depending on coregame extension?
import backend.coregame.Actions;
import backend.coregame.Rooms;
import backend.coregame.Screens;
import backend.Game;
import backend.GameInfo;
import backend.Screen;

abstract class Room extends ActionScreen {
  function getRoomBody(state: Game): UnicodeString return '';

  // TODO: Move all the strings to extensions/campaigns
  function getBody(state: Game): UnicodeString {
    writeMap(state);

    final x: Int = state.player.x;
    final y: Int = state.player.y;
    final room: GameRoom = state.campaign.rooms[x][y];

    var body: UnicodeString = 'This is the game, you are in Room [${x + 1}, ${y + 1}].';
    body += getRoomBody(state);

    return body;
  }

  function getRoomActions(): Array<Action> return [];

  // TODO: Move all the actions to extensions/campaigns
  function getAllActions(): Array<Action> {
    final actions: Array<Action> = [
      new GoNorth('Go North'),
      new GoEast('Go East'),
      new GoSouth('Go South'),
      new GoWest('Go West'),
      // getRoomActions goes here
      new GotoScreen(PlayerEquipmentScreen, 'Check Inventory'),
#if testrooms
      new Quit('Quit Game')
#else
      new GotoScreen(MainMenuScreen, 'Return to main menu')
#end
    ];

    for (action in getRoomActions()) {
      actions.insert(4, action);
    }

    return actions;
  }

  public function hasState(): Bool return false;
  public function createState(): RoomState throw ': Room has no state';


  public static function getRoomID(state: Game, x: Int, y: Int): Int return y * state.campaign.rooms.length + x;


  // Each room take 6x4 but the 6(?) required calls to writeMapRoom per room only
  // handle the top left most 5x3 unless it is the right and/or bottom most room
  static final RoomSizeX: Int = 6;
  static final RoomSizeY: Int = 4;

  static final XLine: UnicodeString = "─";
  static final YLine: UnicodeString = "│";
  static final UpperHalfYLine: UnicodeString = "╵";
  static final LowerHalfYLine: UnicodeString = "╷";

  static final TopRowChars: Array<UnicodeString> =    ['┌', '┬', '┐'];
  static final MiddleRowChars: Array<UnicodeString> = ['├', '┼', '┤'];
  static final BottomRowChars: Array<UnicodeString> = ['└', '┴', '┘'];

  static function roomExists(state: Game, x: Int, y: Int): Bool {
    final floorSize: Int = state.campaign.rooms.length;
    if (x < 0 || y < 0 || x >= floorSize || y >= floorSize) {
      return false;
    }

    return !(state.campaign.rooms[x][y] is UnusedRoom);
  }

  static function getKnownRooms(state: Game): Array<Int> {
    var previousLength: Int;
    // Start with known stateful rooms
    final known: Array<Int> = [
      for (x in 0...state.campaign.rooms.length) {
        for (y in 0...state.campaign.rooms.length) {
          final room: Room = state.campaign.rooms[x][y];
          if (room.hasState()) {
            final roomState: Null<RoomState> = state.tryGetRoomState(x, y);
            if (roomState != null && roomState.isCompleted()) {
              getRoomID(state, x, y);
            }
          }
        }
      }
    ];

    // TODO: Remove once tracking visited rooms
    // Add starting room, duplicate doesn't matter
    final startingRoomID: Int = getRoomID(state, state.campaign.initialRoomX, state.campaign.initialRoomY);
    known.push(startingRoomID);

    // Add current room, duplicate doesn't matter
    final currentRoomID: Int = getRoomID(state, state.player.x, state.player.y);
    known.push(currentRoomID);

    // Add adjacent rooms until all surrounding rooms are uncompleted stateful rooms
    do {
      previousLength = known.length;
      for (x in 0...state.campaign.rooms.length) {
        for (y in 0...state.campaign.rooms.length) {
          final room: Room = state.campaign.rooms[x][y];
          final roomID = getRoomID(state, x, y);
          if (known.contains(roomID) || room is UnusedRoom || room.hasState()) {
            continue;
          }

          if (known.contains(getRoomID(state, x - 1, y)) ||
          known.contains(getRoomID(state, x + 1, y)) ||
          known.contains(getRoomID(state, x, y - 1)) ||
          known.contains(getRoomID(state, x, y + 1))) {
            known.push(roomID);
          }
        }
      }
    } while (known.length != previousLength);

    return known;
  }

  static function writeMapRoom(str: StringBuf, x: Int, y: Int, known: Bool, line: Int, state: Game) : Void {
    if (line == RoomSizeY - 1 && y != 0) {
      return;
    }

    // Top and bottom row lines
    if (line == 0 || line == RoomSizeY - 1) {
      var rowChars: Array<UnicodeString>;
      if (y == state.campaign.rooms.length - 1) {
        rowChars = TopRowChars;
      } else if (y > 0 || line == 0) {
        rowChars = MiddleRowChars;
      } else {
        rowChars = BottomRowChars;
      }

      if (x == 0) {
        str.add(rowChars[0]);
      } else {
        str.add(rowChars[1]);
      }

      if (line == 0 && roomExists(state, x, y + 1)) {
        str.add(XLine + ''.rpad(' ', RoomSizeX - 4) + XLine);
      } else {
        str.add(''.rpad(XLine, RoomSizeX - 2));
      }

      if (x == state.campaign.rooms.length - 1) {
        str.add(rowChars[2] + '\n');
      }

    // middle row lines
    } else {
      final roomExists: Bool = roomExists(state, x, y);

      var wallChar: UnicodeString;
      if (!roomExists || !Room.roomExists(state, x - 1, y)) {
        wallChar = YLine;
      } else if (line == 1) {
        wallChar = UpperHalfYLine;
      } else if (line == RoomSizeY - 2) {
        wallChar = LowerHalfYLine;
      } else {
        wallChar = ' ';
      }

      // TODO: Add specific output for all room types
      // The pads are all -1 to allow room for the wallChar on the right side
      if (line == 1) {
        // Player in room
        if (x == state.player.x && y == state.player.y) {
          str.add((wallChar + 'P').rpad(' ', RoomSizeX - 1));
        // Room has been completed
        } else if (roomExists && known) {
          str.add(wallChar.rpad(' ', RoomSizeX - 1));
        // Room exists but has not been visited
        } else if (roomExists) {
          str.add((wallChar + '?').rpad(' ', RoomSizeX - 1));
        // Room does not exists
        } else {
          str.add((wallChar + 'NO').rpad(' ', RoomSizeX - 1));
        }
      } else {
        str.add(wallChar.rpad(' ', RoomSizeX - 1));
      }

      if (x == state.campaign.rooms.length - 1) {
        str.add(YLine + '\n');
      }
    }
  }

  public static function createMap(state: Game): UnicodeString {
    final str: StringBuf = new StringBuf();
    final knownRooms: Array<Int> = getKnownRooms(state);

    for (flippedY in 0...state.campaign.rooms.length) {
      final y = state.campaign.rooms.length - flippedY - 1;
      for (line in 0...RoomSizeY) {
        for (x in 0...state.campaign.rooms.length) {
          final known: Bool = knownRooms.contains(getRoomID(state, x, y));
          writeMapRoom(str, x, y, known, line, state);
        }
      }
    }

    str.add('
P:  Player
?:  Unvisited
No: Non existent'
    );

    return str.toString();
  }

  static function writeMap(state: Game): Void {
#if debuggame
    final file: FileOutput = File.write('map.txt', false);
    file.writeString(createMap(state));
    file.close();
#end
  }
}

@:generic
abstract class StatefulRoom<T : RoomState & Constructible<Void -> Void>> extends Room {
  override function hasState(): Bool return true;
  override function createState(): T return new T();

  function getStatefulRoomBody(state: Game, roomState: T): UnicodeString return '';

  override function getRoomBody(state: Game): UnicodeString return getStatefulRoomBody(state, state.getRoomState());
}

abstract class RoomState {
  public function new() {
  }

  // Each stateful room must store at least one bool to track completion
  // which is used from saving and the map
  public abstract function isCompleted(): Bool;
  // Optionally the room can prevent exiting to any room except the one
  // used for entry to make sure the player completes the room
  public abstract function requireCompleted(): Bool;
}
