package campaigns;

import backend.Campaign;
import backend.coregame.CoreGame;
import backend.coregame.Rooms;
import backend.coregame.Screens;
import extensions.equipment.Equipment;
import extensions.equipment.EquipmentExtension;
import extensions.test.Rooms;
import extensions.test.TestExtension;

@:nullSafety(Strict)
final TestCampaign: Campaign = {
  mainMenu:   "A campaign example\n"
            + "------------------\n"
            + "By Joshua Wierenga",

  extensions: [CoreGameExt, EquipmentExt, TestExt],

  initialScreen: MainMenuScreen,

  rooms: [[TestRoom]],
  initialRoomX: 0,
  initialRoomY: 0,

  equipmentOrder: [
    HeadNone,
    UpperBodyRags,
    HandsNone,
    LowerBodyRags,
    FeetNone,
    WeaponFist,
    WeaponNone,
  ],
  initialHead: HeadNone,
  initialUpperBody: UpperBodyRags,
  initialHands: HandsNone,
  initialLowerBody: LowerBodyRags,
  initialFeet: FeetNone,
  initialPrimaryWeapon: WeaponFist,
  initialSecondaryWeapon: WeaponNone
};
