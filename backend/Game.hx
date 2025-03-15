package backend;

import backend.Campaign;
import backend.GameInfo;
import haxe.Exception;

class Game {
  public final campaign: Campaign;
  public var currentScreen(null, default): GameScreen;

  public function new(campaign: Campaign) {
    this.campaign = campaign;
    currentScreen = campaign.initialScreen;
  }

  public function getScreen(): Screen {
    if (!GameInfo.Screens.exists(currentScreen)) {
      throw new Exception("Invalid current screen: " + currentScreen);
    }

    return cast GameInfo.Screens[currentScreen];
  }
}
