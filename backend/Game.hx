package backend;

using StringTools;
import haxe.Constraints;

import backend.Campaign;
import backend.macros.Helpers;
import backend.GameInfo;
import backend.Player;
import backend.Screen;

class Game {
  public final campaign: Campaign;
  public final player: Player;

  // Only access via getScreen
  private var currentScreen: GameScreen;
  public var previousScreen(default, null): GameScreen;
  private var screenState: Map<Screen, ScreenState>;

  public function new() {
    campaign = getCampaign();
    player = new Player(campaign);

    currentScreen = campaign.initialScreen;
    previousScreen = campaign.initialScreen;
    // No screen may store state before the game starts
    screenState = [];

#if debuggame
    for (ext in campaign.extensions) {
      var errors: Bool = checkGameTypeDeclarations(ext, "Outcome", ext.outcomes);
      errors = checkGameTypeDeclarations(ext, "Screen", ext.screens) || errors;
      if (errors) {
        throw ': Please fix extension type declarations to continue';
      }
    }
#end
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
      for (ext in campaign.extensions) {
        for (screen in ext.screens) {
          if (screen.hasState()) {
            screen => screen.createState(campaign);
          }
        }
      }
    ];
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
        trace('$type $itemDisplayName declared in incorrect extension ${ext.module}.');
        errors = true;
      }
    }

    return errors;
  }
#end

  private function checkScreen(screen: GameScreen): Void {
#if debuggame
    for (ext in campaign.extensions) {
      if (ext.screens.contains(screen)) {
        return;
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


  /*@:generic
  public function tryGetScreenState<T : ScreenState & Constructible<Campaign -> Void>>(): Null<T> {
    final screen: Screen = getScreen();
    final screenState: Null<ScreenState> = screenState[currentScreen];
    if (!screen.hasState() || screenState == null) {
      return null;
    }

    // TODO: Fix "[1] Instance constructor not found: T" when calling generic function from generic function
    // Constructible appears to be ignored at the second level
#if false // debuggame
    final stateType: String = Type.getClassName(Type.getClass(screenState));
    final expectedState: T = new T(campaign);
    final expectedType: String = Type.getClassName(Type.getClass(expectedState));
    if (stateType != expectedType) {
      throw ': Incorrect state type $expectedType provided for screen with type $stateType';
    }
#end

    return cast screenState;
  }*/

  @:generic
  public function getScreenState<T : ScreenState & Constructible<Campaign -> Void>>(): T {
    final screen: Screen = getScreen();
    final screenState: Null<ScreenState> = screenState[currentScreen];
    if (!screen.hasState() || screenState == null) {
      throw ': Screen $currentScreen does not have any stored state';
    }

    // TODO: Fix "[1] Instance constructor not found: T" when calling generic function from generic function
    // Constructible appears to be ignored at the second level
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
}
