package backend;

import backend.Campaign;
import backend.Equipment;
import backend.GameEnums;
import backend.Player;
import backend.Room;
import backend.Screen;
import haxe.Constraints;

// This class is only seperate from Game to allow accessing GameInfo's maps
// That incldues the references passed via the constructor and the startGame
// function which somehow allows Equipment.hx to access GameInfo as well.
abstract class BaseGame {
  public final campaign: Campaign;
  public final player: Player;

  public final equipment: Map<GameEquipment, Equipment>;
  public final rooms: Map<GameRoom, Void -> Room>;
  public final screens: Map<GameScreen, Screen>;

  public var previousScreen(default, null): GameScreen;
  private var currentScreen: GameScreen;
  private var screenState: Map<GameScreen, ScreenState>;

  public function new(
    campaign: Campaign, equipment: Map<GameEquipment, Equipment>,
    rooms: Map<GameRoom, Void -> Room>, screens: Map<GameScreen, Screen>
  ) {
    this.campaign = campaign;
    player = new Player(campaign);

    this.equipment = equipment;
    this.rooms = rooms;
    this.screens = screens;

    previousScreen = campaign.initialScreen;
    currentScreen = campaign.initialScreen;
    // No screen may store state before the game starts
    screenState = [];
  }

  // TODO: Move room x, y to player class?
  abstract public function startGame(): Void;


  public function getScreen(): Screen {
    final screen: Null<Screen> = screens[currentScreen];
    if (screen == null) {
      throw 'Invalid screen $currentScreen.';
    }

    return screen;
  }

  public function gotoScreen(newScreen: GameScreen): Void {
    previousScreen = currentScreen;
    currentScreen = newScreen;
  }


  @:generic
  public function tryGetScreenState<T : ScreenState & Constructible<Campaign -> Void>>(): Null<T> {
    final screenData: Null<ScreenState> = screenState[currentScreen];
    final screen: T = new T(campaign);
    if (screenData == null) {
      return null;
    }

    final screenDataType: String = Type.getClassName(Type.getClass(screenData));
    final screenType: String = Type.getClassName(Type.getClass(screen));
    if (screenDataType != screenType) {
      throw 'Incorrect result type $screenType provided for screen with type $screenDataType in tryGetScreenState.';
    }

    return cast screenData;
  }

  // For some reason, using the result of tryGetScreenState gives "[1] Instance constructor not found: getScreenState.T"
  @:generic
  public function getScreenState<T : ScreenState & Constructible<Campaign -> Void>>(): T {
    final screenData: Null<ScreenState> = screenState[currentScreen];
    final screen: T = new T(campaign);
    if (screenData == null) {
      throw 'Screen $currentScreen does not have any stored state.';
    }

    final screenDataType: String = Type.getClassName(Type.getClass(screenData));
    final screenType: String = Type.getClassName(Type.getClass(screen));
    if (screenDataType != screenType) {
      throw 'Incorrect result type $screenType provided for screen with type $screenDataType in getScreenState.';
    }

    return cast screenData;
  }
}
