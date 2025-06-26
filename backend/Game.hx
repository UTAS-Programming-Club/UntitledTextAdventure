package backend;

import backend.Campaign;
import backend.GameInfo;
import backend.Player;
import backend.Screen;
import haxe.Constraints;

class Game {
  public final campaign: Campaign;
  public final player: Player;

  private var currentScreen: GameScreen;
  public var previousScreen(default, null): GameScreen;
  private var screenState: Map<Extension, Map<EnumValue, ScreenState>>;

  public function new(campaign: Campaign) {
    this.campaign = campaign;
    player = new Player(campaign);

    currentScreen = campaign.initialScreen;
    previousScreen = campaign.initialScreen;
    // No screen may store state before the game starts
    screenState = [];

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
        extension => [
          for (screen => screenObj in extension.screens) {
            if (screenObj.hasState()) {
              screen => screenObj.createState(campaign);
            }
          }
        ];
      }
    ];
  }


  public function getScreen(): Screen {
    final screen: Null<Screen> = currentScreen.ext.screens[currentScreen.screen];
    if (screen != null) {
      return screen;
    }

    throw 'Invalid screen $currentScreen.';
  }

  public function gotoScreen(newScreen: GameScreen): Void {
    previousScreen = currentScreen;
    currentScreen = newScreen;

#if debug
    if (currentScreen.ext.screens[currentScreen.screen] != null) {
      return;
    }

    throw 'Invalid screen $currentScreen.';
#end
  }


  /*@:generic
  public function tryGetScreenState<T : ScreenState & Constructible<Campaign -> Void>>(): Null<T> {
    final screen: Screen = getScreen();
    final extState: Null<Map<EnumValue, ScreenState>> = screenState[currentScreen.ext];
    final screenState: Null<ScreenState> = extState != null ? extState[currentScreen.screen] : null;
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
    final extState: Null<Map<EnumValue, ScreenState>> = screenState[currentScreen.ext];
    final screenState: Null<ScreenState> = extState != null ? extState[currentScreen.screen] : null;
    if (!screen.hasState() || extState == null ||  screenState == null) {
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
