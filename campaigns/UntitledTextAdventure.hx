package campaigns;

import backend.Campaign;
import backend.coregame.CoreGame;
import backend.GameInfo;
import extensions.equipment.EquipmentExtension;
import extensions.rooms.RoomsExtension;
import extensions.trap.TrapExtension;

@:nullSafety(Strict)
final UntitledTextAdventure: Campaign = {
  mainMenu:   "Untitled text adventure game\n"
            + "----------------------------\n"
            + "By the UTAS Programming Club\n"
            + "\n"
            + "Currently unimplemented :(",

  extensions: [CoreGameExt, EquipmentExt, RoomsExt, TrapExt],

#if testrooms
  initialScreen: GameRooms,
#else
  initialScreen: MainMenu,
#end
  // gameScreen: GameRooms,

  // rooms: {
  //   final gridSize: Int = 10;
  //   final rooms = [for (x in 0...gridSize) [for (y in 0...gridSize) Unused]];
  //   rooms[0][0] = Empty;
  //   rooms[0][1] = Empty;
  //   rooms[1][0] = Empty;
  //   rooms[1][1] = Empty;
  //   rooms[1][2] = Trap;
  //   rooms;
  // },
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
