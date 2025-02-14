import game.generated.Generated;

// haxe --macro "TestMacros.GameGeneration.generateRoomsEnum()" --main Test --interp
// haxe --macro "TestMacros.GameGeneration.generateRoomsEnum()" --main Test --python test.py && python test.py

// With interp, this should give:
// Test.hx:11: characters 13-24 : Unmatched patterns: Chest1 | Chest2 | Test3 | TestA | TestB | Trap | Trap2
class Test {
  public static function main(): Void {
    #if interp
    switch (Rooms.Empty) {
      case Empty:
    }
    #else
    trace(Rooms);
    #end
  }
}
