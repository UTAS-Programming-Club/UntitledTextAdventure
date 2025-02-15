import game.generated.Generated;

// haxe --macro "TestMacros.GameGeneration.generateEnums()" --main Test --interp
// haxe --macro "TestMacros.GameGeneration.generateEnums()" --main Test --python test.py && python test.py

// This should give:
// [TestB,TestA,Empty,Chest2,Chest1,Trap2,Trap,Test3]
// [GoWest,GoSouth,GoNorth,GoEast,OpenChest]
class Test {
  public static function main(): Void {
    Sys.println(Type.getEnumConstructs(Rooms));
    Sys.println(Type.getEnumConstructs(Actions));
  }
}
