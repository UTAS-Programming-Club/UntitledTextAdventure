package backend;

import backend.Campaign;
import backend.GameInfo;

@:nullSafety(Strict)
class Game {
  public var currentScreen(default, null): Null<Screen>;

  public function new(campaign: Campaign) {
    currentScreen = campaign.initialScreen;
  }
}
