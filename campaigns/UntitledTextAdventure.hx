package campaigns;

import backend.Campaign;
import backend.coregame.CoreGame;
import backend.coregame.Screens;
import backend.coregame.Rooms;
import backend.GameInfo;
import extensions.equipment.Equipment;
import extensions.equipment.EquipmentExtension;
import extensions.equipment.Rooms;
import extensions.healing.HealingExtension;
import extensions.healing.Rooms;
import extensions.trap.Rooms;
import extensions.trap.TrapExtension;

@:nullSafety(Strict)
final UntitledTextAdventure: Campaign = {
  mainMenu:   "Untitled text adventure game\n"
            + "----------------------------\n"
            + "By the UTAS Programming Club\n"
            + "\n"
            + "Currently unimplemented :(",

  extensions: [CoreGameExt, EquipmentExt, HealingExt, TrapExt],

  initialScreen: MainMenuScreen,

  rooms: {
    final gridSize: Int = 10;
    final rooms: Array<Array<GameRoom>> = [for (x in 0...gridSize) [for (y in 0...gridSize) new UnusedRoom()]];
    rooms[0][0] = new EmptyRoom();
    rooms[0][1] = new EmptyRoom();
    rooms[1][0] = new EmptyRoom();
    rooms[1][1] = new EmptyRoom();
    rooms[1][2] = new TrapRoom();
    rooms[1][3] = new ChestRoom(WeaponSword);
    rooms[2][1] = new HealingRoom();
    rooms;
  },
  initialRoomX: 0,
  initialRoomY: 0,

  equipmentOrder: [
    HeadNone,
    UpperBodyRags,
    HandsNone,
    LowerBodyRags,
    FeetNone,
    WeaponNone,
    WeaponFist,
    WeaponSword,
  ],
  initialHead: HeadNone,
  initialUpperBody: UpperBodyRags,
  initialHands: HandsNone,
  initialLowerBody: LowerBodyRags,
  initialFeet: FeetNone,
  initialPrimaryWeapon: WeaponFist,
  initialSecondaryWeapon: WeaponNone
};
