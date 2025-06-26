package campaigns;

import backend.Campaign;
import backend.coregame.CoreGame;
import backend.coregame.Screens;
// import extensions.equipment.EquipmentExtension;
import extensions.rooms.RoomsExtension;
import extensions.rooms.Screens;
// import extensions.trap.TrapExtension;

import backend.Room;

@:nullSafety(Strict)
final UntitledTextAdventure: Campaign = {
  mainMenu:   "Untitled text adventure game\n"
            + "----------------------------\n"
            + "By the UTAS Programming Club\n"
            + "\n"
            + "Currently unimplemented :(",

  extensions: [CoreGameExt/*, EquipmentExt*/, RoomsExt/*, TrapExt*/],

#if testrooms
  initialScreen: {ext: RoomsExt, screen: GameRoomsScreen},
#else
  initialScreen: {ext: CoreGameExt, screen: MainMenuScreen},
#end
  gameScreen: {ext: RoomsExt, screen: GameRoomsScreen},

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
