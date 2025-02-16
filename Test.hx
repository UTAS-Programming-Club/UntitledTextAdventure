import game.generated.Generated;

// haxe --macro "TestMacros.GameGeneration.generateTypes()" --main Test --interp
// haxe --macro "TestMacros.GameGeneration.generateTypes()" --main Test --python test.py && python test.py

// This should give:
// [GoWest,GoSouth,GoNorth,GoEast,OpenChest,DodgeTrap]
// [{ name : Rags, type : Chest },{ name : Rags, type : Chest },{ name : Rags, type : Legs },{ name : Rags, type : Legs }]
// [Empty,Chest,Trap]
class Test {
  public static function main(): Void {
    Sys.println(Type.getEnumConstructs(Actions));
    Sys.println(Equipment);
    Sys.println(Type.getEnumConstructs(Rooms));
  }
}
