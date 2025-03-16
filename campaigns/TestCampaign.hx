package campaigns;

import backend.Campaign;
import backend.GameInfo;
import backend.coregame.CoreGame;
import extensions.test.Test;

@:nullSafety(Strict)
final TestCampaign: Campaign = {
  mainMenu:   "A campaign example\n"
            + "------------------\n"
            + "By Joshua Wierenga",

  extensions: [CoreGameExt, TestExt],

  initialScreen: MainMenu,
  gameScreen: Test
};
