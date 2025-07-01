package campaigns;

import backend.Campaign;
import backend.coregame.CoreGame;
import backend.coregame.Screens;
import backend.coregame.Rooms;
import extensions.equipment.EquipmentExtension;
import extensions.trap.Rooms;
import extensions.trap.TrapExtension;

@:nullSafety(Strict)
final UntitledTextAdventure: Campaign = {
  mainMenu:   "Untitled text adventure game\n"
            + "----------------------------\n"
            + "By the UTAS Programming Club\n"
            + "\n"
            + "Currently unimplemented :(",

  extensions: [CoreGameExt, EquipmentExt, TrapExt],

#if testrooms
  initialScreen: GameRoomsScreen,
#else
  initialScreen: MainMenuScreen,
#end

  rooms: {
    final gridSize: Int = 10;
    final rooms = [for (x in 0...gridSize) [for (y in 0...gridSize) UnusedRoom]];
    rooms[0][0] = EmptyRoom;
    rooms[0][1] = EmptyRoom;
    rooms[1][0] = EmptyRoom;
    rooms[1][1] = EmptyRoom;
    rooms[1][2] = TrapRoom;
    rooms;
  },
  initialRoomX: 0,
  initialRoomY: 0,

  // initialHead: HeadNone,
  // initialUpperBody: UpperBodyRags,
  // initialHands: HandsNone,
  // initialLowerBody: LowerBodyRags,
  // initialFeet: FeetNone,
  // initialPrimaryWeapon: PrimaryWeaponFist,
  // initialSecondaryWeapon: SecondaryWeaponNone
};
