package backend;

import backend.Campaign;
import backend.GameInfo;

@:nullSafety(Strict)
class Game {
  public final campaign: Campaign;
  public var currentScreen(default, null): Null<Screen>;

  public function new(campaign: Campaign) {
    this.campaign = campaign;
    currentScreen = campaign.initialScreen;
  }
}
