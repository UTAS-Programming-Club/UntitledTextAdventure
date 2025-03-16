package campaigns;

import backend.Campaign;
import backend.coregame.CoreGame;
import backend.GameInfo;
import extensions.test.TestExtension;

@:nullSafety(Strict)
final TestCampaign: Campaign = {
  mainMenu:   "A campaign example\n"
            + "------------------\n"
            + "By Joshua Wierenga",

  extensions: [CoreGameExt, TestExt],

  initialScreen: MainMenu,
  gameScreen: Test,

  rooms: [[Empty]]
};
