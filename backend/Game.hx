package backend;

import backend.Campaign;
import backend.GameInfo;
import backend.Screen;
import haxe.Constraints;

class Game {
  public final campaign: Campaign;
  public var currentScreen(null, default): GameScreen;
  private var screenState: Map<GameScreen, ScreenState>;

  public function new(campaign: Campaign) {
    this.campaign = campaign;
    currentScreen = campaign.initialScreen;
    // TODO: Switch to only create stating when visited?
    // TODO: Decide if screens even need state
    screenState = [
      for (screen => info in GameInfo.Screens) {
        if (info is StatefulActionScreen) {
          final statefulScreen: StatefulActionScreen = cast info;
          screen => statefulScreen.stateConstructor(campaign);
        }
      }
    ];
  }

  public function getScreen(): Screen {
    final screen: Null<Screen> = GameInfo.Screens[currentScreen];
    if (screen == null) {
      throw 'Invalid screen $currentScreen.';
    }

    return screen;
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
