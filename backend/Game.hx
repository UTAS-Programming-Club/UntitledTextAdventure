package backend;

import backend.Campaign;
import backend.GameInfo;
import backend.Player;
import backend.Screen;
import haxe.Constraints;

class Game {
  public final campaign: Campaign;
  public final player: Player;

  // Only access via getScreen
  private var currentScreen: GameScreen;
  public var previousScreen(default, null): GameScreen;
  private var screenState: Map<Screen, ScreenState>;

  public function new(campaign: Campaign) {
  // TODO: Restore check
/*#if debug
    for (extension in campaign.extensions) {
      for (screen in extension.screens) {
        if (extension != screen.ext) {
          throw 'Incorrect provided extension ${screen.ext} for screen $screen in $extension';
        }
      }
    }
#end*/

    this.campaign = campaign;
    player = new Player(campaign);

    currentScreen = campaign.initialScreen;
    previousScreen = campaign.initialScreen;
    // No screen may store state before the game starts
    screenState = [];

    checkScreen(campaign.initialScreen);
#if testrooms
    startGame();
#end
  }

  // TODO: Move room x, y to player class?
  public function startGame(): Void {
    gotoScreen(campaign.gameScreen);
    player.Reset(campaign);
    screenState = [
      for (extension in campaign.extensions) {
        for (screen in extension.screens) {
          if (screen.hasState()) {
            screen => screen.createState(campaign);
          }
        }
      }
    ];
  }

  private function checkScreen(screen: GameScreen): Void {
#if debug
    for (ext in campaign.extensions) {
      if (ext.screens.contains(screen)) {
        return;
      }
    }

    throw 'Invalid screen $screen.';
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


  /*@:generic
  public function tryGetScreenState<T : ScreenState & Constructible<Campaign -> Void>>(): Null<T> {
    final screen: Screen = getScreen();
    final screenState: Null<ScreenState> = screenState[currentScreen];
    if (!screen.hasState() || screenState == null) {
    if (!screen.hasState() || extState == null ||  screenState == null) {
      return null;
    }

    // TODO: Fix "[1] Instance constructor not found: T" when calling generic function from generic function
    // Constructible appears to be ignored at the second level
#if false // debug
    final stateType: String = Type.getClassName(Type.getClass(screenState));
    final expectedState: T = new T(campaign);
    final expectedType: String = Type.getClassName(Type.getClass(expectedState));
    if (stateType != expectedType) {
      throw 'Incorrect state type $expectedType provided for screen with type $stateType.';
    }
#end

    return cast screenState;
  }*/

  @:generic
  public function getScreenState<T : ScreenState & Constructible<Campaign -> Void>>(): T {
    final screen: Screen = getScreen();
    final screenState: Null<ScreenState> = screenState[currentScreen];
    if (!screen.hasState() || screenState == null) {
      throw 'Screen $currentScreen does not have any stored state.';
    }

    // TODO: Fix "[1] Instance constructor not found: T" when calling generic function from generic function
    // Constructible appears to be ignored at the second level
#if false // debug
    final stateType: String = Type.getClassName(Type.getClass(screenState));
    final expectedState: T = new T(campaign);
    final expectedType: String = Type.getClassName(Type.getClass(expectedState));
    if (stateType != expectedType) {
      throw 'Incorrect state type $expectedType provided for screen with type $stateType.';
    }
#end

    return cast screenState;
  }
}
