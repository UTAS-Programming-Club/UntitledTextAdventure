package campaigns;

import backend.Campaign;
import backend.coregame.CoreGame;
import backend.GameInfo;
import extensions.equipment.EquipmentExtension;
import extensions.test.TestExtension;

@:nullSafety(Strict)
final TestCampaign: Campaign = {
  mainMenu:   "A campaign example\n"
            + "------------------\n"
            + "By Joshua Wierenga",

  extensions: [CoreGameExt, EquipmentExt, TestExt],

  initialScreen: MainMenu,
  gameScreen: Test,

  rooms: [[Empty]],
  initialRoomX: 0,
  initialRoomY: 0,

  initialHead: HeadNone,
  initialUpperBody: UpperBodyRags,
  initialHands: HandsNone,
  initialLowerBody: LowerBodyRags,
  initialFeet: FeetNone,
  initialPrimaryWeapon: PrimaryWeaponFist,
  initialSecondaryWeapon: SecondaryWeaponNone
};
