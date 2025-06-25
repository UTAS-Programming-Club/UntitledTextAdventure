package backend;

import backend.Campaign;
import backend.GameInfo;
import backend.Player;
import backend.Screen;
// import haxe.Constraints;

class Game {
  public final campaign: Campaign;
  public final player: Player;

  private var currentScreen: Screen;
  public var previousScreen(default, null): Screen;
  // private var screenState: Map<GameScreen, ScreenState>;

  public function new(campaign: Campaign) {
    this.campaign = campaign;
    // TODO: Find a better way to share instances?
    previousScreen = currentScreen = campaign.initialScreen();
    player = new Player(campaign);
    // No screen may store state before the game starts
    // screenState = [];
#if testrooms
    startGame();
#end
  }

  // TODO: Move room x, y to player class?
  public function startGame(): Void {
    throw 'Not implemented';
    // gotoScreen(campaign.gameScreen);
    // player.Reset(campaign);
    // screenState = [
    //   for (screen => info in GameInfo.Screens) {
    //     if (info is StatefulActionScreen) {
    //       final statefulScreen: StatefulActionScreen = cast info;
    //       screen => statefulScreen.stateConstructor(campaign);
    //     }
    //   }
    // ];
  }


  public function getScreen(): Screen {
#if debug
    var screenExists: Bool = false;
    for (extension in campaign.extensions) {
      for (screen in extension.screens) {
        screenExists = screenExists || Type.getClass(currentScreen) == screen.type;
      }
    }

    if (!screenExists) {
      throw 'Invalid screen $currentScreen.';
    }
#end

    return currentScreen;
  }

  public function gotoScreen(newScreen: Screen): Void {
    previousScreen = currentScreen;
    currentScreen = newScreen;
  }


  /*@:generic
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
  }*/
}
