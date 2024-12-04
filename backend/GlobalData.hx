package backend;

class GlobalData {
  static var initialised: Bool;
  public static var rooms(default, null): Array<Room>;
  
  public static function Init(newRooms: Array<Room>): Void {
    if (initialised) {
      return;
    }

    rooms = newRooms;
  }

  public static function Log(): Void {
    // Sys.print("Room {}");
    for (room in rooms) {
      Sys.print(room.type + ", " + room.x + ", " + room.y + "\n");
    }
  }
}
