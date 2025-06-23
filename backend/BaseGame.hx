package backend;

import backend.Campaign;
import backend.GameEnums;
import backend.Player;
import backend.Screen;
import haxe.Constraints;

abstract class BaseGame {
  public final campaign: Campaign;
  public final player: Player;

  public var previousScreen(default, null): GameScreen;
  private var currentScreen: GameScreen;
  private var screenState: Map<GameScreen, ScreenState>;

  public function new(campaign: Campaign) {
    this.campaign = campaign;
    previousScreen = campaign.initialScreen;
    currentScreen = campaign.initialScreen;
    player = new Player(campaign);
    // No screen may store state before the game starts
    screenState = [];
  }

  // TODO: Move room x, y to player class?
  abstract public function startGame(): Void;


  abstract public function getScreen(): Screen;

  abstract public function gotoScreen(newScreen: GameScreen): Void;


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
