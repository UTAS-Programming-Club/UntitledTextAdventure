package campaigns;

import backend.Campaign;
import backend.coregame.CoreGame;
import backend.coregame.Screens;
// import extensions.equipment.EquipmentExtension;
import extensions.rooms.RoomsExtension;
import extensions.rooms.Screens;
// import extensions.trap.TrapExtension;
// import haxe.Constraints;

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
  initialScreen: GameRooms,
#else
  initialScreen: MainMenu.new,
#end
  gameScreen: GameRooms.new,

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

// V4 option 4
/*abstract class RoomState {
  public function new() {
  }
}
abstract class Room {
  public function hasState(): Bool {
    return false;
  }
  
  public function createState(): Constructible<Void -> Void> {
    throw 'Room has no state';
  }
}
@:generic
abstract class StatefulRoom<T : Constructible<Void -> Void>> extends Room {
  public function new() {
  }

  public override function hasState(): Bool {
    return true;
  }
  
  public override function createState(): T {
    return new T();
  }
}

class ChestRoomState extends RoomState {
  public var opened: Bool = false;
}
@:structInit class Chest extends StatefulRoom<ChestRoomState> {
  public final item: GameEquipment;
}

@:structInit class Empty extends Room {
}*/
