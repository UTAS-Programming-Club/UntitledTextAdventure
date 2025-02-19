import game.generated.Generated;

// haxe --macro "TestMacros.GameGeneration.generateTypes()" --main Test --interp
// haxe --macro "TestMacros.GameGeneration.generateTypes()" --main Test --python test.py && python test.py

// This should give:
// [GoWest,GoSouth,GoNorth,GoEast,OpenChest,DodgeTrap]
// haxe.iterators.ArrayIterator( array : [ChestRags,LegRags], current : 0, hasNext : function, next : function )
// [Empty,Chest,Trap]
class Test {
  public static function main(): Void {
    Sys.println(Type.getEnumConstructs(Actions));
    Sys.println(Equipment.keys());
    Sys.println(Type.getEnumConstructs(Rooms));
  }
}
