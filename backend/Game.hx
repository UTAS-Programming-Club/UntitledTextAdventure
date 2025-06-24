package backend;

import backend.GameInfo;

import backend.BaseGame;
import backend.Screen;

class Game extends BaseGame {
  public function new() {
    // TODO: Avoid hardcoding campaign
    super(
     campaigns.untitledtextadventure.UntitledTextAdventure.UntitledTextAdventure,
    );
#if testrooms
    startGame();
#end
  }

  // TODO: Find out why moving this to BaseGame.hx causes "Class<backend.GameInfo> has not field Equipment" in Equipment.hx
  // TODO: Move room x, y to player class?
  public function startGame(): Void {
    gotoScreen(campaign.gameScreen);
    player.Reset(campaign);
    screenState = [
      for (screen => info in GameInfo.Screens) {
        if (info is StatefulActionScreen) {
          final statefulScreen: StatefulActionScreen = cast info;
          screen => statefulScreen.stateConstructor(campaign);
        }
      }
    ];
  }
}
