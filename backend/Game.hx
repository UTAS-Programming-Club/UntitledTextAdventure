package backend;

import backend.Campaign;
import backend.GameInfo;

class Game {
  public final campaign: Campaign;
  public var currentScreen(null, default): GameScreen;

  public function new(campaign: Campaign) {
    this.campaign = campaign;
    currentScreen = campaign.initialScreen;
  }

  public function getScreen(): Screen {
    if (!GameInfo.Screens.exists(currentScreen)) {
      throw 'Invalid screen $currentScreen.';
    }

    return cast GameInfo.Screens[currentScreen];
  }
}
