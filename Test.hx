import game.generated.Generated;

// haxe --macro "TestMacros.GameGeneration.generateRoomsEnum()" --main Test --interp
// haxe --macro "TestMacros.GameGeneration.generateRoomsEnum()" --main Test --python test.py && python test.py

// Should give:
// Test.hx:10: characters 13-24 : Unmatched patterns: Chest1 | Chest2 | Test3 | TestA | TestB | Trap | Trap2
class Test {
  public static function main(): Void {
    switch (Rooms.Empty) {
      case Empty:
    }
  }
}
