package backend;

import backend.BaseGame;
import backend.Campaign;
import backend.GameEnums;
import backend.GameInfo;
import backend.Player;
import backend.Screen;
import haxe.Constraints;

class Game extends BaseGame {
  public function new() {
    super(
     campaigns.UntitledTextAdventure.UntitledTextAdventure,
     GameInfo.Equipment, GameInfo.Rooms, GameInfo.Screens
    );
#if testrooms
    startGame();
#end
  }

  // TODO: Move room x, y to player class?
  public function startGame(): Void {
    gotoScreen(campaign.gameScreen);
    player.Reset(campaign);
    screenState = [
      for (screen => info in screens) {
        if (info is StatefulActionScreen) {
          final statefulScreen: StatefulActionScreen = cast info;
          screen => statefulScreen.stateConstructor(campaign);
        }
      }
    ];
  }


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
}
