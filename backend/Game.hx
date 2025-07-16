package backend;

using StringTools;
import haxe.Constraints;

import backend.Campaign;
import backend.macros.Helpers;
import backend.GameInfo;
import backend.Player;
import backend.Save;
import backend.Screen;

class Game {
  public final campaign: Campaign;
  public final player: Player;

  // Only access via getScreen
  private var currentScreen: GameScreen;
  public var previousScreen(default, null): GameScreen;
    // No screen may store state before the game starts
  private var screenState: Map<GameScreen, ScreenState> = [];
  private var roomState: Map<Int, ScreenState> = [];

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
    screenState = [
      for (ext in campaign.extensions) {
        for (screen in ext.screens) {
          if (screen.hasState()) {
            screen => screen.createState();
          }
        }
      }
    ];
    roomState = [];
  }

  public function loadGame(str: UnicodeString): Void {
    if (!Load(this, str)) {
      return;
    }

    gotoRoom(player.x, player.y);
    screenState = [
      for (ext in campaign.extensions) {
        for (screen in ext.screens) {
          if (screen.hasState()) {
            screen => screen.createState();
          }
        }
      }
    ];
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
        throw ': Internal error';
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


  // TODO: Fix "[1] Instance constructor not found: T" when calling generic function from generic function
  // Constructible appears to be ignored at the second level
#if false // debuggame
  @:generic
#end
  public function tryGetScreenState<T : ScreenState & Constructible<Campaign -> Void>>(): Null<T> {
    final screen: Screen = getScreen();
    final screenState: Null<ScreenState> = screenState[currentScreen];
    if (!screen.hasState() || screenState == null) {
      return null;
    }

#if false // debuggame
    final stateType: String = Type.getClassName(Type.getClass(screenState));
    final expectedState: T = new T(campaign);
    final expectedType: String = Type.getClassName(Type.getClass(expectedState));
    if (stateType != expectedType) {
      throw ': Incorrect state type $expectedType provided for screen with type $stateType';
    }
#end

    return cast screenState;
  }

  // TODO: Fix "[1] Instance constructor not found: T" when calling generic function from generic function
  // Constructible appears to be ignored at the second level
#if false // debuggame
  @:generic
#end
  public function getScreenState<T : ScreenState & Constructible<Void -> Void>>(): T {
    final screen: Screen = getScreen();
    final screenState: Null<ScreenState> = screenState[currentScreen];
    if (!screen.hasState() || screenState == null) {
      throw ': Screen $currentScreen does not have any stored state';
    }

#if false // debuggame
    final stateType: String = Type.getClassName(Type.getClass(screenState));
    final expectedState: T = new T(campaign);
    final expectedType: String = Type.getClassName(Type.getClass(expectedState));
    if (stateType != expectedType) {
      throw ': Incorrect state type $expectedType provided for screen with type $stateType';
    }
#end

    return cast screenState;
  }


  // x and y must be in [0, campaign.rooms.length)
  public function gotoRoom(x: Int, y: Int): Void {
    player.changeRoom(campaign, x, y);

    final room: GameRoom = campaign.rooms[x][y];
    gotoScreen(room);

    final point: Int = x * campaign.rooms.length + y;
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

    roomState[point] = room.createState();
  }

  // x and y must be in [0, campaign.rooms.length)
  // TODO: Fix "[0] Instance constructor not found: T" when calling generic function from generic function
  // Constructible appears to be ignored at the second level
#if false // debuggame
  @:generic
#end
  public function getRoomState<T : ScreenState & Constructible<Void -> Void>>(?x: Int, ?y: Int): T {
    final xPos: Int = x ?? player.x;
    final yPos: Int = y ?? player.y;
    final point: Int = xPos * campaign.rooms.length + yPos;

    final room: GameRoom = campaign.rooms[xPos][yPos];
    final roomState: Null<ScreenState> = roomState[point];
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
